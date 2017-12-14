#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>           
#include <fcntl.h>            
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>        
#include <linux/videodev2.h>

#include "v4l2_operation_lib.h"
#include "pic.h"

struct buffer *tempbuffers;

static void errno_exit(const char *s)
{
    fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
}
/*
static unsigned long long _get_systime_us()
{
    struct timespec ts;

    clock_gettime(1, &ts);
    return (unsigned long long) ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

void zm_set_tick_count_us(unsigned long long now)
{
    start_us = (unsigned long long)(_get_systime_us() - now);
    start_us_inited = 1;
}

unsigned long long zm_get_tick_count_us()
{
    unsigned long long now;
    if (!start_us_inited)
        zm_set_tick_count_us(0);
    now = _get_systime_us();
    return (unsigned long long)(now - start_us);
}

unsigned int zm_get_tick_count()
{
    return (unsigned int)((zm_get_tick_count_us() * MAX_U32_US_TO_MS) >> MAX_U32_SHIFT);
}*/
  

int jc_v4l2_open(char *path)
{	
	int fd = 0;
	
	fd = open (path, O_RDWR | O_NONBLOCK, 0);  //非阻塞模式打开
	if (fd < 0)
		printf("open v4l2 dev error [%s]\n", path);
	return fd;
}

int jc_v4l2_query_capability_info(int fd)
{
	struct v4l2_capability cap;
	
	if (ioctl (fd, VIDIOC_QUERYCAP, &cap) < 0)
	{
		errno_exit("v4l2 query capability");
		return JC_ERROR;
	}
	printf("v4l2 info:Driver Name:%s....Card Name:%s....Bus info:%s....Driver Version:%u.%u.%u\n",
	cap.driver,cap.card,cap.bus_info,(cap.version>>16)&0XFF,(cap.version>>8)&0XFF,cap.version&0XFF);

	if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
		printf("v4l2 support:V4L2_CAP_VIDEO_CAPTURE\n");
	else
		printf("v4l2 not support:V4L2_CAP_VIDEO_CAPTURE\n");

	if (cap.capabilities & V4L2_CAP_STREAMING)
		printf("v4l2 support:V4L2_CAP_STREAMING\n");
	else
		printf("v4l2 not support:V4L2_CAP_STREAMING\n");

	if (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)
		printf("v4l2 support:V4L2_CAP_VIDEO_OUTPUT\n");
	else
		printf("v4l2 not support:V4L2_CAP_VIDEO_OUTPUT\n");

	if (cap.capabilities & V4L2_CAP_VIDEO_OVERLAY)
		printf("v4l2 support:V4L2_CAP_VIDEO_OVERLAY\n");
	else
		printf("v4l2 not support:V4L2_CAP_VIDEO_OVERLAY\n");

	if (cap.capabilities & V4L2_CAP_VBI_CAPTURE)
		printf("v4l2 support:V4L2_CAP_VBI_CAPTURE\n");
	else
		printf("v4l2 not support:V4L2_CAP_VBI_CAPTURE\n");

	if (cap.capabilities & V4L2_CAP_VBI_OUTPUT)
		printf("v4l2 support:V4L2_CAP_VBI_OUTPUT\n");
	else
		printf("v4l2 not support:V4L2_CAP_VBI_OUTPUT\n");

	if (cap.capabilities & V4L2_CAP_SLICED_VBI_CAPTURE)
		printf("v4l2 support:V4L2_CAP_SLICED_VBI_CAPTURE\n");
	else
		printf("v4l2 not support:V4L2_CAP_SLICED_VBI_CAPTURE\n");

	if (cap.capabilities & V4L2_CAP_SLICED_VBI_OUTPUT)
		printf("v4l2 support:V4L2_CAP_SLICED_VBI_OUTPUT\n");
	else
		printf("v4l2 not support:V4L2_CAP_SLICED_VBI_OUTPUT\n");

	if (cap.capabilities & V4L2_CAP_RDS_CAPTURE)
		printf("v4l2 support:V4L2_CAP_RDS_CAPTURE\n");
	else
		printf("v4l2 not support:V4L2_CAP_RDS_CAPTURE\n");

	if (cap.capabilities & V4L2_CAP_AUDIO)
		printf("v4l2 support:V4L2_CAP_AUDIO\n");
	else
		printf("v4l2 not support:V4L2_CAP_AUDIO\n");
	
	return JC_SUCCESS;
}

