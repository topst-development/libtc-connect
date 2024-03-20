/****************************************************************************************
 *   FileName    : TCConnectInterface.h
 *   Description : 
 ****************************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This library contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited 
to re-distribution in source or binary form is strictly prohibited.
This source code is provided ¡°AS IS¡± and nothing contained in this source code 
shall constitute any express or implied warranty of any kind, including without limitation, 
any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, 
copyright or other third party intellectual property right. 
No warranty is made, express or implied, regarding the information¡¯s accuracy, 
completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, 
out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement 
between Telechips and Company.
*
****************************************************************************************/

#ifndef _CONNECT_INTERFACE_H_
#define _CONNECT_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef void (*DeviceConnected_cb)(char *node_name);
typedef void (*DeviceDisconnected_cb)(char *node_name);
typedef void (*DeviceMounted_cb)(char *node_name, char * mount_path);
typedef void (*DeviceMountErrorOccurred_cb)(char *node_name);

/* Extern API Functions */
extern int TC_ConnectInitialize(void);
extern void TC_ConnectRelease(void);
extern void TC_ConnectDebugOn(int enable);

/* Extern Callback Functions */
extern void TC_DeviceConnected_notify_cb(DeviceConnected_cb cb_func);
extern void TC_DeviceDisconnected_notify_cb(DeviceDisconnected_cb cb_func);
extern void TC_DeviceMounted_notify_cb(DeviceMounted_cb cb_func);
extern void TC_DeviceMountErrorOccurred_notify_cb(DeviceMountErrorOccurred_cb cb_func);

#ifdef __cplusplus
}
#endif

#endif
	
