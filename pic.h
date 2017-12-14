#ifndef	_PIC_H_
#define	_PIC_H_

extern unsigned int  width;
extern unsigned int  height;
extern unsigned int  depth;
extern unsigned long screenline_size;
extern unsigned int screenwidth;
extern unsigned int screenheight;
extern unsigned int screendepth;
extern unsigned char  *fbmem;



#define UpAlign4(n) (((n) + 3) & ~3)  
#define UpAlign8(n) (((n) + 7) & ~7)  

typedef struct
{
	unsigned int width;  //转换后的RGB width
	unsigned int height;//转换后的RGB height
	unsigned int depth; //转换后的RGB pixfrome ;3表示RGB24
	unsigned int scale_num;  //转换后的RGB的相比原jpeg的比例，分子转换前必须指定
	unsigned int scale_denom;//转换后的RGB的相比原jpeg的比例，分母转换前必须指定
	unsigned char* buffer_24;  //转换后用于存放rgb24数据的指针 
	unsigned long rgb24_size;   //转换成功后rbg24的大小(byte)
}rgb24_type, *prgb24_type;



 unsigned char*RGB888toRGB565(char *RGB888, unsigned int width, unsigned int height);
unsigned char *  open_framebuffer();
void show_jpeg(char * buffer, unsigned int width, unsigned int height, unsigned int startx, unsigned int starty);
void test(void);
int jpeg_to_rgb24_from_mem(unsigned char* jpegdata, unsigned long jpegsize,prgb24_type prgbinfo);
void init_data(void* data);







#endif
