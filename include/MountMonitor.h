/****************************************************************************************
 *   FileName    : MountMonitor.h
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

#ifndef _MOUNT_MONITOR_H_
#define _MOUNT_MONITOR_H_

#ifndef YES
#define YES 1
#endif
#ifndef NO
#define NO 0
#endif
#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/////////////////////// DEFINE ////////////////////////
#define MOUNT_POLLING_METHOD

#define 	PROC_MOUNT_READ_BUF_SIZE	512
#define 	FS_TYPE_LENGTH				32
#define 	MOUNT_PATH_LENGTH			128
#define 	PROC_MOUNT_PATH 				"cat /proc/mounts"
#define	DEVICE_NAME_LENGTH			20		// "tcc_cdrom0"

extern int g_mount_count;

typedef struct 
{
	char mDeviceName[DEVICE_NAME_LENGTH];		// mounted device name
	char mMountPath[MOUNT_PATH_LENGTH];		// mount directory path
	char mFSType[FS_TYPE_LENGTH];				// file system type;

} tMOUNT_INFO;	// structure of /proc/mounts file

typedef struct MountMonitor MountMonitor;

/////////////////////// EXTERN ///////////////////////

#ifdef MOUNT_POLLING_METHOD
extern int mount_thread_initialize(void);
extern void mount_thread_release(void);
#else
extern void mount_monitor(void);
#endif

#endif /* #ifndef _MOUNT_LIST_H */

