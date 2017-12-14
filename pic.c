#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include "jerror.h"
#include "jpeglib.h"
#include "pic.h"


unsigned int  width=0;
unsigned int  height=0;
unsigned int  depth=0;
unsigned long screenline_size=0;
unsigned int screenwidth = 0;
unsigned int screenheight = 0;
unsigned int screendepth = 0;
unsigned char  *fbmem;



#define UpAlign4(n) (((n) + 3) & ~3)  
#define UpAlign8(n) (((n) + 7) & ~7)  

/*
功能:将RGB24压缩成jpeg并保存成了jpeg图片
返回值:-1:error 1:success
参数:
		rgbbuffer: 待压缩的RGB buf起始地址
		jpeg_type.width:压缩后jpeg的宽度(像素)
		jpeg_type.height: 压缩后jpeg的长度(像素)	    
		jpeg_type.jpegname:压缩后jpeg图片的名字
注意:RGB的width和height 必须和压缩后的jpeg的width和height一样，
		否则压缩后的jpeg图像会异常

*/

typedef struct
{
	unsigned int width;  //压缩后的jpeg width  压缩前必须指定
	unsigned int height;//压缩后的jpeg height 压缩前必须指定
	unsigned int depth; //压缩后的jpeg pixfrome ;3表示RGB24
	unsigned char* jpeg_buffer;  //压缩后用于存放jpeg数据的指针 只适用于rgb24_to_jpeg_to_mem
	unsigned long jpeg_size;   //压缩后jpeg的大小(byte)  只适用于rgb24_to_jpeg_to_mem
	char* jpegname;  //压缩后jpeg文件名字(包含路径)只适用于rgb24_to_jpeg_to_file 压缩前必须指定
}jpeg_type, *pjpeg_type;

int rgb24_to_jpeg_to_file(unsigned char *rgbbuffer,pjpeg_type pjpeginfo)
{

	struct jpeg_compress_struct jcs;   //和上面一样；
	struct jpeg_error_mgr jem;         //和上面一样；
	FILE *fp;  
	JSAMPROW row_pointer[1]; // 一行位图数据
	int row_stride; // 每一行的字节数
	unsigned char* temprgbbuffer = rgbbuffer;
	

	jcs.err = jpeg_std_error(&jem);  //绑定错误集
	jpeg_create_compress(&jcs);

	fp = fopen(pjpeginfo->jpegname,"wb");
	if (fp==NULL)
	{	
		printf("open jpeg file error\n");
		return -1;
	}
	jpeg_stdio_dest(&jcs, fp);

	jcs.image_width = pjpeginfo->width; // 位图的宽和高，单位为像素
	jcs.image_height = pjpeginfo->height;
	jcs.input_components = 3; // 在此为1,表示灰度图， 如果是彩色位图，则为3
	jcs.in_color_space = JCS_RGB;//JCS_GRAYSCALE表示灰度图，JCS_RGB表示彩色图像
	pjpeginfo->depth = jcs.input_components;
	//// 指定亮度及色度质量  
	//jpeg.q_scale_factor[0] = jpeg_quality_scaling(100);  
	//jpeg.q_scale_factor[1] = jpeg_quality_scaling(100);  
	//// 图像采样率，默认为2 * 2  
	//jpeg.comp_info[0].v_samp_factor = 1;  
	//jpeg.comp_info[0].h_samp_factor = 1; 
	jpeg_set_defaults(&jcs);
	jpeg_set_quality (&jcs, 50, TRUE); //中间数值为图像压缩率，1-100；100画质最好，1最低；
	jpeg_start_compress(&jcs, TRUE); //默认设置

	row_stride = jcs.image_width * jcs.input_components;  
	while (jcs.next_scanline < jcs.image_height)
	{
	    row_pointer[0] = &temprgbbuffer[jcs.next_scanline * row_stride];  //写一行数据到jpg文件中
	    jpeg_write_scanlines(&jcs, row_pointer, 1);
	}
	
	jpeg_finish_compress(&jcs);
	jpeg_destroy_compress(&jcs);
	fclose(fp);
	return 1;
}

/*
功能:将RGB888的值转换成jpeg的格式并保存在内存当中
		#define JPEG 表示压缩后生成一张jpeg图片
返回值: -1:error   1:success
参数:
		rgbbuffer: 待压缩的RGB buf起始地址
		jpeg_type.width:压缩后jpeg的宽度(像素)
		jpeg_type.height: 压缩后jpeg的长度(像素)	    
		jpeg_type.jpeg_buffer:压缩后jpeg数据存放buf
注意:
	1.RGB的width和height 必须和压缩后的jpeg的width和height一样，
	否则压缩后的jpeg图像会异常
	2.压缩的存放指针jpeg在使用后必须释放free(jpeg_buffer)

*/

