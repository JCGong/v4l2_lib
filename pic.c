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
����:��RGB24ѹ����jpeg���������jpegͼƬ
����ֵ:-1:error 1:success
����:
		rgbbuffer: ��ѹ����RGB buf��ʼ��ַ
		jpeg_type.width:ѹ����jpeg�Ŀ��(����)
		jpeg_type.height: ѹ����jpeg�ĳ���(����)	    
		jpeg_type.jpegname:ѹ����jpegͼƬ������
ע��:RGB��width��height �����ѹ�����jpeg��width��heightһ����
		����ѹ�����jpegͼ����쳣

*/

typedef struct
{
	unsigned int width;  //ѹ�����jpeg width  ѹ��ǰ����ָ��
	unsigned int height;//ѹ�����jpeg height ѹ��ǰ����ָ��
	unsigned int depth; //ѹ�����jpeg pixfrome ;3��ʾRGB24
	unsigned char* jpeg_buffer;  //ѹ�������ڴ��jpeg���ݵ�ָ�� ֻ������rgb24_to_jpeg_to_mem
	unsigned long jpeg_size;   //ѹ����jpeg�Ĵ�С(byte)  ֻ������rgb24_to_jpeg_to_mem
	char* jpegname;  //ѹ����jpeg�ļ�����(����·��)ֻ������rgb24_to_jpeg_to_file ѹ��ǰ����ָ��
}jpeg_type, *pjpeg_type;

int rgb24_to_jpeg_to_file(unsigned char *rgbbuffer,pjpeg_type pjpeginfo)
{

	struct jpeg_compress_struct jcs;   //������һ����
	struct jpeg_error_mgr jem;         //������һ����
	FILE *fp;  
	JSAMPROW row_pointer[1]; // һ��λͼ����
	int row_stride; // ÿһ�е��ֽ���
	unsigned char* temprgbbuffer = rgbbuffer;
	

	jcs.err = jpeg_std_error(&jem);  //�󶨴���
	jpeg_create_compress(&jcs);

	fp = fopen(pjpeginfo->jpegname,"wb");
	if (fp==NULL)
	{	
		printf("open jpeg file error\n");
		return -1;
	}
	jpeg_stdio_dest(&jcs, fp);

	jcs.image_width = pjpeginfo->width; // λͼ�Ŀ�͸ߣ���λΪ����
	jcs.image_height = pjpeginfo->height;
	jcs.input_components = 3; // �ڴ�Ϊ1,��ʾ�Ҷ�ͼ�� ����ǲ�ɫλͼ����Ϊ3
	jcs.in_color_space = JCS_RGB;//JCS_GRAYSCALE��ʾ�Ҷ�ͼ��JCS_RGB��ʾ��ɫͼ��
	pjpeginfo->depth = jcs.input_components;
	//// ָ�����ȼ�ɫ������  
	//jpeg.q_scale_factor[0] = jpeg_quality_scaling(100);  
	//jpeg.q_scale_factor[1] = jpeg_quality_scaling(100);  
	//// ͼ������ʣ�Ĭ��Ϊ2 * 2  
	//jpeg.comp_info[0].v_samp_factor = 1;  
	//jpeg.comp_info[0].h_samp_factor = 1; 
	jpeg_set_defaults(&jcs);
	jpeg_set_quality (&jcs, 50, TRUE); //�м���ֵΪͼ��ѹ���ʣ�1-100��100������ã�1��ͣ�
	jpeg_start_compress(&jcs, TRUE); //Ĭ������

	row_stride = jcs.image_width * jcs.input_components;  
	while (jcs.next_scanline < jcs.image_height)
	{
	    row_pointer[0] = &temprgbbuffer[jcs.next_scanline * row_stride];  //дһ�����ݵ�jpg�ļ���
	    jpeg_write_scanlines(&jcs, row_pointer, 1);
	}
	
	jpeg_finish_compress(&jcs);
	jpeg_destroy_compress(&jcs);
	fclose(fp);
	return 1;
}

