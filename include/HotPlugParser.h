/****************************************************************************************
 *   FileName    : HotPlugParser.h
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

#ifndef _HOTPLUG_PARSER_H_
#define _HOTPLUG_PARSER_H_

#define 	MAX_UDEV_MSG_SIZE 			1024
#define	MAX_DEVICE_NUM				10
#define	HP_DEVICE_TYPE				10
#define	HP_PARTITION_NUMBER			10
#define	HP_SLOT_NUMBER				20
#define	HP_NODE_LENGTH				20		// /dev/mmcblkp1

#define	SS_ACTION						7		// string size "ACTION="
#define	SS_SUBSYSTEM					10		// string size "SUBSYSTEM="
#define	SS_DEVNAME					8		// string size "DEVNAME="
#define	SS_DEVTYPE						8		// string size "DEVTYPE="
#define	SS_NPARTS						7		// string size "NPARTS="
#define	SS_DISK_MEDIA_CHANGE			19		// string size "DISK_MEDIA_CHANGE=1"

#define	SS_ADD							3		// string size "ADD"
#define	SS_REMOVE						6		// string size "REMOVE"
#define	SS_CHANGE						6		// string size "STRING"
#define	SS_PARTITION					9		// string size "PARTITION"
#define	SS_BLOCK						5		// string size "BLOCK"

enum udev_monitor_netlink_group 
{
        UDEV_MONITOR_NONE,
        UDEV_MONITOR_KERNEL,
        UDEV_MONITOR_UDEV,
};

typedef struct
{
	char *mNodeName;
	int mInserted;
	int mMounted;
	int mChanged;
	
} tBLOCK_DEVICENODE;

typedef struct
{
	char *action;
	char *subsystem;		// Sub System (usb, scsi, scsi_host, usb_device, scsi, scsi_disk, scsi_device, scsi_generic, bdi, block)
	char *devname;		// Device Name (bus/usb/001/029, usbdev1.29, sg0, sda, sda1)
	char *devtype;		// Device Type (usb_device, scsi_host, scsi_target, scsi_device, disk, partition)
	char *nparts;			// Partition Number
	int	change_flag;	// Slot status was changed

} tUDEV_DEVICE;

/////////////////////// EXTERN ///////////////////////

extern void ClearBlockDeviceListAll(void);
extern int hotplug_thread_initialize(void);
extern void hotplug_thread_release(void);
extern tBLOCK_DEVICENODE *pGetBlockDeviceList(int index);
#endif

