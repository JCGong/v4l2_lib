/******************************************************************** 
    created:    2012/05/19 
    filename:   rgb_convert.c 
    author:      
     
    purpose:     
*********************************************************************/  
  
//-------------------------------------------------------------------  
#include <stdlib.h>  
#include <stdio.h>  
#include <memory.h>  
#include "mybmp.h"  
  
//-------------------------------------------------------------------  
//拷贝  
void rgb_copy(const void * psrc, void * pdst, int sw, int sh, int dw, int dh, int bpp)  
{  
    int bytes = UpAlign8(bpp) >> 3; // bpp / 8  
    int srclinesize = UpAlign4(sw * bytes);  
    int dstlinesize = UpAlign4(dw * bytes);  
    int copylinesize = srclinesize < dstlinesize ? srclinesize : dstlinesize;  
    int copylines = sh < dh ? sh : dh;  
      
    const unsigned char * psrcline = (const unsigned char *)psrc;  
    const unsigned char * pend = psrcline + copylines * srclinesize;  
    unsigned char * pdstline = (unsigned char *)pdst;  
      
    while (psrcline < pend) {  
        memcpy(pdstline, psrcline, copylinesize);  
        psrcline += srclinesize;  
        pdstline += dstlinesize;  
    }  
}  
  
void rbg565_copy(const void * psrc, void * pdst, int sw, int sh, int dw, int dh)  
{  
    rgb_copy(psrc, pdst, sw, sh, dw, dh, 16);  
}  
  
void rbg888_copy(const void * psrc, void * pdst, int sw, int sh, int dw, int dh)  
{  
    rgb_copy(psrc, pdst, sw, sh, dw, dh, 24);  
}  
  
void rbgx8888_copy(const void * psrc, void * pdst, int sw, int sh, int dw, int dh)  
{  
    rgb_copy(psrc, pdst, sw, sh, dw, dh, 32);  
}  
  
//-------------------------------------------------------------------  
//行数据翻转  
void line_reversal(void * pdata, int w, int h, int bpp)  
{  
    int bytes     = UpAlign8(bpp) >> 3; // bpp / 8  
    int linesize  = UpAlign4(w * bytes);//4的整数倍  
    int copylines = h >> 1;  
  
    int i;  
    unsigned char * pline = NULL;  
    unsigned char * pline1 = NULL;  
    unsigned char * linebuffer = NULL;  
  
    if (pdata && w > 0 && h > 1) {//至少两行才需要翻转  
        linebuffer = (unsigned char *)malloc(linesize);  
        if (linebuffer) {  
            pline  = (unsigned char *)pdata;  
            pline1 = (unsigned char *)pdata + linesize * (h - 1);  
            for (i = 0; i < copylines; i++) {  
                memcpy(linebuffer, pline, linesize);  
                memcpy(pline, pline1, linesize);  
                memcpy(pline1, linebuffer, linesize);  
                pline  += linesize;  
                pline1 -= linesize;  
            }  
            free(linebuffer);  
        }  
    }  
}  
  
void rgb565_line_reversal(void * p565, int w, int h)  
{  
    line_reversal(p565, w, h, 16);  
}  
  
void rgb888_line_reversal(void * p888, int w, int h)  
{  
    line_reversal(p888, w, h, 24);  
}  
  
void rgbx8888_line_reversal(void * p8888, int w, int h)  
{  
    line_reversal(p8888, w, h, 32);  
}  
  
//-------------------------------------------------------------------  
//转换  
static int rgb565_to_rgb888(const void * psrc, int w, int h, void * pdst)  
{  
    int srclinesize = UpAlign4(w * 2);  
    int dstlinesize = UpAlign4(w * 3);  
  
    const unsigned char  * psrcline;  
    const unsigned short * psrcdot;  
    unsigned char  * pdstline;  
    unsigned char  * pdstdot;  
  
    int i,j;  
  
    if (!psrc || !pdst || w <= 0 || h <= 0) {  
        printf("rgb565_to_rgb888 : parameter error\n");  
        return -1;  
    }  
  
    psrcline = (const unsigned char *)psrc;  
    pdstline = (unsigned char *)pdst;  
    for (i=0; i<h; i++) {  
        psrcdot = (const unsigned short *)psrcline;  
        pdstdot = pdstline;  
        for (j=0; j<w; j++) {  
            //565 b|g|r -> 888 r|g|b  
            *pdstdot++ = (unsigned char)(((*psrcdot) >> 0 ) << 3);  
            *pdstdot++ = (unsigned char)(((*psrcdot) >> 5 ) << 2);  
            *pdstdot++ = (unsigned char)(((*psrcdot) >> 11) << 3);  
            psrcdot++;  
        }  
        psrcline += srclinesize;  
        pdstline += dstlinesize;  
    }  
  
    return 0;  
}  
  
