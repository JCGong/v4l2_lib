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
v4l2�г��õ�����
VIDIOC_QUERYCAP     /* ��ȡ�豸֧�ֵĲ��� */  
VIDIOC_G_FMT        /* ��ȡ����֧�ֵ���Ƶ��ʽ */  
VIDIOC_S_FMT        /* ���ò�����Ƶ�ĸ�ʽ */  
VIDIOC_REQBUFS      /* ��������������ڴ������ */  
VIDIOC_QUERYBUF     /* ��������ѯ���뵽���ڴ� */  
VIDIOC_QBUF         /* �����е��ڴ����ɲ�����Ƶ�Ķ��� */  
VIDIOC_DQBUF        /* ���Ѿ��������Ƶ���ڴ������Ѳ�����Ƶ�Ķ��� */  
VIDIOC_STREAMON     /* ����Ƶ�� */  
VIDIOC_STREAMOFF    /* �ر���Ƶ�� */  
VIDIOC_QUERYCTRL    /* ��ѯ�����Ƿ�֧�ָ����� */  
VIDIOC_G_CTRL       /* ��ȡ��ǰ����ֵ */  
VIDIOC_S_CTRL       /* �����µ�����ֵ */  
VIDIOC_G_TUNER      /* ��ȡ��г����Ϣ */  
VIDIOC_S_TUNER      /* ���õ�г����Ϣ */  
VIDIOC_G_FREQUENCY  /* ��ȡ��г��Ƶ�� */  
VIDIOC_S_FREQUENCY  /* ���õ�г��Ƶ�� */  
#endif

#endif
