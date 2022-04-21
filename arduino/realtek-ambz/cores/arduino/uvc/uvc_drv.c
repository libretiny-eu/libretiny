/*
 * uvc_drv.c
 *
 *  Created on: 25/09/2017.
 *      Author: pvvx
 */


#include <platform_opts.h>
#include "rtl8195a.h"
#include "FreeRTOS.h"
#include "task.h"
#include "rtl8195a/rtl_libc.h"
#include "section_config.h"

#include <platform/platform_stdlib.h>
#include <lwip/sockets.h>
#include "sockets.h"
#include "lwip/netif.h"

#include "cmsis_os.h"

#include "freertos_pmu.h"

#include "os_support.h"
#include "timer_api.h"

#include "videodev2.h"
#include "uvcvideo.h"
#include "v4l2_driver.h"
#include "mjpeg/mjpeg.h"
#include "rtsp/rtsp_api.h"
#include "dwc_otg_driver.h"
#include "v4l2_intf.h"

#include "uvc_drv.h"

//----------------------------------------------
#undef info_printf
#define info_printf(fmt, ...) rtl_printf(fmt, ##__VA_ARGS__)
#undef err_printf
#define err_printf(fmt, ...) rtl_printf(fmt, ##__VA_ARGS__)
//----------------------------------------------
extern struct usb_device * usb_get_dev_index(int index);
extern void usb_hub_reset(void);
extern void usb_stop(void);
//----------------------------------------------
char uvc_is_start; //
vfrm_buf_t vfrmb; // управляющая структура
//---start_uvc()-------------------------------------------
int start_uvc(void) {
	int result = -1;
	if (uvc_is_start == 0) {
		do {
			info_printf("USB init...\n");
			_usb_init();
			result = wait_usb_ready();
			if (result < 0) {
				err_printf("\r\nFail to init usb driver!\n");
				break;
			}
			struct usb_device * usb_dev = usb_get_dev_index(1);
			if (usb_dev)
				printf("USB ID %04X:%04X\n", usb_dev->descriptor.idProduct,
						usb_dev->descriptor.idVendor);
			info_printf("UVC stream init...\n");
			result = uvc_stream_init();
			if (result < 0) {
				err_printf("Fail!\n");
				break;
			}
			uvc_is_start = 1;
		} while (0);
	} else  if (uvc_is_start == 2) {
		do {
			info_printf("USB hub reset...\n");
			usb_hub_reset(); // ?
			result = wait_usb_ready();
			if (result < 0) {
				err_printf("\r\nFail to init usb driver!\n");
				break;
			}
			result = 2;
		} while(0);
	} else 	{
		result = 1;
	}
	return result;
}
//---stop_uvc()-------------------------------------------
void stop_uvc(void) {
	//TODO:  как выгрузить всё полностью? _usb_deinit() = { return; } only!
	info_printf("USB Stop.\n");
	usb_stop();
	uvc_is_start = 2;
}
//---get_video_frame_thrd()-------------------------------------------
void get_video_frame_thrd(void *parm)
{
	int ret;
	vfrm_buf_t * pvfrm = (vfrm_buf_t *) parm;
	struct uvc_buf_context buf;

	pvfrm->run = 1;
	if (uvc_stream_on() >= 0) {
		while (uvc_is_stream_on() && !pvfrm->off) {
			memset(&buf, 0, 0x10u);
			vTaskDelay(1);
			ret = uvc_dqbuf(&buf);
//			DiagPrintf("dqbuf(%d)\r\n", ret);
			if (ret < 0) {
				if (ret == -61)
					break;
				uvc_stream_free();
			}
//			DiagPrintf("buf.index(%d)\r\n", buf.index);
			if (buf.index == -1)
				continue;
			pvfrm->frame_count++;
#if CONFIG_DEBUG_LOG > 2
			if(pvfrm->pbuf_size != buf.len) HalSerialPutcRtl8195a('@');
			else HalSerialPutcRtl8195a('#');
#endif
			if (pvfrm->get && pvfrm->pbuf) {
				if(pvfrm->get == 1 || pvfrm->pbuf_size == buf.len) {
					memcpy(pvfrm->pbuf, buf.data, buf.len);
					pvfrm->frame_size = buf.len;
					pvfrm->copy_count++;
					pvfrm->get = 0;
					pvfrm->ok = 1;
//					DiagPrintf("\r\nFrameBuf[%d] at %p[%d]\r\n", buf.index, buf.data, buf.len);
				}
			}
			ret = uvc_qbuf(&buf);
//			DiagPrintf("qbuf(%d)\r\n", ret);
			if (ret >= 0)
				continue;
			uvc_stream_free();
		};
	};
	pvfrm->run = 0;
#if CONFIG_DEBUG_LOG > 2
	DiagPrintf("\r\nTask 'get-vfrm' close.\r\n");
#endif
	vTaskDelete(NULL);
}