static int rgb888_to_rgb565(const void * psrc, int w, int h, void * pdst)  
{  
    int srclinesize = UpAlign4(w * 3);  
    int dstlinesize = UpAlign4(w * 2);  
      
    const unsigned char * psrcline;  
    const unsigned char * psrcdot;  
    unsigned char  * pdstline;  
    unsigned short * pdstdot;  
      
    int i,j;  
      
    if (!psrc || !pdst || w <= 0 || h <= 0) {  
        printf("rgb888_to_rgb565 : parameter error\n");  
        return -1;  
    }  
  
    psrcline = (const unsigned char *)psrc;  
    pdstline = (unsigned char *)pdst;  
    for (i=0; i<h; i++) {  
        psrcdot = psrcline;  
        pdstdot = (unsigned short *)pdstline;  
        for (j=0; j<w; j++) {  
            //888 r|g|b -> 565 b|g|r  
            *pdstdot =  (((psrcdot[0] >> 3) & 0x1F) << 0)//r  
                        |(((psrcdot[1] >> 2) & 0x3F) << 5)//g  
                        |(((psrcdot[2] >> 3) & 0x1F) << 11);//b  
            psrcdot += 3;  
            pdstdot++;  
        }  
        psrcline += srclinesize;  
        pdstline += dstlinesize;  
    }  
  
    return 0;  
}  
  
static int rgb565_to_rgbx8888(const void * psrc, int w, int h, void * pdst)  
{  
    int srclinesize = UpAlign4(w * 2);  
    int dstlinesize = UpAlign4(w * 4);  
  
    const unsigned char  * psrcline;  
    const unsigned short * psrcdot;  
    unsigned char  * pdstline;  
    unsigned char  * pdstdot;  
  
    int i,j;  
  
    if (!psrc || !pdst || w <= 0 || h <= 0) {  
        printf("rgb565_to_rgbx8888 : parameter error\n");  
        return -1;  
    }  
  
    psrcline = (const unsigned char *)psrc;  
    pdstline = (unsigned char *)pdst;  
    for (i=0; i<h; i++) {  
        psrcdot = (const unsigned short *)psrcline;  
        pdstdot = pdstline;  
        for (j=0; j<w; j++) {  
            pdstdot++;  
            *pdstdot++ = (unsigned char)(((*psrcdot) >> 0 ) << 3);  
            *pdstdot++ = (unsigned char)(((*psrcdot) >> 5 ) << 2);  
            *pdstdot++ = (unsigned char)(((*psrcdot) >> 11) << 3);  
            psrcdot++;  
        }  
        psrcline += srclinesize;  
        pdstline += dstlinesize;  
    }  
      
    return 0;  
}  
  
static int rgbx8888_to_rgb565(const void * psrc, int w, int h, void * pdst)  
{  
    int srclinesize = UpAlign4(w * 4);  
    int dstlinesize = UpAlign4(w * 2);  
      
    const unsigned char * psrcline;  
    const unsigned char * psrcdot;  
    unsigned char  * pdstline;  
    unsigned short * pdstdot;  
      
    int i,j;  
      
    if (!psrc || !pdst || w <= 0 || h <= 0) {  
        printf("rgbx8888_to_rgb565 : parameter error\n");  
        return -1;  
    }  
      
    psrcline = (const unsigned char *)psrc;  
    pdstline = (unsigned char *)pdst;  
    for (i=0; i<h; i++) {  
        psrcdot = psrcline;  
        pdstdot = (unsigned short *)pdstline;  
        for (j=0; j<w; j++) {  
            //888 r|g|b -> 565 b|g|r  
            *pdstdot =  (((psrcdot[1] >> 3) & 0x1F) << 0)//r  
                |(((psrcdot[2] >> 2) & 0x3F) << 5)//g  
                |(((psrcdot[3] >> 3) & 0x1F) << 11);//b  
            psrcdot += 4;  
            pdstdot++;  
        }  
        psrcline += srclinesize;  
        pdstline += dstlinesize;  
    }  
      
    return 0;  
}  
  
