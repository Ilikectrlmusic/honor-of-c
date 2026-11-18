#include "draw.h"
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
static ID2D1Bitmap *images[5];
static float iw[5], ih[5], scale = 1, ox, oy;
static Font fonts[128];
static int nf, W, H;
static ID2D1Bitmap *fog;
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
        hr = ID2D1RenderTarget_CreateBitmapFromWicBitmap(rt, (IWICBitmapSource *)cv, NULL, &images[i]);
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
    for (int i = 0; i < 5; i++)
        if (!load(i))
            return 0;
    draw_resize(w, h);
    return 1;
}
void draw_close(void) {
    for (int i = 0; i < nf; i++)
        if (fonts[i].f)
            IDWriteTextFormat_Release(fonts[i].f);
    nf = 0;
    for (int i = 0; i < 5; i++)
        if (images[i]) {
            ID2D1Bitmap_Release(images[i]);
            images[i] = NULL;
        }
    if (fog)
        ID2D1Bitmap_Release(fog);
    fog = NULL;
    fogrev = -1;
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
void label(const wchar_t *s, float x, float y, float w, float h, float sz, uint32_t c, float a, int face,
           int align) {
    if (!s || !*s)
        return;
    int size = (int)(sz + .5f);
    IDWriteTextFormat *f = NULL;
    for (int i = 0; i < nf; i++)
        if (fonts[i].face == face && fonts[i].size == size) {
            f = fonts[i].f;
            break;
        }
    if (!f) {
        if (nf >= 128)
            return;
        const wchar_t *family = face == F_SERIF ? L"SimSun"
                                : face == F_NUM ? L"Bahnschrift"
                                                : L"Microsoft YaHei UI";
        if (FAILED(IDWriteFactory_CreateTextFormat(
                dw, family, NULL,
                face == F_BOLD || face == F_SERIF ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, (FLOAT)size, L"zh-CN", &f)))
            return;
        IDWriteTextFormat_SetWordWrapping(f, DWRITE_WORD_WRAPPING_NO_WRAP);
        IDWriteTextFormat_SetParagraphAlignment(f, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        fonts[nf++] = (Font){face, size, f};
    }
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
    float cw = iw[1] / 5, sw = cw - 2, sh = ih[1], ratio = w / h;
    if (sw / sh > ratio)
        sw = sh * ratio;
    else
        sh = sw / ratio;
    art(1, x, y, w, h, cw * id + (cw - sw) * .5f, (ih[1] - sh) * bias, sw, sh, a, 0);
}
void sprite(int id, float x, float feet, float size, float a, int flip) {
    art(3, x - size * .5f, feet - size * .966f, size, size, (id % 6) * 256.f, (id / 6) * 256.f, 256, 256, a,
        flip);
}
void shroud(float x, float y, float w, float h, const unsigned char *light, int cols, int rows, int rev) {
    if (!fog || fogrev != rev) {
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
    }
    D2D1_RECT_F dst = {x, y, x + w, y + h};
    ID2D1RenderTarget_DrawBitmap(rt, fog, &dst, 1, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, NULL);
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