void jc_v4l2_query_support_image_format(int fd)
{	
	struct v4l2_fmtdesc fmtdesc;

	CLEAR(fmtdesc);
	fmtdesc.index=0;  
	fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;  
	printf("Supportformat:\n");  
	while(ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc)!=-1)  
	{  
		printf("	%d.%s pixelformat:%d\n",fmtdesc.index, fmtdesc.description, 
			fmtdesc.pixelformat);  
		fmtdesc.index++;  
	} 
}

int enum_frame_formats(int fd) // dev是利用open打开的设备文件描述符
{
    int ret;
    struct v4l2_fmtdesc fmt;

    memset(&fmt, 0, sizeof(fmt));
    fmt.index = 0;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	printf("Supportformat info:\n"); 
    while ((ret = ioctl(fd, VIDIOC_ENUM_FMT, &fmt)) == 0) {
        fmt.index++;
        printf("{		pixelformat = '%c%c%c%c', description = '%s' }\n",
                fmt.pixelformat & 0xFF, (fmt.pixelformat >> 8) & 0xFF,
                (fmt.pixelformat >> 16) & 0xFF, (fmt.pixelformat >> 24) & 0xFF,
                fmt.description);
       // ret = enum_frame_sizes(dev, fmt.pixelformat); // 列举该格式下的帧大小
       // if (ret != 0)
           // printf("  Unable to enumerate frame sizes.\n");
    }
    if (errno != EINVAL) {
        printf("ERROR enumerating frame formats: %d\n", errno);
        return errno;
    }
	printf("Supportformat info query success:\n"); 
    return 0;
}

//检查是否支持某种帧格式
int jc_v4l2_pixelformat_is_support(int fd, u32 pixelformat)
{
	struct v4l2_format fmt;
	
	fmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;  
	fmt.fmt.pix.pixelformat=pixelformat;  
	if(ioctl(fd,VIDIOC_TRY_FMT,&fmt)==-1)  
	{
		if(errno==EINVAL)  
		{
			printf("notsupport pixelformat %d!\n", pixelformat);  
			return JC_NO;
		}
	}
	 printf("v4l2 support pixelformat = '%c%c%c%c'\n",
	            pixelformat & 0xFF, (pixelformat >> 8) & 0xFF,
	            (pixelformat >> 16) & 0xFF, (pixelformat >> 24) & 0xFF);
	return JC_YES;
}
//获取当前帧的相关信息
int jc_v4l2_get_current_pixelformat(int fd, struct v4l2_format *format)
{	
	struct v4l2_format fmt;
	
	fmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;  
	if (ioctl(fd,VIDIOC_G_FMT,&fmt) < 0)
	{	
		errno_exit("get current pixelformat error");
		return JC_ERROR;
	}
	printf("get current pixelformat info:\n");
	printf("	Currentdata format information:width:%d,height:%d,field:%d, pixelformat:%d,bytesperline:%d,sizeimage:%d\n",
		fmt.fmt.pix.width,fmt.fmt.pix.height, fmt.fmt.pix.field,fmt.fmt.pix.pixelformat,fmt.fmt.pix.bytesperline, fmt.fmt.pix.sizeimage);  
	strncpy((char *)format, (char *)&fmt, sizeof(fmt));
	return JC_SUCCESS;
}