static int rgb888_to_rgbx8888(const void * psrc, int w, int h, void * pdst)  
{  
    int srclinesize = UpAlign4(w * 3);  
    int dstlinesize = UpAlign4(w * 4);  
  
    const unsigned char * psrcline;  
    const unsigned char * psrcdot;  
    unsigned char  * pdstline;  
    unsigned char  * pdstdot;  
  
    int i,j;  
  
    if (!psrc || !pdst || w <= 0 || h <= 0) {  
        printf("rgb888_to_rgbx8888 : parameter error\n");  
        return -1;  
    }  
  
    psrcline = (const unsigned char *)psrc;  
    pdstline = (unsigned char *)pdst;  
    for (i=0; i<h; i++) {  
        psrcdot = psrcline;  
        pdstdot = pdstline;  
        for (j=0; j<w; j++) {  
            *pdstdot++ = 0;  
            *pdstdot++ = *psrcdot++;  
            *pdstdot++ = *psrcdot++;  
            *pdstdot++ = *psrcdot++;  
        }  
        psrcline += srclinesize;  
        pdstline += dstlinesize;  
    }  
      
    return 0;  
}  
  
static int rgbx8888_to_rgb888(const void * psrc, int w, int h, void * pdst)  
{  
    int srclinesize = UpAlign4(w * 4);  
    int dstlinesize = UpAlign4(w * 3);  
      
    const unsigned char * psrcline;  
    const unsigned char * psrcdot;  
    unsigned char  * pdstline;  
    unsigned char  * pdstdot;  
      
    int i,j;  
      
    if (!psrc || !pdst || w <= 0 || h <= 0) {  
        printf("rgbx8888_to_rgb888 : parameter error\n");  
        return -1;  
    }  
      
    psrcline = (const unsigned char *)psrc;  
    pdstline = (unsigned char *)pdst;  
    for (i=0; i<h; i++) {  
        psrcdot = psrcline;  
        pdstdot = pdstline;  
        for (j=0; j<w; j++) {  
            psrcdot++;  
            *pdstdot++ = *psrcdot++;  
            *pdstdot++ = *psrcdot++;  
            *pdstdot++ = *psrcdot++;  
        }  
        psrcline += srclinesize;  
        pdstline += dstlinesize;  
    }  
      
    return 0;  
}  
  
void * rgb565_to_rgb888_buffer(const void * psrc, int w, int h)  
{  
    int size = h * UpAlign4(w * 3);  
    void * pdst = NULL;  
    if (psrc && w > 0 && h > 0) {  
        pdst = malloc(size);  
        if (pdst) {  
            if (rgb565_to_rgb888(psrc, w, h, pdst)) {  
                free(pdst);  
                pdst = NULL;  
            }  
        }  
    }  
    return pdst;  
}  
  
void * rgb888_to_rgb565_buffer(const void * psrc, int w, int h)  
{  
    int size = h * UpAlign4(w * 2);  
    void * pdst = NULL;  
    if (psrc && w > 0 && h > 0) {  
        pdst = malloc(size);  
        if (pdst) {  
            if (rgb888_to_rgb565(psrc, w, h, pdst)) {  
                free(pdst);  
                pdst = NULL;  
            }  
        }  
    }  
    return pdst;  
}  
  
void * rgb565_to_rgbx8888_buffer(const void * psrc, int w, int h)  
{  
    int size = h * UpAlign4(w * 4);  
    void * pdst = NULL;  
    if (psrc && w > 0 && h > 0) {  
        pdst = malloc(size);  
        if (pdst) {  
            if (rgb565_to_rgbx8888(psrc, w, h, pdst)) {  
                free(pdst);  
                pdst = NULL;  
            }  
        }  
    }  
    return pdst;  
}  
  
void * rgbx8888_to_rgb565_buffer(const void * psrc, int w, int h)  
{  
    int size = h * UpAlign4(w * 2);  
    void * pdst = NULL;  
    if (psrc && w > 0 && h > 0) {  
        pdst = malloc(size);  
        if (pdst) {  
            if (rgbx8888_to_rgb565(psrc, w, h, pdst)) {  
                free(pdst);  
                pdst = NULL;  
            }  
        }  
    }  
    return pdst;  
}  
  
void * rgb888_to_rgbx8888_buffer(const void * psrc, int w, int h)  
{  
    int size = h * UpAlign4(w * 4);  
    void * pdst = NULL;  
    if (psrc && w > 0 && h > 0) {  
        pdst = malloc(size);  
        if (pdst) {  
            if (rgb888_to_rgbx8888(psrc, w, h, pdst)) {  
                free(pdst);  
                pdst = NULL;  
            }  
        }  
    }  
    return pdst;  
}  
  
