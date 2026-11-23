/* Decode the masters and release atlases with the same system WIC pixel
   format as the game. A size reduction must never cost a frame or an alpha edge. */
#define COBJMACROS
#include <windows.h>
#include <wincodec.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
static IWICImagingFactory *factory;
static BYTE *read_pixels(const wchar_t *path,UINT *w,UINT *h){
    IWICBitmapDecoder *dec=NULL;IWICBitmapFrameDecode *frame=NULL;IWICFormatConverter *cv=NULL;
    assert(SUCCEEDED(IWICImagingFactory_CreateDecoderFromFilename(factory,path,NULL,GENERIC_READ,
           WICDecodeMetadataCacheOnLoad,&dec)));
    UINT frames=0;assert(SUCCEEDED(IWICBitmapDecoder_GetFrameCount(dec,&frames)) && frames==1);
    assert(SUCCEEDED(IWICBitmapDecoder_GetFrame(dec,0,&frame)));
    assert(SUCCEEDED(IWICImagingFactory_CreateFormatConverter(factory,&cv)));
    assert(SUCCEEDED(IWICFormatConverter_Initialize(cv,(IWICBitmapSource*)frame,&GUID_WICPixelFormat32bppPBGRA,
           WICBitmapDitherTypeNone,NULL,0,WICBitmapPaletteTypeCustom)));
    assert(SUCCEEDED(IWICFormatConverter_GetSize(cv,w,h)));
    UINT bytes=(*w)*(*h)*4;BYTE *pixels=malloc(bytes);assert(pixels);
    assert(SUCCEEDED(IWICFormatConverter_CopyPixels(cv,NULL,(*w)*4,bytes,pixels)));
    IWICFormatConverter_Release(cv);IWICBitmapFrameDecode_Release(frame);IWICBitmapDecoder_Release(dec);
    return pixels;
}
int main(void){
    const wchar_t *names[]={L"units",L"motion-0",L"motion-1",L"motion-2",L"motion-3",L"motion-4",
        L"ultimate",L"portraits-silk",L"portraits-iron",L"title-landscape",L"title-swordsman",
        L"domains",L"domain-props",L"result-crests",L"roster-busts"};
    assert(SUCCEEDED(CoInitialize(NULL)));
    assert(SUCCEEDED(CoCreateInstance(&CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,
                                      &IID_IWICImagingFactory,(void**)&factory)));
    double worst=0;
    for(int i=0;i<15;i++){
        wchar_t path[160];UINT w,h,x,y;
        swprintf(path,160,L"assets/%ls.png",names[i]); BYTE *a=read_pixels(path,&w,&h);
        swprintf(path,160,L"assets/runtime-art/%ls.jxr",names[i]);BYTE *b=read_pixels(path,&x,&y);
        assert(x==w && y==h);double squared=0;size_t visiblePixels=0;
        for(size_t k=0;k<(size_t)w*h*4;k+=4){
            assert(a[k+3]==b[k+3]);
            if(!a[k+3])continue;
            visiblePixels++;
            for(int c=0;c<3;c++){int d=(int)a[k+c]-(int)b[k+c];squared+=d*d;}
        }
        double rms=sqrt(squared/(visiblePixels*3.0));if(rms>worst)worst=rms;
        free(a);free(b);assert(rms<5);
    }
    IWICImagingFactory_Release(factory);CoUninitialize();
    printf("15 release images: all dimensions and alpha bytes identical; worst visible RGB RMS %.2f/255.\n",worst);
    return 0;
}