//设置图像格式
int jc_v4l2_set_mjpeg_pixelformat(int fd, u32 width, u32 height)
{	
	struct v4l2_format fmt;
	 
	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width    = width;
	fmt.fmt.pix.height   = height;
	fmt.fmt.pix.field      = V4L2_FIELD_INTERLACED;
	//fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;  //V4L2_PIX_FMT_YUYV
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV; 
	printf("set field to filed:%d\n",fmt.fmt.pix.field);
	printf("set mjpeg pixelformat to:%c%c%c%c\n", 
	        fmt.fmt.pix.pixelformat & 0xFF,
	        (fmt.fmt.pix.pixelformat >> 8) & 0xFF,   
	        (fmt.fmt.pix.pixelformat >> 16) & 0xFF,
	        (fmt.fmt.pix.pixelformat >> 24) & 0xFF); 
	if (ioctl (fd, VIDIOC_S_FMT, &fmt) < 0)	
	{

		errno_exit("set pixelformat error");
		return JC_ERROR;
	}
	printf("set mjpeg pixelformat is success pixelformat:%c%c%c%c\n", 
	        fmt.fmt.pix.pixelformat & 0xFF,
	        (fmt.fmt.pix.pixelformat >> 8) & 0xFF,   
	        (fmt.fmt.pix.pixelformat >> 16) & 0xFF,
	        (fmt.fmt.pix.pixelformat >> 24) & 0xFF); 
	printf("set field is success filed:%d\n",fmt.fmt.pix.field); 
	return JC_SUCCESS;
}

//设置图像格式
int jc_v4l2_get_mjpeg_pixelformat(int fd, u32 *width, u32 *height)
{	
	struct v4l2_format fmt;
	
	fmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;  
	if (ioctl(fd,VIDIOC_G_FMT,&fmt) < 0)
	{	
		errno_exit("get mjpeg pixelformat error");
	}
	printf("get mjpeg pixelformat info:\n");
	printf("	fmt.type:%d\n",fmt.type);   
	printf("	pix.pixelformat:%c%c%c%c\n", 
	        fmt.fmt.pix.pixelformat & 0xFF,
	        (fmt.fmt.pix.pixelformat >> 8) & 0xFF,   
	        (fmt.fmt.pix.pixelformat >> 16) & 0xFF,
	        (fmt.fmt.pix.pixelformat >> 24) & 0xFF);  
	printf("	pix.width:%d\n",fmt.fmt.pix.width);  
	printf("	pix.height:%d\n",fmt.fmt.pix.height);  
	printf("	pix.field:%d\n",fmt.fmt.pix.field);  
	
	*width = fmt.fmt.pix.width;
	*height = fmt.fmt.pix.height;
	return JC_SUCCESS;
}


int jc_v4l2_query_stream_info(int fd)
{
	struct v4l2_streamparm stream_parm;
	struct v4l2_captureparm capture;
	int frame_num = 0;

	memset(&stream_parm, 0, sizeof(struct v4l2_streamparm));
	stream_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
	
	if (ioctl(fd, VIDIOC_G_PARM, &stream_parm) == -1)
	{	
		if (errno == EINVAL)
			printf("not support VIDIOC_G_PARM\n");
		else
			{
			//errno_exit("get stream info");
			printf("get stream info error\n");
		return -1;
			}
	}
	capture = stream_parm.parm.capture;
	printf("v4l2 stream info:\n");
	if (capture.capability & V4L2_CAP_TIMEPERFRAME)
		printf("	frame num is support set\n");
	else
		printf("	frame num is not support set\n");
	printf("	capture.capability:%d\n", capture.capability);
	
	if (capture.capturemode & V4L2_MODE_HIGHQUALITY)
		printf("	camera current is high definition mode\n");
	else
		printf("	camera current is not high definition mode\n");
	printf("	capture.capturemode:%d\n", capture.capturemode);
	
	//frame_num = capture.timeperframe.denominator / capture.timeperframe.numerator;
	printf("	 width/height:%d/%d\n", capture.timeperframe.denominator, capture.timeperframe.numerator);
	
	printf("get steam info success\n");
	//strncpy((char*)stream_info, (char*)&stream_parm, sizeof(stream_parm));
	return JC_SUCCESS;

}