void * rgbx8888_to_rgb888_buffer(const void * psrc, int w, int h)  
{  
    int size = h * UpAlign4(w * 3);  
    void * pdst = NULL;  
    if (psrc && w > 0 && h > 0) {  
        pdst = malloc(size);  
        if (pdst) {  
            if (rgbx8888_to_rgb888(psrc, w, h, pdst)) {  
                free(pdst);  
                pdst = NULL;  
            }  
        }  
    }  
    return pdst;  
}  
  
static const RGB_CONVERT_FUN g_convert_func[3][3] =   
{  
    {NULL, rgb565_to_rgb888_buffer, rgb565_to_rgbx8888_buffer},  
    {rgb888_to_rgb565_buffer, NULL, rgb888_to_rgbx8888_buffer},  
    {rgbx8888_to_rgb565_buffer, rgbx8888_to_rgb888_buffer, NULL}  
};  
  
RGB_CONVERT_FUN get_convert_func(int frombpp, int tobpp)  
{  
    RGB_CONVERT_FUN func_ptr = NULL;  
    frombpp = UpAlign8(frombpp) / 8 - 2;  
    tobpp = UpAlign8(tobpp) / 8 - 2;  
    if ((frombpp >= 0 && frombpp <= 2)  
        && (tobpp >= 0 && tobpp <= 2)) {  
        func_ptr = g_convert_func[frombpp][tobpp];  
    }  
    return func_ptr;  
}  

/*
出列帧缓冲后就可以调用该函数，rgb 需要先开辟大小为WIDTH * HEIGTH *  3的空间，
因为我们用的RGB是24位格式，3个字节分别代表一个像素点的R、G、B，
根据公式转换就好了。
*/

void yuyv_to_rgb24(unsigned char* yuv,unsigned char* rgb, int width, int height)  
{  
    unsigned int i;  
    unsigned char* y0 = yuv + 0;     
    unsigned char* u0 = yuv + 1;  
    unsigned char* y1 = yuv + 2;  
    unsigned char* v0 = yuv + 3;  
  
    unsigned  char* r0 = rgb + 0;  
    unsigned  char* g0 = rgb + 1;  
    unsigned  char* b0 = rgb + 2;  
    unsigned  char* r1 = rgb + 3;  
    unsigned  char* g1 = rgb + 4;  
    unsigned  char* b1 = rgb + 5;  
     
    float rt0 = 0, gt0 = 0, bt0 = 0, rt1 = 0, gt1 = 0, bt1 = 0;  
  
    for(i = 0; i <= (width * height) / 2 ;i++)  
    {  
        bt0 = 1.164 * (*y0 - 16) + 2.018 * (*u0 - 128);   
        gt0 = 1.164 * (*y0 - 16) - 0.813 * (*v0 - 128) - 0.394 * (*u0 - 128);   
        rt0 = 1.164 * (*y0 - 16) + 1.596 * (*v0 - 128);   
     
        bt1 = 1.164 * (*y1 - 16) + 2.018 * (*u0 - 128);   
        gt1 = 1.164 * (*y1 - 16) - 0.813 * (*v0 - 128) - 0.394 * (*u0 - 128);   
        rt1 = 1.164 * (*y1 - 16) + 1.596 * (*v0 - 128);   
      
        
                if(rt0 > 250)    rt0 = 255;  
        if(rt0< 0)       rt0 = 0;      
  
        if(gt0 > 250)    gt0 = 255;  
        if(gt0 < 0)  gt0 = 0;      
  
        if(bt0 > 250)    bt0 = 255;  
        if(bt0 < 0)  bt0 = 0;      
  
        if(rt1 > 250)    rt1 = 255;  
        if(rt1 < 0)  rt1 = 0;      
  
        if(gt1 > 250)    gt1 = 255;  
        if(gt1 < 0)  gt1 = 0;      
  
        if(bt1 > 250)    bt1 = 255;  
        if(bt1 < 0)  bt1 = 0;      
                      
        *r0 = (unsigned char)rt0;  
        *g0 = (unsigned char)gt0;  
        *b0 = (unsigned char)bt0;  
      
        *r1 = (unsigned char)rt1;  
        *g1 = (unsigned char)gt1;  
        *b1 = (unsigned char)bt1;  
  
        yuv = yuv + 4;  
        rgb = rgb + 6;  
        if(yuv == NULL)  
          break;  
  
        y0 = yuv;  
        u0 = yuv + 1;  
        y1 = yuv + 2;  
        v0 = yuv + 3;  
    
        r0 = rgb + 0;  
        g0 = rgb + 1;  
        b0 = rgb + 2;  
        r1 = rgb + 3;  
        g1 = rgb + 4;  
        b1 = rgb + 5;  
    }     
}
  
//-------------------------------------------------------------------  