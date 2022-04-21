/*
 *  Get video frame YUYV/MGPEG (usb uvc)
 *
 *  Created on: 22/09/2017
 *      Author: pvvx
 */
#ifndef _USB_UVC_DRV_H_
#define _USB_UVC_DRV_H_

#ifdef __cplusplus

#define VFRMT_H264   0
#define VFRMT_MJPEG  1
#define VFRMT_YUYV   2

extern "C" { 

#else

#include "uvc_intf.h"
#include "uapi_videodev2.h"

typedef struct {
	volatile char off; // !=0 - завершить  get_video_frame_thrd
	volatile char run; // !=0 - get_video_frame_thrd запущен
	volatile char get; // =1 - запрос копирования фрейма который влезет в буфер, = 2 - копирования фрейма только с размером pbuf_size
	volatile char ok;  //
	char *pbuf;
	unsigned int pbuf_size;
	unsigned int frame_size;
	unsigned int copy_count;
	unsigned int frame_count;
	struct uvc_context uvc_ctx;	
} vfrm_buf_t;

extern vfrm_buf_t vfrmb; // управляющая структура драйвера get video frame

#endif



//extern void stop_uvc(void);
//extern int start_uvc(void);
void FreeBufCameraFrame(void);
unsigned int SetCameraParam(int fmt_type, int width, int height, int frame_rate, int compression_ratio);
int StartCamera(void);
unsigned int GetCameraFrame(void *pbuf);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _USB_UVC_DRV_H_