int jc_v4l2_get_stream_framenum(int fd, u32 *frame_num)
{	
	
	struct v4l2_streamparm stream_parm;
	u32 temp_frame_num = 0;
	
	memset(&stream_parm, 0, sizeof(struct v4l2_streamparm));
	stream_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 

	if (ioctl(fd, VIDIOC_G_PARM, &stream_parm) < 0)
		errno_exit("get stream frame num error");
	
	temp_frame_num = stream_parm.parm.capture.timeperframe.denominator / stream_parm.parm.capture.timeperframe.numerator;
	*frame_num = temp_frame_num;
	printf("get stream frame num success frame num:\n", frame_num);
	
	return JC_SUCCESS;
}


int jc_v4l2_set_stream_framenum(int fd, u32 frame_num)
{	
	
	struct v4l2_streamparm stream_parm;
	
	memset(&stream_parm, 0, sizeof(struct v4l2_streamparm));
	stream_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 

	stream_parm.parm.capture.timeperframe.denominator = frame_num;;
	stream_parm.parm.capture.timeperframe.numerator = 1;

	if (ioctl(fd, VIDIOC_S_PARM, &stream_parm) < 0)
		errno_exit("set stream frame num error");
	printf("set stream frame num success\n");
	return JC_SUCCESS;
}
//获取当前视频输入的节点
int jc_v4l2_get_input_index(int fd, u32 *index)
{
	struct v4l2_input input;

	memset (&input, 0, sizeof (input));

	//首先获得当前输入的 index,注意只是 index，要获得具体的信息，就的调用列举操作

	if (-1 == ioctl (fd, VIDIOC_G_INPUT, &input.index))
		errno_exit("get V4L2 input index error");
	printf("current v4l2 input index:%d\n", input.index);
	*index = input.index;
	return JC_SUCCESS;
}
//设置当前视频输入的节点
int jc_v4l2_set_input_index(int fd, u32 index)
{
	struct v4l2_input input;

	memset (&input, 0, sizeof (input));
	input.index = index;

	if (-1 == ioctl (fd, VIDIOC_S_INPUT, &input.index))
		errno_exit("set V4L2 input index error");
	printf("set v4l2 input index success\n");
	return JC_SUCCESS;
}

//获取当前视频输出的节点
int jc_v4l2_get_output_index(int fd, u32 *index)
{
	struct v4l2_output output;

	memset (&output, 0, sizeof (output));

	if (-1 == ioctl (fd, VIDIOC_G_OUTPUT, &output.index))
		errno_exit("get V4L2 output index error");
	printf("current v4l2 output index:%d\n", output.index);
	*index = output.index;
	return JC_SUCCESS;
}
//设置当前视频输出的节点
int jc_v4l2_set_output_index(int fd, u32 index)
{
	struct v4l2_output output;

	memset (&output, 0, sizeof (output));
	output.index = index;

	if (-1 == ioctl (fd, VIDIOC_S_OUTPUT, &output.index))
		errno_exit("set V4L2 output index error");
	printf("set v4l2 output index success\n");
	return JC_SUCCESS;
}

