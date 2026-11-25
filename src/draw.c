#include "draw.h"
#include "living.h"
#include <d2d1.h>
#include <initguid.h>
#include <dwrite.h>
#include <wincodec.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
typedef struct {
    int face, size;
    IDWriteTextFormat *f;
} Font;
static ID2D1Factory *factory;
static ID2D1RenderTarget *rt;
static ID2D1HwndRenderTarget *hw;
static ID2D1SolidColorBrush *brush;
static IDWriteFactory *dw;
static IWICImagingFactory *wic;
static IWICBitmap *canvas;
static ID2D1Bitmap *images[ART_COUNT];
static float iw[ART_COUNT], ih[ART_COUNT], scale = 1, ox, oy;
static Font fonts[128];
static int nf, W, H;
static ID2D1Bitmap *fog;
static ID2D1Layer *hudLayer;
static int fogrev = -1;
static D2D1_MATRIX_3X2_F mat(float a, float d, float x, float y) {
    D2D1_MATRIX_3X2_F m;
    memset(&m, 0, sizeof m);
    m._11 = a;
    m._22 = d;
    m._31 = x;
    m._32 = y;
    return m;
}
static D2D1_COLOR_F color(uint32_t c, float a) {
    return (D2D1_COLOR_F){((c >> 16) & 255) / 255.f, ((c >> 8) & 255) / 255.f, (c & 255) / 255.f, a};
}
static ID2D1Brush *ink(uint32_t c, float a) {
    D2D1_COLOR_F v = color(c, a);
    ID2D1SolidColorBrush_SetColor(brush, &v);
    return (ID2D1Brush *)brush;
}
static void transform(void) {
    D2D1_MATRIX_3X2_F m = mat(scale, scale, ox, oy);
    ID2D1RenderTarget_SetTransform(rt, &m);
}
typedef struct {
    Living *rig;
    ID2D1Bitmap *surface;
    int w, h, frame;
} PuppetImage;
static PuppetImage actors[RIG_COUNT];
static HRESULT make_actor(int id, IWICBitmapSource *image, int sx, int sy, int sw, int sh, int outWidth) {
    PuppetImage *a = &actors[id];
    a->w = outWidth;
    a->h = (sh * outWidth + sw / 2) / sw;
    a->frame = -1;
    IWICBitmapClipper *clip = NULL;
    IWICBitmapScaler *resize = NULL;
    WICRect crop = {sx, sy, sw, sh};
    HRESULT hr = IWICImagingFactory_CreateBitmapClipper(wic, &clip);
    if (SUCCEEDED(hr))
        hr = IWICBitmapClipper_Initialize(clip, image, &crop);
    if (SUCCEEDED(hr))
        hr = IWICImagingFactory_CreateBitmapScaler(wic, &resize);
    if (SUCCEEDED(hr))
        hr = IWICBitmapScaler_Initialize(resize, (IWICBitmapSource *)clip, a->w, a->h,
                                         WICBitmapInterpolationModeFant);
    size_t bytes = (size_t)a->w * a->h * 4;
    uint32_t *pixels = malloc(bytes);
    if (!pixels)
        hr = E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
        hr = IWICBitmapScaler_CopyPixels(resize, NULL, a->w * 4, (UINT)bytes, (BYTE *)pixels);
    if (SUCCEEDED(hr) && !(a->rig = living_new(id, a->w, a->h, pixels)))
        hr = E_OUTOFMEMORY;
    if (SUCCEEDED(hr)) {
        D2D1_BITMAP_PROPERTIES bp = {{DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED}, 96, 96};
        D2D1_SIZE_U size = {(UINT)a->w, (UINT)a->h};
        hr = ID2D1RenderTarget_CreateBitmap(rt, size, pixels, a->w * 4, &bp, &a->surface);
    }
    free(pixels);
    if (resize)
        IWICBitmapScaler_Release(resize);
    if (clip)
        IWICBitmapClipper_Release(clip);
    return hr;
}
void living_art(int model, float x, float y, float w, float h, float seconds, float alpha) {
    if (model < 0 || model >= RIG_COUNT || alpha <= 0)
        return;
    PuppetImage *a = &actors[model];
    if (!a->surface)
        return;
    int frame = (int)floorf(seconds * 30.f);
    if (frame != a->frame) {
        const uint32_t *p = living_frame(a->rig, frame / 30.f);
        if (SUCCEEDED(ID2D1Bitmap_CopyFromMemory(a->surface, NULL, p, a->w * 4)))
            a->frame = frame;
    }
    D2D1_RECT_F d = {x, y, x + w, y + h};
    ID2D1RenderTarget_DrawBitmap(rt, a->surface, &d, alpha, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, NULL);
}
static int load(int i) {
    IWICStream *stream = NULL;
    IWICBitmapDecoder *dec = NULL;
    IWICBitmapFrameDecode *f = NULL;
    IWICFormatConverter *cv = NULL;
    HRSRC res = FindResourceW(NULL, MAKEINTRESOURCEW(201 + i), MAKEINTRESOURCEW(10));
    if (!res)
        return 0;
    BYTE *data = LockResource(LoadResource(NULL, res));
    DWORD len = SizeofResource(NULL, res);
    HRESULT hr = IWICImagingFactory_CreateStream(wic, &stream);
    if (SUCCEEDED(hr))
        hr = IWICStream_InitializeFromMemory(stream, data, len);
    if (SUCCEEDED(hr))
        hr = IWICImagingFactory_CreateDecoderFromStream(wic, (IStream *)stream, NULL,
                                                        WICDecodeMetadataCacheOnLoad, &dec);
    if (SUCCEEDED(hr))
        hr = IWICBitmapDecoder_GetFrame(dec, 0, &f);
    if (SUCCEEDED(hr))
        hr = IWICImagingFactory_CreateFormatConverter(wic, &cv);
    if (SUCCEEDED(hr))
        hr = IWICFormatConverter_Initialize(cv, (IWICBitmapSource *)f, &GUID_WICPixelFormat32bppPBGRA,
                                            WICBitmapDitherTypeNone, NULL, 0, WICBitmapPaletteTypeCustom);
    if (SUCCEEDED(hr)) {
        UINT w, h;
        IWICFormatConverter_GetSize(cv, &w, &h);
        iw[i] = (float)w;
        ih[i] = (float)h;
        if (i != ART_SILK && i != ART_IRON && i != ART_SWORDSMAN && i != ART_ROSTER_BUSTS)
            hr = ID2D1RenderTarget_CreateBitmapFromWicBitmap(rt, (IWICBitmapSource *)cv, NULL, &images[i]);
        if (SUCCEEDED(hr) && i == ART_SWORDSMAN)
            hr = make_actor(0, (IWICBitmapSource *)cv, 0, 0, w, h, 1280);
        if (i==ART_ROSTER_BUSTS)
            for(int k=0;k<5 && SUCCEEDED(hr); k++)
                hr=make_actor(RIG_STORM+k,(IWICBitmapSource*)cv,k*(w/5),0,w/5,h,360);
        if(i==ART_PROPS){
            const int tiles[]={0,1,3,5};
            for(int k=0;k<4 && SUCCEEDED(hr);k++){
                int tile=tiles[k];
                hr=make_actor(RIG_FLAME+k,(IWICBitmapSource*)cv,tile%3*512,tile/3*512,512,512,256);
            }
        }
        if (i == ART_SILK || i == ART_IRON) {
            static const int crops[][3] = {
                {0, 634, 822}, {638, 634, 822}, {1275, 638, 822}, {0, 759, 1024}, {765, 771, 1024}};
            int lo = i == ART_SILK ? 0 : 3, hi = i == ART_SILK ? 3 : 5;
            for (int n = lo; n < hi && SUCCEEDED(hr); n++)
                hr = make_actor(n + 1, (IWICBitmapSource *)cv, crops[n][0], 0, crops[n][1], crops[n][2], 464);
        }
    }
    if (cv)
        IWICFormatConverter_Release(cv);
    if (f)
        IWICBitmapFrameDecode_Release(f);
    if (dec)
        IWICBitmapDecoder_Release(dec);
    if (stream)
        IWICStream_Release(stream);
    return SUCCEEDED(hr);
}
int draw_init(HWND wnd, int w, int h) {
    W = w;
    H = h;
    HRESULT hr =
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &IID_ID2D1Factory, NULL, (void **)&factory);
    if (FAILED(hr))
        return 0;
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, &IID_IDWriteFactory, (IUnknown **)&dw);
    if (FAILED(hr))
        return 0;
    hr = CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, &IID_IWICImagingFactory,
                          (void **)&wic);
    if (FAILED(hr))
        return 0;
    D2D1_RENDER_TARGET_PROPERTIES p = {0};
    p.dpiX = p.dpiY = 96;
    if (wnd) {
        D2D1_HWND_RENDER_TARGET_PROPERTIES hp = {0};
        hp.hwnd = wnd;
        hp.pixelSize = (D2D1_SIZE_U){w, h};
        hr = ID2D1Factory_CreateHwndRenderTarget(factory, &p, &hp, &hw);
        if (FAILED(hr)) {
            p.type = D2D1_RENDER_TARGET_TYPE_SOFTWARE;
            hr = ID2D1Factory_CreateHwndRenderTarget(factory, &p, &hp, &hw);
        }
        rt = (ID2D1RenderTarget *)hw;
    } else {
        hr = IWICImagingFactory_CreateBitmap(wic, w, h, &GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad,
                                             &canvas);
        if (SUCCEEDED(hr)) {
            p.type = D2D1_RENDER_TARGET_TYPE_SOFTWARE;
            hr = ID2D1Factory_CreateWicBitmapRenderTarget(factory, (IWICBitmap *)canvas, &p, &rt);
        }
    }
    if (FAILED(hr))
        return 0;
    D2D1_COLOR_F white = color(0xffffff, 1);
    hr = ID2D1RenderTarget_CreateSolidColorBrush(rt, &white, NULL, &brush);
    if (FAILED(hr))
        return 0;
    if(FAILED(ID2D1RenderTarget_CreateLayer(rt,NULL,&hudLayer)))return 0;
    for (int i = 0; i < ART_COUNT; i++)
        if (!load(i))
            return 0;
    draw_resize(w, h);
    return 1;
}
void draw_close(void) {
    for(int i=0;i<RIG_COUNT;i++){
        living_free(actors[i].rig);
        if(actors[i].surface)ID2D1Bitmap_Release(actors[i].surface);
        actors[i]=(PuppetImage){0};
    }
    for (int i = 0; i < nf; i++)
        if (fonts[i].f)
            IDWriteTextFormat_Release(fonts[i].f);
    nf = 0;
    for (int i = 0; i < ART_COUNT; i++)
        if (images[i]) {
            ID2D1Bitmap_Release(images[i]);
            images[i] = NULL;
        }
    if (fog)
        ID2D1Bitmap_Release(fog);
    fog = NULL;
    fogrev = -1;
    if(hudLayer)ID2D1Layer_Release(hudLayer);
    hudLayer=NULL;
    if (brush)
        ID2D1SolidColorBrush_Release(brush);
    if (rt)
        ID2D1RenderTarget_Release(rt);
    if (canvas)
        IWICBitmap_Release(canvas);
    if (wic)
        IWICImagingFactory_Release(wic);
    if (dw)
        IDWriteFactory_Release(dw);
    if (factory)
        ID2D1Factory_Release(factory);
    hw = NULL;
    rt = NULL;
    wic = NULL;
    dw = NULL;
    factory = NULL;
    brush = NULL;
    canvas = NULL;
}
void draw_resize(int w, int h) {
    if (w < 1 || h < 1)
        return;
    W = w;
    H = h;
    scale = fminf(w / 1280.f, h / 720.f);
    ox = (w - scale * 1280) * .5f;
    oy = (h - scale * 720) * .5f;
    if (hw) {
        D2D1_SIZE_U s = {(UINT)w, (UINT)h};
        ID2D1HwndRenderTarget_Resize(hw, &s);
    }
}
void to_virtual(float *x, float *y) {
    *x = (*x - ox) / scale;
    *y = (*y - oy) / scale;
}
void draw_begin(void) {
    ID2D1RenderTarget_BeginDraw(rt);
    transform();
    D2D1_COLOR_F c = color(0x071116, 1);
    ID2D1RenderTarget_Clear(rt, &c);
    cursor_clip(0, 0, 1280, 720);
}
int draw_end(void) {
    clip_end();
    return SUCCEEDED(ID2D1RenderTarget_EndDraw(rt, NULL, NULL));
}
void cursor_clip(float x, float y, float w, float h) {
    D2D1_RECT_F r = {x, y, x + w, y + h};
    ID2D1RenderTarget_PushAxisAlignedClip(rt, &r, D2D1_ANTIALIAS_MODE_ALIASED);
}
void clip_end(void) {
    ID2D1RenderTarget_PopAxisAlignedClip(rt);
}
void opacity_begin(float alpha){
    D2D1_LAYER_PARAMETERS p={0};
    p.contentBounds=(D2D1_RECT_F){0,0,1280,720};
    p.maskTransform=mat(1,1,0,0);
    p.opacity=alpha;
    ID2D1RenderTarget_PushLayer(rt,&p,hudLayer);
}
void opacity_end(void){ID2D1RenderTarget_PopLayer(rt);}
void box(float x, float y, float w, float h, uint32_t c, float a) {
    if (w <= 0 || h <= 0)
        return;
    D2D1_RECT_F r = {x, y, x + w, y + h};
    ID2D1RenderTarget_FillRectangle(rt, &r, ink(c, a));
}
void frame(float x, float y, float w, float h, uint32_t c, float a, float sw) {
    D2D1_RECT_F r = {x, y, x + w, y + h};
    ID2D1RenderTarget_DrawRectangle(rt, &r, ink(c, a), sw, NULL);
}
void line(float x, float y, float xx, float yy, uint32_t c, float a, float s) {
    ID2D1RenderTarget_DrawLine(rt, ((D2D1_POINT_2F){x, y}), ((D2D1_POINT_2F){xx, yy}), ink(c, a), s, NULL);
}
void oval(float x, float y, float rx, float ry, uint32_t c, float a, int fill, float s) {
    if (rx <= 0 || ry <= 0)
        return;
    D2D1_ELLIPSE e = {{x, y}, rx, ry};
    if (fill)
        ID2D1RenderTarget_FillEllipse(rt, &e, ink(c, a));
    else
        ID2D1RenderTarget_DrawEllipse(rt, &e, ink(c, a), s, NULL);
}
void poly(const float *p, int n, uint32_t c, float a) {
    ID2D1PathGeometry *geo = NULL;
    ID2D1GeometrySink *s = NULL;
    if (n < 3 || FAILED(ID2D1Factory_CreatePathGeometry(factory, &geo)))
        return;
    if (SUCCEEDED(ID2D1PathGeometry_Open(geo, &s))) {
        ID2D1GeometrySink_BeginFigure(s, ((D2D1_POINT_2F){p[0], p[1]}), D2D1_FIGURE_BEGIN_FILLED);
        for (int i = 1; i < n; i++)
            ID2D1GeometrySink_AddLine(s, ((D2D1_POINT_2F){p[i * 2], p[i * 2 + 1]}));
        ID2D1GeometrySink_EndFigure(s, D2D1_FIGURE_END_CLOSED);
        ID2D1GeometrySink_Close(s);
        ID2D1RenderTarget_FillGeometry(rt, (ID2D1Geometry *)geo, ink(c, a), NULL);
        ID2D1GeometrySink_Release(s);
    }
    ID2D1PathGeometry_Release(geo);
}
void gradient(float x, float y, float w, float h, uint32_t c, float a, uint32_t d, float b, int vert) {
    D2D1_GRADIENT_STOP stops[] = {{0, color(c, a)}, {1, color(d, b)}};
    ID2D1GradientStopCollection *s = NULL;
    ID2D1LinearGradientBrush *br = NULL;
    if (FAILED(rt->lpVtbl->CreateGradientStopCollection(rt, stops, 2, D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP,
                                                        &s)))
        return;
    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES p = {{x, y}, {vert ? x : x + w, vert ? y + h : y}};
    if (SUCCEEDED(ID2D1RenderTarget_CreateLinearGradientBrush(rt, &p, NULL, s, &br))) {
        D2D1_RECT_F r = {x, y, x + w, y + h};
        ID2D1RenderTarget_FillRectangle(rt, &r, (ID2D1Brush *)br);
        ID2D1LinearGradientBrush_Release(br);
    }
    ID2D1GradientStopCollection_Release(s);
}
/* Soft additive-looking aura, clipped to the ellipse with a zero-alpha edge. */
void glow(float cx, float cy, float rx, float ry, uint32_t col, float a) {
    if (rx <= 0 || ry <= 0 || a <= 0)
        return;
    D2D1_GRADIENT_STOP stops[] = {{0, color(col, a)}, {.47f, color(col, a * .44f)}, {1, color(col, 0)}};
    ID2D1GradientStopCollection *st = NULL;
    if (FAILED(rt->lpVtbl->CreateGradientStopCollection(rt, stops, 3, D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP,
                                                        &st)))
        return;
    ID2D1RadialGradientBrush *br = NULL;
    D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES p = {{cx, cy}, {0, 0}, rx, ry};
    if (SUCCEEDED(ID2D1RenderTarget_CreateRadialGradientBrush(rt, &p, NULL, st, &br))) {
        D2D1_ELLIPSE el = {{cx, cy}, rx, ry};
        ID2D1RenderTarget_FillEllipse(rt, &el, (ID2D1Brush *)br);
        ID2D1RadialGradientBrush_Release(br);
    }
    ID2D1GradientStopCollection_Release(st);
}
static IDWriteTextFormat *format(float sz, int face) {
    int size = (int)(sz + .5f);
    IDWriteTextFormat *f = NULL;
    for (int i = 0; i < nf; i++)
        if (fonts[i].face == face && fonts[i].size == size) {
            f = fonts[i].f;
            break;
        }
    if (!f) {
        if (nf >= 128)
            return NULL;
        const wchar_t *family = face == F_SERIF ? L"SimSun"
                                : face == F_NUM ? L"Bahnschrift"
                                                : L"Microsoft YaHei UI";
        if (FAILED(IDWriteFactory_CreateTextFormat(
                dw, family, NULL,
                face == F_BOLD || face == F_SERIF ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, (FLOAT)size, L"zh-CN", &f)))
            return NULL;
        IDWriteTextFormat_SetWordWrapping(f, DWRITE_WORD_WRAPPING_NO_WRAP);
        IDWriteTextFormat_SetParagraphAlignment(f, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        fonts[nf++] = (Font){face, size, f};
    }
    return f;
}
/* HUD toasts and skill tooltips use the same DWrite font as label(). Cache
   measured strings so drawing the same tip at 60 Hz does not allocate layouts. */
float text_width(const wchar_t *s, float sz, int face) {
    static struct {
        wchar_t s[160];
        int size, face;
        float width;
    } cache[32];
    static unsigned next;
    if (!s || !s[0])
        return 0;
    int px = (int)(sz + .5f);
    for (int i = 0; i < 32; i++)
        if (cache[i].size == px && cache[i].face == face && !wcscmp(cache[i].s, s))
            return cache[i].width;
    IDWriteTextFormat *f = format(sz, face);
    IDWriteTextLayout *layout = NULL;
    if (!f || FAILED(IDWriteFactory_CreateTextLayout(dw, s, (UINT32)wcslen(s), f, 2048, 84, &layout)))
        return (float)wcslen(s) * px;
    DWRITE_TEXT_METRICS m;
    HRESULT hr = IDWriteTextLayout_GetMetrics(layout, &m);
    IDWriteTextLayout_Release(layout);
    float width = SUCCEEDED(hr) ? ceilf(m.widthIncludingTrailingWhitespace) : (float)wcslen(s) * px;
    if (wcslen(s) < 160) {
        unsigned j = next++ % 32;
        wcscpy(cache[j].s, s);
        cache[j].size = px;
        cache[j].face = face;
        cache[j].width = width;
    }
    return width;
}
void label(const wchar_t *s, float x, float y, float w, float h, float sz, uint32_t c, float a, int face,
           int align) {
    if (!s || !*s)
        return;
    IDWriteTextFormat *f = format(sz, face);
    if (!f)
        return;
    DWRITE_TEXT_ALIGNMENT ta[] = {DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_TEXT_ALIGNMENT_CENTER,
                                  DWRITE_TEXT_ALIGNMENT_TRAILING};
    IDWriteTextFormat_SetTextAlignment(f, ta[align]);
    D2D1_RECT_F r = {x, y, x + w, y + h};
    ID2D1RenderTarget_DrawText(rt, s, (UINT32)wcslen(s), f, &r, ink(c, a), D2D1_DRAW_TEXT_OPTIONS_CLIP,
                               DWRITE_MEASURING_MODE_NATURAL);
}
void art(int id, float x, float y, float w, float h, float sx, float sy, float sw, float sh, float a,
         int flip) {
    D2D1_RECT_F dest = {x, y, x + w, y + h}, src = {sx, sy, sx + sw, sy + sh};
    if (flip) {
        D2D1_MATRIX_3X2_F m = mat(-scale, scale, ox + (2 * x + w) * scale, oy);
        ID2D1RenderTarget_SetTransform(rt, &m);
    }
    ID2D1RenderTarget_DrawBitmap(rt, images[id], &dest, a, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &src);
    if (flip)
        transform();
}
void cover(int id, float x, float y, float w, float h, float bias, float a) {
    float sw = iw[id], sh = ih[id], ratio = w / h;
    if (sw / sh > ratio)
        sw = sh * ratio;
    else
        sh = sw / ratio;
    art(id, x, y, w, h, (iw[id] - sw) * .5f, (ih[id] - sh) * bias, sw, sh, a, 0);
}
void portrait(int id, float x, float y, float w, float h, float bias, float a) {
    int sheet=id<5?ART_HEROES:ART_ROSTER_CARDS;
    float cw = iw[sheet] / 5, sw = cw - 2, sh = ih[sheet], ratio = w / h;
    if (sw / sh > ratio)
        sw = sh * ratio;
    else
        sh = sw / ratio;
    art(sheet, x, y, w, h, cw * (id%5) + (cw - sw) * .5f, (ih[sheet] - sh) * bias, sw, sh, a, 0);
}
void sprite(int id, float x, float feet, float size, float a, int flip) {
    art(3, x - size * .5f, feet - size * .966f, size, size, (id % 6) * 256.f, (id / 6) * 256.f, 256, 256, a,
        flip);
}
void effect_stamp(int tile, float cx, float cy, float w, float h, float r, float opacity) {
    if (opacity <= 0 || w <= 0 || h <= 0)
        return;
    float co = cosf(r) * scale, si = sinf(r) * scale;
    D2D1_MATRIX_3X2_F m = mat(co, co, ox + cx * scale, oy + cy * scale);
    m._12 = si;
    m._21 = -si;
    ID2D1RenderTarget_SetTransform(rt, &m);
    D2D1_RECT_F dst = {-w * .5f, -h * .5f, w * .5f, h * .5f};
    float sx = (tile % 5) * 256, sy = (tile / 5) * 256;
    D2D1_RECT_F src = {sx, sy, sx + 256, sy + 256};
    ID2D1RenderTarget_DrawBitmap(rt, images[ART_ULTIMATE], &dst, fminf(1, opacity),
                                 D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &src);
    transform();
}
static const unsigned char *fogSource;
void invalidate_shroud(void) {
    fogrev = -1;
    fogSource = NULL;
}
void shroud(float x, float y, float w, float h, const unsigned char *light, int cols, int rows, int rev,float opacity) {
    if (!fog || fogrev != rev || fogSource != light) {
        UINT32 data[8192];
        if (cols * rows > 8192)
            return;
        for (int i = 0; i < cols * rows; i++) {
            unsigned a = (255 - light[i]) * 147 / 255;
            data[i] = (a << 24) | ((a * 9 / 255) << 16) | ((a * 22 / 255) << 8) | (a * 39 / 255);
        }
        if (!fog) {
            D2D1_BITMAP_PROPERTIES bp = {{DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED}, 96, 96};
            D2D1_SIZE_U sz = {(UINT32)cols, (UINT32)rows};
            if (FAILED(ID2D1RenderTarget_CreateBitmap(rt, sz, data, cols * 4, &bp, &fog)))
                return;
        } else
            ID2D1Bitmap_CopyFromMemory(fog, NULL, data, cols * 4);
        fogrev = rev;
        fogSource = light;
    }
    D2D1_RECT_F dst = {x, y, x + w, y + h};
    ID2D1RenderTarget_DrawBitmap(rt, fog, &dst, opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, NULL);
}
uint32_t draw_region_hash(int x,int y,int w,int h){
    if(!canvas || w<1 || h<1 || x<0 || y<0 || x+w>W || y+h>H)return 0;
    size_t size=(size_t)w*h*4;
    BYTE *p=malloc(size); if(!p)return 0;
    WICRect r={x,y,w,h}; uint32_t hash=0;
    if(SUCCEEDED(IWICBitmap_CopyPixels(canvas,&r,w*4,(UINT)size,p))){
        hash=2166136261u; for(size_t k=0;k<size;k++)hash=(hash^p[k])*16777619u;
    }
    free(p);return hash;
}
int draw_save_bmp(const wchar_t *path) {
    if (!canvas)
        return 0;
    int sz = W * H * 4;
    BYTE *p = malloc(sz);
    if (!p)
        return 0;
    HRESULT hr = IWICBitmap_CopyPixels(canvas, NULL, W * 4, sz, p);
    FILE *f = SUCCEEDED(hr) ? _wfopen(path, L"wb") : NULL;
    int ok = 0;
    if (f) {
        BITMAPFILEHEADER fh = {0};
        BITMAPINFOHEADER ihd = {0};
        fh.bfType = 0x4d42;
        fh.bfSize = sizeof fh + sizeof ihd + sz;
        fh.bfOffBits = sizeof fh + sizeof ihd;
        ihd.biSize = sizeof ihd;
        ihd.biWidth = W;
        ihd.biHeight = -H;
        ihd.biPlanes = 1;
        ihd.biBitCount = 32;
        fwrite(&fh, sizeof fh, 1, f);
        fwrite(&ihd, sizeof ihd, 1, f);
        ok = fwrite(p, 1, sz, f) == (size_t)sz;
        fclose(f);
    }
    free(p);
    return ok;
}

#ifdef RIFT_CAPTURE
int draw_copy_pixels(unsigned char *output,int bytes){
    return canvas && bytes==W*H*4 && SUCCEEDED(IWICBitmap_CopyPixels(canvas,NULL,W*4,bytes,output));
}
#endif