#define JPEG
int rgb24_to_jpeg_to_mem(unsigned char *rgbbuffer, pjpeg_type pjpeginfo)  
{  
	unsigned long jpeg_size;  
	struct jpeg_compress_struct jcs;  
	struct jpeg_error_mgr jem;  
	JSAMPROW row_pointer[1];  
	int row_stride;  
	unsigned char* temprgbbuffer = rgbbuffer;

	pjpeginfo->jpeg_buffer = NULL;
	jcs.err = jpeg_std_error(&jem);  
	jpeg_create_compress(&jcs);  
	  
	jpeg_mem_dest(&jcs, &pjpeginfo->jpeg_buffer, &pjpeginfo->jpeg_size);//会在内存中malloc一赌内存jpeg指向这段内存
	  
	jcs.image_width = pjpeginfo->width;  
	jcs.image_height = pjpeginfo->height;  

	jcs.input_components = 3;
	jcs.in_color_space = JCS_RGB;//JCS_GRAYSCALE;  
	pjpeginfo->depth = jcs.input_components;

	jpeg_set_defaults(&jcs);  
	jpeg_set_quality(&jcs, 50, TRUE);  
	  
	jpeg_start_compress(&jcs, TRUE);  
	row_stride =jcs.image_width * jcs.input_components;  

	while(jcs.next_scanline < jcs.image_height){//对每一行进行压缩  
	    row_pointer[0] = (JSAMPROW)&temprgbbuffer[jcs.next_scanline * row_stride];  
	    (void)jpeg_write_scanlines(&jcs, row_pointer, 1);  
	}  
	jpeg_finish_compress(&jcs);  
	jpeg_destroy_compress(&jcs);  

#ifdef JPEG //jpeg 保存，测试用  
	FILE *jpeg_fd;  

	jpeg_fd = fopen("jpeg.jpg","w");  
	if(jpeg_fd < 0 ){  
	    perror("open jpeg.jpg failed!\n");  
	    exit(-1);  
	}  
	  
	fwrite(pjpeginfo->jpeg_buffer, pjpeginfo->jpeg_size, 1, jpeg_fd);  
	close(jpeg_fd);  
#endif   
	return 1;  
}  


/*
实验过，转换效果还可以
*/
 unsigned char*RGB888toRGB565(char *RGB888, unsigned int width, unsigned int height)
{	
	int i;
	char *RGB565;
	unsigned short R,G,B,result;	
	RGB565 = (char*)malloc(width*height*2);
	for(i=0 ;i<width*height;i++)
	{
		B = ( RGB888[3*i+2]>> 3) & 0x001F;
		G = ((RGB888[3*i+1] >> 2) << 5) & 0x07E0;
		R = ((RGB888[3*i] >> 3) << 11) & 0xF800;
		result =  (R | G | B);
		RGB565[2*i] = result;
		RGB565[2*i+1] = result>>8;
	}
	return RGB565;
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

/*
经过转换的图片偏黄
*/
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

/*
功能:将保存在硬盘中jpeg图片转换成rgb24
返回值:-1:error 1:success
参数:jpegpath:需要解压的jpeg图片；
 		prgbinfo:压缩信息保存结构体,转换前先赋值scale_num, scale_denom其他成员
 				由转换后填写
 注意:buffer_24成员函数在使用完后要释放该指针指向的地址
*/
/*
typedef struct
{
	unsigned int width;  //转换后的RGB width
	unsigned int height;//转换后的RGB height
	unsigned int depth; //转换后的RGB pixfrome ;3表示RGB24
	unsigned int scale_num;  //转换后的RGB的相比原jpeg的比例，分子转换前必须指定
	unsigned int scale_denom;//转换后的RGB的相比原jpeg的比例，分母转换前必须指定
	unsigned char* buffer_24;  //转换后用于存放rgb24数据的指针 
	unsigned long rgb24_size;   //转换成功后rbg24的大小(byte)
}rgb24_type, *prgb24_type;*/

int jpeg_to_rgb24_from_file(char* jpegpath, prgb24_type prgbinfo)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE           *infile;
	unsigned char *buffer_tmp;
	unsigned char *buffer;

	prgbinfo->buffer_24 = NULL;
	// 把jpg文件打开
	if ((infile = fopen(jpegpath, "rb")) == NULL) 
	{
		fprintf(stderr,"open %s failed\n", jpegpath);
		return-1;
	}
	//解码过程
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	
	jpeg_stdio_src(&cinfo, infile);
	jpeg_read_header(&cinfo, TRUE);
	 //设定解压缩参数
	 cinfo.scale_num=prgbinfo->scale_num;   //分子
	 cinfo.scale_denom=prgbinfo->scale_denom;  //分母
	 cinfo.out_color_space = JCS_RGB;  //输出为灰度图像，若为彩色则为：JCS_RGB;
	jpeg_start_decompress(&cinfo);
	prgbinfo->width  = cinfo.output_width;//图像宽度
	prgbinfo->height = cinfo.output_height;//图像高度
	prgbinfo->depth  = cinfo.output_components;//图像深度
	printf("%3d--%3d--%d\n",prgbinfo->width,prgbinfo->height,prgbinfo->depth); //压缩后的RGB的width,height,depth

	//分配一块内存用于存放解压后的RGB的数据
	buffer = (unsigned char *) malloc(cinfo.output_width * cinfo.output_height* cinfo.output_components);
	memset(buffer, 0, sizeof(unsigned char) *cinfo.output_width * cinfo.output_height*cinfo.output_components );//清0
	buffer_tmp = buffer;

	while (cinfo.output_scanline < cinfo.output_height)
	{
		jpeg_read_scanlines(&cinfo, &buffer_tmp, 1);

	    buffer_tmp +=  cinfo.output_width *cinfo.output_components;	
	}

	jpeg_finish_decompress(&cinfo);	
	jpeg_destroy_decompress(&cinfo);

	fclose(infile);
	printf("jpeg decompress is success\n");
	prgbinfo->buffer_24 = (unsigned char*)buffer;
	prgbinfo->rgb24_size = prgbinfo->width * prgbinfo->height * prgbinfo->depth;
	return 1;
	//return (unsigned char*)buffer;
}