//查询当前输入支持的所有标准
void jc_v4l2_query_current_input_standard(int fd, u32 index)
{
	struct v4l2_input input;
	struct v4l2_standard standard;

	memset (&input, 0, sizeof (input));
	if (-1 == ioctl (fd, VIDIOC_G_INPUT, &input.index)) 
		errno_exit("VIDIOC_G_INPUT error");

	//调用列举操作，获得 input.index 对应的输入的具体信息
	input.index = index;
	if (-1 == ioctl (fd, VIDIOC_ENUMINPUT, &input)) 
		errno_exit("VIDIOC_ENUMINPUT error");

	printf ("current input %s supports:\n", input.name); 
	memset (&standard, 0, sizeof (standard)); standard.index = 0;

	//列举所有的所支持的 standard，如果 standard.id 与当前 input 的 input.std 有共同的
	//bit flag，意味着当前的输入支持这个 standard,这样将所有驱动所支持的 standard 列举一个
	//遍，就可以找到该输入所支持的所有 standard 了。
	while (0 == ioctl (fd, VIDIOC_ENUMSTD, &standard))  //返回值为0表示函数成功，为EINVAL表示所有的STD以枚举完成
	{
		if (standard.id & input.std)
			printf ("	%s\n", standard.name);
		standard.index++;
	}

	/* EINVAL indicates the end of the enumeration, which cannot be empty unless this device falls under the USB exception. */
	if (errno != EINVAL || standard.index == 0) 
		errno_exit("VIDIOC_ENUMSTD");

}
//查询摄像头增益值的设置范围
void jc_v4l2_query_camera_gain_range(int fd)
{
	struct v4l2_queryctrl  setting;
	
	setting.id = V4L2_CID_GAIN;

	if (ioctl(fd, VIDIOC_QUERYCTRL, &setting) <0 )
		errno_exit("query camera gain error");
	printf("camera gain range info:\n");
	printf("	name:%s", setting.name);
	printf("	minimum:%s", setting.minimum);
	printf("	maximum:%s", setting.maximum);
	printf("	step:%s", setting.step);
	printf("	default_value:%s", setting.default_value);
	printf("	flags:%s", setting.flags);
}

//获取摄像头增益当前值
int jc_v4l2_get_camera_gain(int fd, u32 *gain)
{
	struct v4l2_control ctrl;
	
	ctrl.id = V4L2_CID_GAIN;
	
	if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0)
		errno_exit("get camera gain error");
	
	*gain = ctrl.value; 
	printf("camera gain:%d\n", *gain);
	return JC_SUCCESS;
}

//设置摄像头增益值
int jc_v4l2_set_camera_gain(int fd, u32 gain)
{
	struct v4l2_control ctrl;
	
	ctrl.id = V4L2_CID_GAIN;
	ctrl.value = gain;
	
	if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0)
		errno_exit("set camera gain error");
	printf("set camera gain succes\n");
	return JC_SUCCESS;
}

//查询设备的图像裁剪能力
void jc_v4l2_query_corpcap_info(int fd)
{	
	struct v4l2_cropcap cropcap;
	
	memset (&cropcap, 0, sizeof (cropcap));
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl (fd, VIDIOC_CROPCAP, &cropcap) < 0)
		errno_exit("query cropcap info error");
	printf("cropcap info:\n");
	printf("	image max para:left:%d, top:%d, width:%d, height:%d\n", 
		cropcap.bounds.left, cropcap.bounds.top, cropcap.bounds.width, cropcap.bounds.height);
	printf("	image default para:left:%d, top:%d, width:%d, height:%d\n", 
		cropcap.defrect.left, cropcap.defrect.top, cropcap.defrect.width, cropcap.defrect.height);
	printf("	width/height: %d/%d", cropcap.pixelaspect.numerator, cropcap.pixelaspect.denominator);

}

//设置图像矩形边框到默认

int jc_v4l2_set_crop_default(int fd)
{
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
        
    CLEAR(cropcap);
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (0 == ioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
        CLEAR(crop);
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect;
        if (-1 == ioctl(fd, VIDIOC_S_CROP, &crop)) {
            printf("set crop error!\n");
		errno_exit("set crop error!\n");
        }
    } else {
		  errno_exit("get crop capability error");
    }

    return JC_SUCCESS;
}