/*
����:��RGB888��ֵת����jpeg�ĸ�ʽ���������ڴ浱��
		#define JPEG ��ʾѹ��������һ��jpegͼƬ
����ֵ: -1:error   1:success
����:
		rgbbuffer: ��ѹ����RGB buf��ʼ��ַ
		jpeg_type.width:ѹ����jpeg�Ŀ��(����)
		jpeg_type.height: ѹ����jpeg�ĳ���(����)	    
		jpeg_type.jpeg_buffer:ѹ����jpeg���ݴ��buf
ע��:
	1.RGB��width��height �����ѹ�����jpeg��width��heightһ����
	����ѹ�����jpegͼ����쳣
	2.ѹ���Ĵ��ָ��jpeg��ʹ�ú�����ͷ�free(jpeg_buffer)

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
	  
	jpeg_mem_dest(&jcs, &pjpeginfo->jpeg_buffer, &pjpeginfo->jpeg_size);//�����ڴ���mallocһ���ڴ�jpegָ������ڴ�
	  
	jcs.image_width = pjpeginfo->width;  
	jcs.image_height = pjpeginfo->height;  

	jcs.input_components = 3;
	jcs.in_color_space = JCS_RGB;//JCS_GRAYSCALE;  
	pjpeginfo->depth = jcs.input_components;

	jpeg_set_defaults(&jcs);  
	jpeg_set_quality(&jcs, 50, TRUE);  
	  
	jpeg_start_compress(&jcs, TRUE);  
	row_stride =jcs.image_width * jcs.input_components;  

	while(jcs.next_scanline < jcs.image_height){//��ÿһ�н���ѹ��  
	    row_pointer[0] = (JSAMPROW)&temprgbbuffer[jcs.next_scanline * row_stride];  
	    (void)jpeg_write_scanlines(&jcs, row_pointer, 1);  
	}  
	jpeg_finish_compress(&jcs);  
	jpeg_destroy_compress(&jcs);  

#ifdef JPEG //jpeg ���棬������  
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
ʵ�����ת��Ч��������
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
����ת����ͼƬƫ��
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
����:��������Ӳ����jpegͼƬת����rgb24
����ֵ:-1:error 1:success
����:jpegpath:��Ҫ��ѹ��jpegͼƬ��
 		prgbinfo:ѹ����Ϣ����ṹ��,ת��ǰ�ȸ�ֵscale_num, scale_denom������Ա
 				��ת������д
 ע��:buffer_24��Ա������ʹ�����Ҫ�ͷŸ�ָ��ָ��ĵ�ַ
*/
/*
typedef struct
{
	unsigned int width;  //ת�����RGB width
	unsigned int height;//ת�����RGB height
	unsigned int depth; //ת�����RGB pixfrome ;3��ʾRGB24
	unsigned int scale_num;  //ת�����RGB�����ԭjpeg�ı���������ת��ǰ����ָ��
	unsigned int scale_denom;//ת�����RGB�����ԭjpeg�ı�������ĸת��ǰ����ָ��
	unsigned char* buffer_24;  //ת�������ڴ��rgb24���ݵ�ָ�� 
	unsigned long rgb24_size;   //ת���ɹ���rbg24�Ĵ�С(byte)
}rgb24_type, *prgb24_type;*/