//---FreeBufCameraFrame()-------------------------------------------
void FreeBufCameraFrame(void) {
	vfrmb.get = 0;
	if(vfrmb.pbuf != NULL) {
		free(vfrmb.pbuf);
		vfrmb.pbuf = NULL;
	}
}
//---StartCamera()-------------------------------------------
unsigned int SetCameraParam(int fmt_type, int width, int height, int frame_rate, int compression_ratio) {
	FreeBufCameraFrame();
	switch(fmt_type) {
		case 0:
			vfrmb.uvc_ctx.fmt_type = V4L2_PIX_FMT_H264;
			break;
		case 1:
			vfrmb.uvc_ctx.fmt_type = V4L2_PIX_FMT_MJPEG;
			break;
		default:
			vfrmb.uvc_ctx.fmt_type = V4L2_PIX_FMT_YUYV;
			break;
	}
	vfrmb.uvc_ctx.width = width;
	vfrmb.uvc_ctx.height = height;
	vfrmb.uvc_ctx.frame_rate = frame_rate;
	vfrmb.uvc_ctx.compression_ratio = compression_ratio;
	vfrmb.pbuf_size = vfrmb.uvc_ctx.width * vfrmb.uvc_ctx.height;
	if(vfrmb.uvc_ctx.fmt_type == V4L2_PIX_FMT_YUYV) vfrmb.pbuf_size <<= 1;
	return vfrmb.pbuf_size;
}
//---StartCamera()-------------------------------------------
int StartCamera(void) {
	if (vfrmb.run && vfrmb.pbuf_size) return 1;
	vfrmb.off = 0;
	vfrmb.run = 0;
	vfrmb.get = 0;
	vfrmb.ok = 0;
	vfrmb.copy_count = 0;
	vfrmb.frame_count = 0;
	if (start_uvc() >= 0) {
		info_printf("\r\nStart camera...\n");
		if (v4l_set_param(vfrmb.uvc_ctx.fmt_type, &vfrmb.uvc_ctx.width, &vfrmb.uvc_ctx.height, &vfrmb.uvc_ctx.frame_rate, &vfrmb.uvc_ctx.compression_ratio) < 0) {
			err_printf("Not set param!\r\n");
			return 0;
		}
		if(xTaskCreate((TaskFunction_t) get_video_frame_thrd, "get-vfrm", 1024, &vfrmb, tskIDLE_PRIORITY + 2, NULL) != pdPASS) { // +4 //  + PRIORITIE_OFFSET
			err_printf("Not start Task 'get-vfrm'!\r\n");
			return 0;
		}
		vTaskDelay(2);
	} else return 0;
	return 1;
}
//---GetCameraFrame()-------------------------------------------
unsigned int GetCameraFrame(void *pbuf) {
	if (vfrmb.run != 0) {
		if(pbuf == NULL) {
			if(vfrmb.pbuf == NULL) {
				vfrmb.pbuf = malloc(vfrmb.pbuf_size);
				if(vfrmb.pbuf == NULL) {
#if CONFIG_DEBUG_LOG > 2
					err_printf("Not alloc frame buf!\r\n");
#endif
					return 0;
				}
			}
		} else vfrmb.pbuf = (char *)pbuf;
		TickType_t tt = xTaskGetTickCount();
		vfrmb.get = 0;	vfrmb.ok = 0;
		if(vfrmb.uvc_ctx.fmt_type == V4L2_PIX_FMT_YUYV) vfrmb.get = 2;
		else vfrmb.get = 1;
		vTaskDelay(10);
		while(vfrmb.get) {
#if CONFIG_DEBUG_LOG > 2
			HalSerialPutcRtl8195a('.') ;
#endif
			if(xTaskGetTickCount() - tt > 5000) {
				err_printf("USB timeout!\r\n");
				vfrmb.off = 1;
				return 0;
			}
			vTaskDelay(10);
		}
		if(!vfrmb.get && vfrmb.ok && vfrmb.frame_size) {
			return vfrmb.frame_size;
		}
	}
#if CONFIG_DEBUG_LOG > 2
	else err_printf("Camera not run!\r\n");
#endif
	return 0;
}


