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
	unsigned int width;  //ת�����RGB width
	unsigned int height;//ת�����RGB height
	unsigned int depth; //ת�����RGB pixfrome ;3��ʾRGB24
	unsigned int scale_num;  //ת�����RGB�����ԭjpeg�ı���������ת��ǰ����ָ��
	unsigned int scale_denom;//ת�����RGB�����ԭjpeg�ı�������ĸת��ǰ����ָ��
	unsigned char* buffer_24;  //ת�������ڴ��rgb24���ݵ�ָ�� 
	unsigned long rgb24_size;   //ת���ɹ���rbg24�Ĵ�С(byte)
}rgb24_type, *prgb24_type;



 unsigned char*RGB888toRGB565(char *RGB888, unsigned int width, unsigned int height);
unsigned char *  open_framebuffer();
void show_jpeg(char * buffer, unsigned int width, unsigned int height, unsigned int startx, unsigned int starty);
void test(void);
int jpeg_to_rgb24_from_mem(unsigned char* jpegdata, unsigned long jpegsize,prgb24_type prgbinfo);
void init_data(void* data);







#endif
