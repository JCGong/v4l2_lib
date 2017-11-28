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

struct mjpeg_size_t
{
	int width;
	int height;
};

struct buffer *buffers;
struct mjpeg_size_t mjpeg_size = {640,480};

int file_fd = 0;
int fd = 0;
int buf_count = 4;
#define mjpeg_name  "./test.jpg"
 
int main (int argc,char **argv)
{	
	struct v4l2_rect rect;
	struct mjpeg_size_t size;
	if (argc < 2)
	{
		printf("usge:para0:app name para1:dev name\n");
		return -1;
	}
	printf("dev name:%s\n", argv[1]);
	fd = jc_v4l2_open(argv[1]); 
	if (fd < 0)
	{
		printf("open video dev:%s error",argv[1]);
	}

	file_fd = open(mjpeg_name, O_RDWR | O_CREAT, 0777);
	//查询设置支持的操作
	jc_v4l2_query_capability_info(fd);
	//查询设备支持的帧格式
	//jc_v4l2_query_support_image_format(fd);
	//enum_frame_formats(fd);
	jc_v4l2_pixelformat_is_support(fd,V4L2_PIX_FMT_MJPEG);
	jc_v4l2_pixelformat_is_support(fd,V4L2_PIX_FMT_YUYV);
	jc_v4l2_pixelformat_is_support(fd,V4L2_PIX_FMT_JPEG);
	jc_v4l2_pixelformat_is_support(fd,V4L2_PIX_FMT_YUYV);
	jc_v4l2_pixelformat_is_support(fd,V4L2_PIX_FMT_YVU420);
	//查询设备数据流信息
	jc_v4l2_query_stream_info(fd);
	
	//查询设备图像裁剪信息
	//jc_v4l2_query_corpcap_info(fd);
	//jc_v4l2_set_crop_default(fd);
	//jc_v4l2_get_crop_para(fd,&rect);
	//rect.height = 500;
	//rect.width = 200;
	//jc_v4l2_get_crop_para(fd, &rect);

	
	//设置捕获mjpeg图像
	jc_v4l2_set_mjpeg_pixelformat(fd, mjpeg_size.width, mjpeg_size.height);
	jc_v4l2_get_mjpeg_pixelformat(fd, &size.width, &size.height);
	printf("mjpeg width:%d, height:%d\n", size.width, size.height);

	//请求图像buf
	buf_count = jc_v4l2_reqbuf(fd, buf_count);
	printf("buf_count:%d\n", buf_count);
	//内存映射
	jc_v4l2_init_mmap(fd, buf_count, buffers);
	//开始数据抓取
	jc_v4l2_start_capturing(fd, buf_count);
	//数据处理(抓取一张jpg的图片)
	jc_v4l2_capture_mjpeg(fd, buffers, file_fd);

	//停止数据抓取
	jc_v4l2_stop_capturing(fd);
	
	//删除内存映射
	//jc_v4l2_uninit_mmap(buf_count, buffers);
	//关闭video 设备
	jc_v4l2_close(fd);
	close(file_fd);
 
	return 1;
}