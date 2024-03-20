/****************************************************************************************
 *   FileName    : MountMonitor.c
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

#include <sys/vfs.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <asm/types.h>
#include <unistd.h>
#include <glib.h>
#include "MountMonitor.h"
#include "HotPlugParser.h"

extern int cd_debug;
#define DEBUG_MOUNT_PROCESS_PRINTF(format, arg...) \
	if (cd_debug) \
	{ \
		fprintf(stderr, "[MOUNT PROCESS] %s: "format"", __FUNCTION__, ##arg); \
	}

//////////////////////////// Thread /////////////////////////////

int g_mount_run;
pthread_t mount_thread;

//////////////////////////Mount Info ////////////////////////////

int g_mount_count = 0;
static tMOUNT_INFO Mount_Info;	// proc/mounts file contents

///////////////////////////////////////////////////////////////

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
#define error(...) do {\
	fprintf(stderr, "[%s:%d] ", __FUNCTION__, __LINE__); \
	fprintf(stderr, __VA_ARGS__); \
	putc('\n', stderr); \
} while (0)
#else
#define error(args...) do {\
	fprintf(stderr, "[%s:%d] ", __FUNCTION__, __LINE__); \
	fprintf(stderr, ##args); \
	putc('\n', stderr); \
} while (0)
#endif

static tMOUNT_INFO *pGetMountInfo(void);
static void clear_mountInfo(void);
static void clear_all_mountlist(void);
static int SearchMountedDevice(char *mountStr, char *nodeName);
static gboolean mount_changed_event(void);
#ifdef MOUNT_POLLING_METHOD
static void *mount_thread_function(void *arg);
#endif

static tMOUNT_INFO *pGetMountInfo(void)
{
	return (&Mount_Info);
}

static void clear_mountInfo(void)
{
	tMOUNT_INFO *pMountInfo;

	pMountInfo = pGetMountInfo();

	memset(pMountInfo->mDeviceName, 0, DEVICE_NAME_LENGTH);
	memset(pMountInfo->mMountPath, 0, MOUNT_PATH_LENGTH);
	memset(pMountInfo->mFSType, 0, FS_TYPE_LENGTH);
}

static int SearchMountedDevice(char *mountStr, char *nodeName)
{
	char 	*token;
	char 	*devNode;
	char 	*path;
	char 	*ptr[1];
	int		ret = 0;

	// /dev/sda1 on /run/media/sda1 type vfat
	if ((strstr(mountStr, "/dev")) && (strstr(mountStr, nodeName)))
	{
		// /dev/sda1
		token = strtok_r(mountStr, "/", &ptr[0]);
		devNode = strtok_r(NULL, " ", &ptr[0]);

		path = strtok_r(NULL, " \t", &ptr[0]);
		DEBUG_MOUNT_PROCESS_PRINTF("MOUNT : DeviceNode = (%s), MountPath = (%s)\n", devNode, path);
		//fprintf(stderr, "devnode (%s), mountpath (%s)\n", devNode, path);
		DeviceMounted(devNode, path);
		ret = 1;
	}
	else
	{
		ret = 0;
	}
	return ret;
}

extern int g_hotplug_run;
#ifdef MOUNT_POLLING_METHOD
static void *mount_thread_function(void *arg)
{
	char 			buf[PROC_MOUNT_READ_BUF_SIZE];
	FILE 			*pFile;
	tMOUNT_INFO 	*pMountInfo;
	int	 			err = 0;
	int				start = 1;
	unsigned int		cnt = 0;
	tBLOCK_DEVICENODE *pBlockDeviceList;
	
	pFile = fopen("/proc/mounts", "r");
	if (pFile != 0)
	{
		while (g_mount_run) 
		{
			for (cnt=0; cnt < MAX_DEVICE_NUM; cnt++)
			{
				start = 1;
				pBlockDeviceList = pGetBlockDeviceList(cnt);
				if ((pBlockDeviceList->mInserted != 0) && (pBlockDeviceList->mMounted == 0) && (pBlockDeviceList->mChanged == 0))
				{
					while (start)
					{
						memset(buf, 0, PROC_MOUNT_READ_BUF_SIZE);
						if (fgets(buf, PROC_MOUNT_READ_BUF_SIZE, pFile) != NULL)
						{
							err = SearchMountedDevice(buf, pBlockDeviceList->mNodeName);
							if (err)
							{
								pBlockDeviceList->mMounted = 1;
								start = 0;
							}
						}
						else
						{
							err = fseek(pFile, 0, SEEK_SET);
							if (err < 0)
							{
								fprintf(stderr, "%s : fseek error !!\n", __FUNCTION__);
							}
							start = 0;
						}
					}
				}

				if (cnt == MAX_DEVICE_NUM)
				{
					cnt = 0;
					fprintf(stderr, "cnt = 0\n");
				}
				usleep(10000);
			}
		}
		fclose(pFile);
	}
	else
	{
		fprintf(stderr, "%s: /proc/mounts file open fail.\n", __FUNCTION__);
	}

	pthread_exit("mount thread exit\n");
	(void)arg;
	return NULL;
}

int mount_thread_initialize(void)
{
	int err = 0;
	int ret = 0;

	DEBUG_MOUNT_PROCESS_PRINTF("\n");
	g_mount_run = 1;
	err = pthread_create(&mount_thread, NULL, mount_thread_function, NULL); 
	if (err == 0)
	{
		ret = 1;
	}
	else
	{
		perror("create mount thread failed: ");
		g_mount_run = 0;
		ret = 0;
	}

	return ret;
}

void mount_thread_release(void)
{
	int err;
	void *res;

	DEBUG_MOUNT_PROCESS_PRINTF("\n");
	if (g_mount_run)
	{
		g_mount_run = 0;
		err = pthread_join(mount_thread, &res);
		if (err != 0)
		{
			perror("mount thread join faild: ");
		}
	}
}
#else
static gboolean mount_changed_event(void)
{
	FILE *pFile;
	int ret;
	char buf[PROC_MOUNT_READ_BUF_SIZE];
	tMOUNT_INFO *pMountInfo;
	char *pbuf;
	int IsValid = 0;
	int IsStop = 0;
	char *err;

	//DEBUG_MOUNT_PROCESS_PRINTF("\n");
	clear_mountInfo();	// Mount structure Initailize

	pFile = popen("cat /proc/mounts", "r");
	if (pFile != 0)
	{
		while (IsStop == 0) 
		{
			memset(buf, 0, PROC_MOUNT_READ_BUF_SIZE);
			err = fgets(buf, PROC_MOUNT_READ_BUF_SIZE, pFile);

			if (err != NULL)
			{
				pMountInfo = pParseProcMountFile(buf);	// parse about /proc/mounts
				if (pMountInfo != NULL)
				{
					SearchMountedDevice(pMountInfo);	// Check Mount point
				}
				else
				{
					fprintf(stderr, "%s, mount info isn't exist !!\n", __FUNCTION__);
				}
			}
			else
			{
				IsStop = 1;
			}
		}
		pclose(pFile);
	}
	else
	{
		fprintf(stderr, "%s: /proc/mounts file open fail.\n", __FUNCTION__);
	}
	
	return TRUE;
}

void mount_monitor(void)
{
	GError *error = NULL;
	GIOChannel *mdstat_channel;
	GSource *mounts_source;

	mdstat_channel = g_io_channel_new_file ("/proc/mounts", "r", &error);

	if (mdstat_channel != NULL)
	{
		mounts_source = g_io_create_watch (mdstat_channel, G_IO_ERR);
		g_source_set_callback (mounts_source, (GSourceFunc) mount_changed_event, NULL, NULL);
		g_source_attach (mounts_source, g_main_context_get_thread_default ());
		g_source_unref (mounts_source);		
	}
	else
	{
		g_error_free (error);
	}
}
#endif

