/******************************************************************** 
    created:    2012/04/07 
    filename:   mybmp.h 
    author:      
     
    purpose:     
*********************************************************************/  
  
#ifndef _mybmp_h__  
#define _mybmp_h__  
//-------------------------------------------------------------------  
  
#ifdef __cplusplus  
extern "C" {  
#endif  
  
#define UpAlign4(n) (((n) + 3) & ~3)  
#define UpAlign8(n) (((n) + 7) & ~7)  
  
//拷贝数据  
void rgb_copy(const void * psrc, void * pdst, int sw, int sh, int dw, int dh, int bpp);  
void rbg565_copy(const void * psrc, void * pdst, int sw, int sh, int dw, int dh);  
void rbg888_copy(const void * psrc, void * pdst, int sw, int sh, int dw, int dh);  
void rbgx8888_copy(const void * psrc, void * pdst, int sw, int sh, int dw, int dh);  
  
//行数据翻转  
void line_reversal(void * pdata, int w, int h, int bpp);  
void rgb565_line_reversal(void * p565, int w, int h);  
void rgb888_line_reversal(void * p888, int w, int h);  
void rgbx8888_line_reversal(void * p8888, int w, int h);  
  
//转换  
typedef void * (* RGB_CONVERT_FUN)(const void * psrc, int w, int h);  
void * rgb565_to_rgb888_buffer(const void * psrc, int w, int h);  
void * rgb888_to_rgb565_buffer(const void * psrc, int w, int h);  
void * rgb565_to_rgbx8888_buffer(const void * psrc, int w, int h);  
void * rgbx8888_to_rgb565_buffer(const void * psrc, int w, int h);  
void * rgb888_to_rgbx8888_buffer(const void * psrc, int w, int h);  
void * rgbx8888_to_rgb888_buffer(const void * psrc, int w, int h);  
RGB_CONVERT_FUN get_convert_func(int frombpp, int tobpp);  
  
#ifdef __cplusplus  
};  
#endif  



