int jpeg_to_rgb24_from_file(char* jpegpath, prgb24_type prgbinfo)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE           *infile;
	unsigned char *buffer_tmp;
	unsigned char *buffer;

	prgbinfo->buffer_24 = NULL;
	// ��jpg�ļ���
	if ((infile = fopen(jpegpath, "rb")) == NULL) 
	{
		fprintf(stderr,"open %s failed\n", jpegpath);
		return-1;
	}
	//�������
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	
	jpeg_stdio_src(&cinfo, infile);
	jpeg_read_header(&cinfo, TRUE);
	 //�趨��ѹ������
	 cinfo.scale_num=prgbinfo->scale_num;   //����
	 cinfo.scale_denom=prgbinfo->scale_denom;  //��ĸ
	 cinfo.out_color_space = JCS_RGB;  //���Ϊ�Ҷ�ͼ����Ϊ��ɫ��Ϊ��JCS_RGB;
	jpeg_start_decompress(&cinfo);
	prgbinfo->width  = cinfo.output_width;//ͼ����
	prgbinfo->height = cinfo.output_height;//ͼ��߶�
	prgbinfo->depth  = cinfo.output_components;//ͼ�����
	printf("%3d--%3d--%d\n",prgbinfo->width,prgbinfo->height,prgbinfo->depth); //ѹ�����RGB��width,height,depth

	//����һ���ڴ����ڴ�Ž�ѹ���RGB������
	buffer = (unsigned char *) malloc(cinfo.output_width * cinfo.output_height* cinfo.output_components);
	memset(buffer, 0, sizeof(unsigned char) *cinfo.output_width * cinfo.output_height*cinfo.output_components );//��0
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
����:���������ڴ��е�jpegͼƬת����rgb24
����ֵ:-1:error 1:success
����:jpegpath:��Ҫ��ѹ��jpegͼƬ��
 		prgbinfo:ѹ����Ϣ����ṹ��,ת��ǰ�ȸ�ֵscale_num, scale_denom������Ա
 				��ת������д
 ע��:buffer_24��Ա������ʹ�����Ҫ�ͷŸ�ָ��ָ��ĵ�ַ
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

	//���н������
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	 //���ڴ��ȡ
	jpeg_mem_src(&cinfo, tempjpegdata, tempjpegsize);
	jpeg_read_header(&cinfo, TRUE);
	 //�趨��ѹ���������˴����ǽ�ͼ��ߴ粻�䣬
	 cinfo.scale_num=prgbinfo->scale_num;   //����
	 cinfo.scale_denom=prgbinfo->scale_denom;  //��ĸ 
	 cinfo.out_color_space = JCS_RGB;  //���Ϊ�Ҷ�ͼ����Ϊ��ɫ��Ϊ��JCS_RGB;
	jpeg_start_decompress(&cinfo);
	prgbinfo->width  = cinfo.output_width;//ͼ����
	prgbinfo->height = cinfo.output_height;//ͼ��߶�
	prgbinfo->depth  = cinfo.output_components;//ͼ�����
	printf("%3d--%3d--%d\n",prgbinfo->width,prgbinfo->height,prgbinfo->depth); //ѹ�����RGB��width,height,depth
	//����һ���ڴ����ڴ�Ž�ѹ���RGB������
	buffer = (unsigned char *) malloc(cinfo.output_width * cinfo.output_height* cinfo.output_components);
	memset(buffer, 0, sizeof(unsigned char) *cinfo.output_width * cinfo.output_height*cinfo.output_components );//��0
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

	//����ʾ�豸  
	fbfd = open("/dev/fb0", O_RDWR);  
	if (!fbfd)  
	{  
	    printf("Error: cannot open framebuffer device.\n");  
	    exit(1);  
	}  

	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo))  
	{  
	    printf("Error��reading fixed information.\n");  
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

	//������Ļ���ܴ�С���ֽڣ�  
	screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;  
	printf("screensize=%d byte\n",screensize);  

	//����ӳ��  
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
	    //��ʾÿһ������  
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
	fbmem = open_framebuffer();//framebuffer�ĵ�ַ 

	rgb24_type rgbinfo;
	init_data(&rgbinfo);
	rgbinfo.scale_num = 1;
	rgbinfo.scale_denom = 2;
	//��Ӳ���е�jpegת��ΪRGB24 
	if (jpeg_to_rgb24_from_file(jpg_filename, &rgbinfo) == -1)
		printf("jpeg_to_rgb24_from_file error\n");

	//���ڴ��е�jpegת����RGB
		
	
	//RGB24ת����jpeg���������ļ���
	jpeg_type jpeginfo;
	init_data(&jpeginfo);
	jpeginfo.width = rgbinfo.width;
	jpeginfo.height = rgbinfo.height;
	jpeginfo.jpegname = "test1.jpg";
	ret = rgb24_to_jpeg_to_file(rgbinfo.buffer_24,&jpeginfo);

	//RGB24ת����jpeg���������ڴ���
	//pjpeg_type jpeginfo;
	jpeginfo.width = rgbinfo.width;
	jpeginfo.height = rgbinfo.height;
	//jpeginfo.jpeg_buffer = NULL;
	ret = rgb24_to_jpeg_to_mem(rgbinfo.buffer_24,&jpeginfo);

	//���ڴ��е�jpegת��λrgb24����ʾ����Ļ��
	rgb24_type rgbinfo2;
	init_data(&rgbinfo2);
	rgbinfo2.scale_num = 1;
	rgbinfo2.scale_denom = 1;
	if (jpeg_to_rgb24_from_mem(jpeginfo.jpeg_buffer, jpeginfo.jpeg_size, &rgbinfo2))
	//����Ļ����ʾͼ��	
	buffer_565 = RGB888toRGB565(rgbinfo2.buffer_24,rgbinfo2.width,rgbinfo2.height) ;//����õ����ݵĵ�ַ 
	//free(buffer_888);
	//printf("buffer_565 in %x\n",buffer_565);
	//5 ��ת��������д��framebuffer�����ʱ����Ļ�ͻ���ʾjpgͼƬ
	show_jpeg(buffer_565, rgbinfo2.width, rgbinfo2.height, 10, 0);	
	//�ͷ��ڴ��ֹ�ڴ�й©
	free(buffer_565);
	if (rgbinfo.buffer_24)
		free(rgbinfo.buffer_24);
	if (rgbinfo2.buffer_24)
		free(rgbinfo2.buffer_24);
	if (jpeginfo.jpeg_buffer)
		free(jpeginfo.jpeg_buffer);
}


