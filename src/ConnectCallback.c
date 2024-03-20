/****************************************************************************************
 *   FileName    : ConnectCallback.c
 *   Description : 
 ****************************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
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

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ConnectCallback.h"
#include "TCConnectInterface.h"


static DeviceConnected_cb			gDeviceConnected_cb			= NULL;
static DeviceDisconnected_cb		gDeviceDisconnected_cb			= NULL;
static DeviceMounted_cb			gDeviceMounted_cb				= NULL;
static DeviceMountErrorOccurred_cb	gDeviceMountErrorOccurred_cb	= NULL;	


/* Set Callback Functions */
void TC_DeviceConnected_notify_cb(DeviceConnected_cb cb_func)
{
	gDeviceConnected_cb = cb_func;
}

void TC_DeviceDisconnected_notify_cb(DeviceDisconnected_cb cb_func)
{
	gDeviceDisconnected_cb = cb_func;
}

void TC_DeviceMounted_notify_cb(DeviceMounted_cb cb_func)
{
	gDeviceMounted_cb = cb_func;
}

void TC_DeviceMountErrorOccurred_notify_cb(DeviceMountErrorOccurred_cb cb_func)
{
	gDeviceMountErrorOccurred_cb = cb_func;
}


/* API Functions */
void DeviceConnected(char *nodeName)
{
	if (gDeviceConnected_cb != NULL)
	{
		gDeviceConnected_cb(nodeName);
	}
}

void DeviceDisconnected(char * nodeName)
{
	if (gDeviceDisconnected_cb != NULL)
	{
		gDeviceDisconnected_cb(nodeName);
	}
}

void DeviceMounted(char * nodeName, char * mountPath)
{
	if (gDeviceMounted_cb != NULL)
	{
		gDeviceMounted_cb(nodeName, mountPath);
	}
}

void DeviceMounErrorOccured(char * nodeName)
{
	if (gDeviceMountErrorOccurred_cb != NULL)
	{
		gDeviceMountErrorOccurred_cb(nodeName);
	}
}