//获取图像裁剪参数
int jc_v4l2_get_crop_para(int fd, struct v4l2_rect *rect)
{	
	struct v4l2_crop crop;
	CLEAR(crop);
	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl(fd, VIDIOC_G_CROP, &crop)) 
		errno_exit("get crop para error");
	
	printf("get crop para success");
	strncpy((char*)rect,(char*)&crop.c,sizeof(crop.c));
	return JC_SUCCESS;	
}
//设置图像裁剪参数
int jc_v4l2_set_crop_para(int fd, struct v4l2_rect * rect)
{	
	struct v4l2_crop crop;
	
	memset (&crop, 0, sizeof (crop));

	crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	crop.c = *rect;
	
	if (-1 == ioctl (fd, VIDIOC_S_CROP, &crop))
		errno_exit("set crop error");
	printf("set crop success");
	return JC_SUCCESS;
}

//将获取的图像大小设置到crop 默认参数参数的一半大小
int jc_v4l2_set_image_4point_default_crop(int fd)
{
	struct v4l2_cropcap cropcap;
	struct v4l2_format format;

	CLEAR(cropcap);
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 == ioctl(fd, VIDIOC_CROPCAP, &cropcap)) 
		errno_exit("get crop error");
	
	/* Scale down to 1/4 size of full picture. */
	memset (&format, 0, sizeof (format)); /* defaults */
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width = cropcap.defrect.width >> 1;
	format.fmt.pix.height = cropcap.defrect.height >> 1;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;

	if (-1 == ioctl (fd, VIDIOC_S_FMT, &format)) 
		errno_exit("image not support scanl");
	return JC_SUCCESS;
}

//将取景区域设置为默认crop 参数的一半，中心不变
int jc_v4l2_set_crop_2point_default_crop(int fd)
{
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;

	memset (&cropcap, 0, sizeof (cropcap));
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == ioctl (fd, VIDIOC_CROPCAP, &cropcap)) 
		errno_exit("set_crop_2point_default_crop error");
	
	memset (&crop, 0, sizeof (crop));
	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	crop.c = cropcap.defrect;

	/* Scale the width and height to 50 % of their original size
	and center the output. */

	crop.c.width /= 2;
	crop.c.height /= 2;
	crop.c.left += crop.c.width / 2;
	crop.c.top += crop.c.height / 2;

	/* Ignore if cropping is not supported (EINVAL). */

	if (-1 == ioctl (fd, VIDIOC_S_CROP, &crop) && errno != EINVAL) 
		errno_exit("set_crop_2point_default_crop error");
	return JC_SUCCESS;
}
//向内核申请帧缓存buff只有使用内存映射方式才有必要
int jc_v4l2_reqbuf(int fd, int buf_count)
{	
	
	struct v4l2_requestbuffers req; 

	req.count=buf_count; 
	req.type=V4L2_BUF_TYPE_VIDEO_CAPTURE; 
	req.memory=V4L2_MEMORY_MMAP; 

	if (ioctl(fd,VIDIOC_REQBUFS,&req) < 0)
		errno_exit("req image buff error");
	if (req.count < 2) 
		printf("Insufficient buffer memory\n");
	printf("req success image buf count:%d", req.count);
	return buf_count;
}

int jc_v4l2_init_mmap(int fd, int buf_count, struct buffer *buffers)
{	
	struct v4l2_buffer buf;
	int i = 0;

	tempbuffers = (struct buffer*)calloc(buf_count, sizeof(*tempbuffers));

	if (!tempbuffers) 
		errno_exit("calloc buf error");

	for (i = 0; i < buf_count; ++i) 
	{
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		if (-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf)) 
			errno_exit("query buf error");

		tempbuffers[i].length = buf.length;
		tempbuffers[i].start = mmap(NULL, // start anywhere
		 				buf.length,
						PROT_READ | PROT_WRITE,
		 				MAP_SHARED,
		 				fd, buf.m.offset);              
		if(MAP_FAILED == tempbuffers[i].start) 
			errno_exit("mmap error");
	}
	return JC_SUCCESS;
}