/*
功能:将保存在内存中的jpeg图片转换成rgb24
返回值:-1:error 1:success
参数:jpegpath:需要解压的jpeg图片；
 		prgbinfo:压缩信息保存结构体,转换前先赋值scale_num, scale_denom其他成员
 				由转换后填写
 注意:buffer_24成员函数在使用完后要释放该指针指向的地址
*/

int jpeg_to_rgb24_from_mem(unsigned char* jpegdata, unsigned long jpegsize,prgb24_type prgbinfo)
{
			
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	unsigned char *buffer_tmp;
	unsigned char *buffer;
	unsigned char* tempjpegdata = jpegdata;
	unsigned long tempjpegsize = jpegsize;
		
	prgbinfo->buffer_24 = NULL;

	//进行解码过程
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	 //从内存读取
	jpeg_mem_src(&cinfo, tempjpegdata, tempjpegsize);
	jpeg_read_header(&cinfo, TRUE);
	 //设定解压缩参数，此处我们将图像尺寸不变，
	 cinfo.scale_num=prgbinfo->scale_num;   //分子
	 cinfo.scale_denom=prgbinfo->scale_denom;  //分母 
	 cinfo.out_color_space = JCS_RGB;  //输出为灰度图像，若为彩色则为：JCS_RGB;
	jpeg_start_decompress(&cinfo);
	prgbinfo->width  = cinfo.output_width;//图像宽度
	prgbinfo->height = cinfo.output_height;//图像高度
	prgbinfo->depth  = cinfo.output_components;//图像深度
	printf("%3d--%3d--%d\n",prgbinfo->width,prgbinfo->height,prgbinfo->depth); //压缩后的RGB的width,height,depth
	//分配一块内存用于存放解压后的RGB的数据
	buffer = (unsigned char *) malloc(cinfo.output_width * cinfo.output_height* cinfo.output_components);
	memset(buffer, 0, sizeof(unsigned char) *cinfo.output_width * cinfo.output_height*cinfo.output_components );//清0
	buffer_tmp = buffer;

	while (cinfo.output_scanline < cinfo.output_height)
	{
		jpeg_read_scanlines(&cinfo, &buffer_tmp, 1);
		buffer_tmp +=  cinfo.output_width *cinfo.output_components;	
	}

	jpeg_finish_decompress(&cinfo);	
	jpeg_destroy_decompress(&cinfo);
	prgbinfo->buffer_24 = (unsigned char*)buffer;
	prgbinfo->rgb24_size = prgbinfo->width * prgbinfo->height * prgbinfo->depth;
	return 1;
}

void init_data(void* data)
{
	memset((char*)data,0,sizeof(*data));
}


