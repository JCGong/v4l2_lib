#ifndef _V4L2_OPERATION_LIB_H_
#define _V4L2_OPERATION_LIB_H_

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

struct buffer {
        void *                  start;
        size_t                  length;
};

typedef unsigned int  u32;

#define JC_ERROR   -1
#define JC_SUCCESS   1
#define JC_YES 1
#define JC_NO 0


#define CLEAR(x) memset(&(x), 0, sizeof(x))

int jc_v4l2_open(char *path);
int jc_v4l2_query_capability_info(int fd);
void jc_v4l2_query_support_image_format(int fd);
int jc_v4l2_pixelformat_is_support(int fd, u32 pixelformat);
int jc_v4l2_get_current_pixelformat(int fd, struct v4l2_format *format);
int jc_v4l2_set_mjpeg_pixelformat(int fd, u32 width, u32 height);
int jc_v4l2_get_mjpeg_pixelformat(int fd, u32 *width, u32 *height);
int jc_v4l2_query_stream_info(int fd);
int jc_v4l2_get_stream_framenum(int fd, u32 *frame_num);
int jc_v4l2_set_stream_framenum(int fd, u32 frame_num);
void jc_v4l2_query_camera_gain_range(int fd);
int jc_v4l2_get_camera_gain(int fd, u32 *gain);
int jc_v4l2_set_camera_gain(int fd, u32 gain);
void jc_v4l2_query_corpcap_info(int fd);
int jc_v4l2_get_crop_para(int fd, struct v4l2_rect *rect);
int jc_v4l2_set_crop_para(int fd, struct v4l2_rect * rect);
int jc_v4l2_reqbuf(int fd, int buf_count);
int jc_v4l2_init_mmap(int fd, int buf_count, struct buffer *buffers);
int jc_v4l2_start_capturing(int fd, int buf_count);
int jc_v4l2_stop_capturing(int fd);
int jc_v4l2_uninit_mmap(int buf_count, struct buffer *buffers);
int jc_v4l2_capture_mjpeg(int fd, struct buffer *buffers, int file_fd);
void jc_v4l2_close(int fd);
int jc_v4l2_set_image_4point_default_crop(int fd);
int jc_v4l2_set_crop_2point_default_crop(int fd);
int jc_v4l2_set_crop_default(int fd);
int jc_v4l2_get_input_index(int fd, u32 *index);
int jc_v4l2_set_input_index(int fd, u32 index);
int jc_v4l2_get_output_index(int fd, u32 *index);
int jc_v4l2_set_output_index(int fd, u32 index);
void jc_v4l2_query_current_input_standard(int fd, u32 index);
int enum_frame_formats(int fd);



#if 0
v4l2中常用的命令
VIDIOC_QUERYCAP     /* 获取设备支持的操作 */  
VIDIOC_G_FMT        /* 获取设置支持的视频格式 */  
VIDIOC_S_FMT        /* 设置捕获视频的格式 */  
VIDIOC_REQBUFS      /* 向驱动提出申请内存的请求 */  
VIDIOC_QUERYBUF     /* 向驱动查询申请到的内存 */  
VIDIOC_QBUF         /* 将空闲的内存加入可捕获视频的队列 */  
VIDIOC_DQBUF        /* 将已经捕获好视频的内存拉出已捕获视频的队列 */  
VIDIOC_STREAMON     /* 打开视频流 */  
VIDIOC_STREAMOFF    /* 关闭视频流 */  
VIDIOC_QUERYCTRL    /* 查询驱动是否支持该命令 */  
VIDIOC_G_CTRL       /* 获取当前命令值 */  
VIDIOC_S_CTRL       /* 设置新的命令值 */  
VIDIOC_G_TUNER      /* 获取调谐器信息 */  
VIDIOC_S_TUNER      /* 设置调谐器信息 */  
VIDIOC_G_FREQUENCY  /* 获取调谐器频率 */  
VIDIOC_S_FREQUENCY  /* 设置调谐器频率 */  
#endif

#endif