//开始捕获图像
int jc_v4l2_start_capturing(int fd, int buf_count)
{
	unsigned int i;//fd_v4l2_fb;
	enum v4l2_buf_type type;
	struct v4l2_buffer buf;
	//将buf放入缓存
	for (i = 0; i < buf_count; ++i) 
	{
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory =V4L2_MEMORY_MMAP;
		buf.index = i;
		if (-1 == ioctl(fd, VIDIOC_QBUF, &buf)) 
			errno_exit("VIDIOC_QBUF error");
	}
	//开始捕获
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl(fd, VIDIOC_STREAMON, &type)) 
		errno_exit("start capturing error");

	return JC_SUCCESS;
}

int jc_v4l2_stop_capturing(int fd)
{
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 == ioctl(fd, VIDIOC_STREAMOFF, &type)) 
		errno_exit("stop stream error");
	
    return JC_SUCCESS;
}

int jc_v4l2_uninit_mmap(int buf_count, struct buffer *buffers)
{	
	
	int i;
	
	for (i= 0; i< buf_count; ++i) 
	{
		if (-1 == munmap(buffers[i].start, buffers[i].length)) 
			errno_exit("munmap error");
	}
	free(buffers);
	return JC_SUCCESS;
}


int jc_v4l2_capture_mjpeg(int fd, struct buffer *buffers, int file_fd)
{
	struct v4l2_buffer buf;
	int ret = -1;
	fd_set fds;
	FD_ZERO (&fds);
	FD_SET (fd, &fds);

	ret = select(fd + 1, &fds, NULL, NULL, NULL);
	if (ret < 0)
		errno_exit("select error");
	/*帧出列*/
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if (ioctl (fd, VIDIOC_DQBUF, &buf) < 0)
		errno_exit("read image data error");

	//对读到的帧数据进行处理
	write(file_fd,tempbuffers[buf.index].start,tempbuffers[buf.index].length);

	/*buf入列*/
	if (ioctl(fd, VIDIOC_QBUF, &buf) < 0)
		errno_exit("VIDIOC_QBUF");

	return JC_SUCCESS;
}

int jc_v4l2_show_camera_image(int fd)
{
	struct v4l2_buffer buf;
	unsigned char *buffer_565;
	rgb24_type rgbinfo;
	int ret = -1;
	fd_set fds;
	FD_ZERO (&fds);
	FD_SET (fd, &fds);

	init_data(&rgbinfo);
	rgbinfo.scale_num = 1;
	rgbinfo.scale_denom = 2;

	ret = select(fd + 1, &fds, NULL, NULL, NULL);
	if (ret < 0)
		errno_exit("select error");
	/*帧出列*/
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if (ioctl (fd, VIDIOC_DQBUF, &buf) < 0)
		errno_exit("read image data error");
	//将数据显示到屏幕
	if (jpeg_to_rgb24_from_mem(tempbuffers[buf.index].start,tempbuffers[buf.index].length,&rgbinfo) == -1)
		printf("jpeg_to_rgb24_from_mem error\n");
	buffer_565 = RGB888toRGB565(rgbinfo.buffer_24,rgbinfo.width,rgbinfo.height) ;//解码好的数据的地址 
	show_jpeg(buffer_565, rgbinfo.width, rgbinfo.height, 10, 0);
	free(buffer_565); 
	if (rgbinfo.buffer_24)
		free(rgbinfo.buffer_24);
	/*buf入列*/
	if (ioctl(fd, VIDIOC_QBUF, &buf) < 0)
		errno_exit("VIDIOC_QBUF");

	return JC_SUCCESS;
}

void jc_v4l2_close(int fd)
{
	close(fd);
}