unsigned char *  open_framebuffer()
{
	int fbfd = 0;  
	struct fb_var_screeninfo vinfo;  
	struct fb_fix_screeninfo finfo;  
	long int screensize = 0;  
	static char *fbp = 0;  
	static int xres = 0;  
	static int yres = 0;  
	static int bits_per_pixel = 0;  

	//打开显示设备  
	fbfd = open("/dev/fb0", O_RDWR);  
	if (!fbfd)  
	{  
	    printf("Error: cannot open framebuffer device.\n");  
	    exit(1);  
	}  

	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo))  
	{  
	    printf("Error：reading fixed information.\n");  
	    exit(2);  
	}  

	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo))  
	{  
	    printf("Error: reading variable information.\n");  
	    exit(3);  
	}  

	printf("R:%d,G:%d,B:%d \n", vinfo.red, vinfo.green, vinfo.blue );  

	printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel );  
	xres = vinfo.xres;  
	yres = vinfo.yres;  
	bits_per_pixel = vinfo.bits_per_pixel;  
	screenline_size = xres * bits_per_pixel/8;
	screenwidth = xres;
	screenheight = yres;
	screendepth = bits_per_pixel/8;

	//计算屏幕的总大小（字节）  
	screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;  
	printf("screensize=%d byte\n",screensize);  

	//对象映射  
	fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);  
	if ((int)fbp == -1)  
	{  
	    printf("Error: failed to map framebuffer device to memory.\n");  
	    exit(4);  
	}  

	printf("fbp in %x\n",fbp);
	return fbp;

}



void show_jpeg(char * buffer, unsigned int width, unsigned int height, unsigned int startx, unsigned int starty)
{
	//int startx = 100;
	//int starty = 10;
	int startxoffset = 0;
	int startyoffset = 0; 
	unsigned long location = 0;
	unsigned long imageoffset = 0;
	while(1)
	{
	  location = ((startx+startxoffset)* 2) + ((starty+startyoffset)*screenline_size);  
	    //显示每一个像素  
	    *(fbmem + location + 0)=*(buffer+(imageoffset++));  
	    *(fbmem + location + 1)=*(buffer+(imageoffset++));  
	   // *(fbp + location + 2)=pix.red;  buffer
	    //*(fbp + location + 3)=pix.reserved;  
				
	   startxoffset ++;
	    if ((startxoffset >= width) || startxoffset + startx>= 480)  
	    {  		    	
	        startxoffset = 0;  
	        startyoffset++;  	
		imageoffset = width*startyoffset*2;
	        if((startyoffset >=height) || startyoffset+starty >= 272)  
	            break;  
	    }  
	}

}


void test(void)
{
	char jpg_filename[20]="test.jpg";
	unsigned char  *buffer_888,*buffer_565, *jpeg; 
	unsigned long ret=0;
	fbmem = open_framebuffer();//framebuffer的地址 

	rgb24_type rgbinfo;
	init_data(&rgbinfo);
	rgbinfo.scale_num = 1;
	rgbinfo.scale_denom = 2;
	//将硬盘中的jpeg转换为RGB24 
	if (jpeg_to_rgb24_from_file(jpg_filename, &rgbinfo) == -1)
		printf("jpeg_to_rgb24_from_file error\n");

	//将内存中的jpeg转换成RGB
		
	
	//RGB24转换到jpeg并保存在文件中
	jpeg_type jpeginfo;
	init_data(&jpeginfo);
	jpeginfo.width = rgbinfo.width;
	jpeginfo.height = rgbinfo.height;
	jpeginfo.jpegname = "test1.jpg";
	ret = rgb24_to_jpeg_to_file(rgbinfo.buffer_24,&jpeginfo);

	//RGB24转换到jpeg并保存在内存中
	//pjpeg_type jpeginfo;
	jpeginfo.width = rgbinfo.width;
	jpeginfo.height = rgbinfo.height;
	//jpeginfo.jpeg_buffer = NULL;
	ret = rgb24_to_jpeg_to_mem(rgbinfo.buffer_24,&jpeginfo);

	//将内存中的jpeg转换位rgb24并显示在屏幕上
	rgb24_type rgbinfo2;
	init_data(&rgbinfo2);
	rgbinfo2.scale_num = 1;
	rgbinfo2.scale_denom = 1;
	if (jpeg_to_rgb24_from_mem(jpeginfo.jpeg_buffer, jpeginfo.jpeg_size, &rgbinfo2))
	//在屏幕上显示图像	
	buffer_565 = RGB888toRGB565(rgbinfo2.buffer_24,rgbinfo2.width,rgbinfo2.height) ;//解码好的数据的地址 
	//free(buffer_888);
	//printf("buffer_565 in %x\n",buffer_565);
	//5 将转化的数据写入framebuffer，这个时候屏幕就会显示jpg图片
	show_jpeg(buffer_565, rgbinfo2.width, rgbinfo2.height, 10, 0);	
	//释放内存防止内存泄漏
	free(buffer_565);
	if (rgbinfo.buffer_24)
		free(rgbinfo.buffer_24);
	if (rgbinfo2.buffer_24)
		free(rgbinfo2.buffer_24);
	if (jpeginfo.jpeg_buffer)
		free(jpeginfo.jpeg_buffer);
}