//将yuyv转换到rgb格式
/*
rgb 需要先开辟大小为WIDTH * HEIGTH *  3的空间，
因为我们用的RGB是24位格式，
3个字节分别代表一个像素点的R、G、B，根据公式转换就好了
*/
void yuyv_to_rgb(unsigned char* yuv,unsigned char* rgb)  
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
  
    for(i = 0; i <= (WIDTH * HEIGHT) / 2 ;i++)  
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


//rgb缩放
void rgb_stretch(char* src_buf, char* dest_buf, int des_width, int des_hight)  
{  
/*
    //最临近插值算法  
    //双线性内插值算法放大后马赛克很严重 而且帧率下降严重  
    printf("des_width = %d, des_hight = %d \n ",des_width, des_hight);  
    double rate_w = (double) WIDTH / des_width;//横向放大比  
    double rate_h = (double) HEIGHT / des_hight;//轴向放大比  
      
    int dest_line_size = ((des_width * BITCOUNT +31) / 32) * 4;   
    int src_line_size = BITCOUNT * WIDTH / 8;  
    int i = 0, j = 0, k = 0;  
    for (i = 0; i < des_hight; i++)//desH 目标高度  
    {  
        //选取最邻近的点  
        int t_src_h = (int)(rate_h * i + 0.5);//rateH (double)srcH / desH;  
        for (j = 0; j < des_width; j++)//desW 目标宽度  
        {  
            int t_src_w = (int)(rate_w * j + 0.5);                              
            memcpy(&dest_buf[i * dest_line_size] + j * BITCOUNT / 8, \  
                &src_buf[t_src_h * src_line_size] + t_src_w * BITCOUNT / 8,\  
                BITCOUNT / 8);              
        }  
    }     */
}  

/*
void rgb_to_bmp(unsigned char* pdata, FILE* bmp_fd)       
{  
    //分别为rgb数据，要保存的bmp文件名   
    int size = WIDTH * HEIGHT * 33 * sizeof(char); // 每个像素点3个字节    
    // 位图第一部分，文件信息    
    BMPFILEHEADER_T bfh;    
    bfh.bfType = (unsigned short)0x4d42;  //bm    
    bfh.bfSize = size  // data size    
        + sizeof( BMPFILEHEADER_T ) // first section size    
        + sizeof( BMPINFOHEADER_T ) // second section size    
        ;    
    bfh.bfReserved1 = 0; // reserved    
    bfh.bfReserved2 = 0; // reserved    
    bfh.bfOffBits = sizeof( BMPFILEHEADER_T )+ sizeof( BMPINFOHEADER_T );//真正的数据的位置   
//    printf("bmp_head== %ld\n", bfh.bfOffBits);   
    // 位图第二部分，数据信息    
    BMPINFOHEADER_T bih;    
    bih.biSize = sizeof(BMPINFOHEADER_T);    
    bih.biWidth = WIDTH;    
    bih.biHeight = -HEIGHT;//BMP图片从最后一个点开始扫描，显示时图片是倒着的，所以用-height，这样图片就正了    
    bih.biPlanes = 1;//为1，不用改    
    bih.biBitCount = 24;    
    bih.biCompression = 0;//不压缩    
    bih.biSizeImage = size;    
  
    bih.biXPelsPerMeter = 0;//像素每米    
    
    bih.biYPelsPerMeter = 0;    
    bih.biClrUsed = 0;//已用过的颜色，为0,与bitcount相同    
    bih.biClrImportant = 0;//每个像素都重要     
      
    fwrite( &bfh, 8, 1, bmp_fd);   
    fwrite(&bfh.bfReserved2, sizeof(bfh.bfReserved2), 1, bmp_fd);    
    fwrite(&bfh.bfOffBits, sizeof(bfh.bfOffBits), 1, bmp_fd);    
    fwrite(&bih, sizeof(BMPINFOHEADER_T), 1, bmp_fd);    
    fwrite(pdata, size, 1, bmp_fd);     
}*/




