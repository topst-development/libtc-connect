/****************************************************************************************
 *   FileName    : HotPlugParser.c
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include "HotPlugParser.h"
#include "ConnectCallback.h"

extern int cd_debug;

#define DEBUG_HOTPLUG_THREAD_PRINTF(format, arg...) \
	if (cd_debug) \
	{ \
		fprintf(stderr, "[HOTPLUG] %s: "format"", __FUNCTION__, ##arg); \
	}

#define DEBUG_UDEV_MESSAGE_PRINTF(format, arg...) \
	if (cd_debug > 1) \
	{ \
		fprintf(stderr, "[HOTPLUG] %s: "format"", __FUNCTION__, ##arg); \
	}
//////////////////////////// Thread /////////////////////////////

int g_hotplug_run;
pthread_t hotplug_thread;

//////////////////////////// Hot Plug ////////////////////////////

tBLOCK_DEVICENODE BlockDeviceNode[MAX_DEVICE_NUM];
static tUDEV_DEVICE UdevDevice;

///////////////////////////////////////////////////////////////

static tUDEV_DEVICE *pGetUdevDevice(void);
static void ClearBlockDeviceList(int index);
static int EmptyBlockDeviceList(void);
static void SetBlockDeviceInserted(char * pNodeName);
static int SearchInsertedBlockDeviceIndex(char *pNodeName);
static int SearchMountedBlockDeviceIndex(char *pNodeName);
static void send_connect_message(char *str);
static void send_disconnect_message(char *str);
static void send_change_message(char *str);
static void DoActionField(tUDEV_DEVICE *udev);
static void ParseUdevMessage(char *str);
static void *hotplug_thread_function(void *arg);

static tUDEV_DEVICE *pGetUdevDevice(void)
{
	return (&UdevDevice);
}

tBLOCK_DEVICENODE *pGetBlockDeviceList(int index)
{
	return (&BlockDeviceNode[index]);
}

static void ClearBlockDeviceList(int index)
{
	tBLOCK_DEVICENODE *pBlockDeviceList;
	int 					cnt = 0;

	if (index >= 0)
	{
		pBlockDeviceList = pGetBlockDeviceList(index);
		pBlockDeviceList->mInserted = 0;
		pBlockDeviceList->mMounted = 0;
		pBlockDeviceList->mChanged = 0;
	}
	else
	{
		fprintf(stderr, "%s: Wrong Index number (%d)\n", __FUNCTION__, index);
	}
}

void ClearBlockDeviceListAll(void)
{
	int 	cnt = 0;

	for(cnt = 0; cnt < MAX_DEVICE_NUM; cnt++)
	{
		ClearBlockDeviceList(cnt);
	}
}

static int EmptyBlockDeviceList(void)
{
	int 					cnt = 0;
	int 					ret = -1;
	tBLOCK_DEVICENODE *pBlockDeviceList;

	for(cnt=0; cnt<MAX_DEVICE_NUM; cnt++)
	{
		pBlockDeviceList = pGetBlockDeviceList(cnt);
		if (pBlockDeviceList->mInserted == 0)
		{
			ret = cnt;
			break;
		}
	}

	if (ret < 0)
	{
		fprintf(stderr, "None empty Block Name List\n");
	}

	DEBUG_HOTPLUG_THREAD_PRINTF("empty index (%d)\n", ret);
	return ret;
}

static void SetBlockDeviceInserted(char * pNodeName)
{
	tBLOCK_DEVICENODE *pBlockDeviceList;
	int 					index = -1;
	int 					cnt = 0;

	index = EmptyBlockDeviceList();
	if (index >= 0)
	{
		pBlockDeviceList = pGetBlockDeviceList(index);
		free(pBlockDeviceList->mNodeName);
		pBlockDeviceList->mNodeName = strdup(pNodeName);
		pBlockDeviceList->mInserted = 1;
	}
	else
	{
		fprintf(stderr, "%s: Wrong Index number (%d)\n", __FUNCTION__, index);
	}
}

static int SearchInsertedBlockDeviceIndex(char *pNodeName)
{
	tBLOCK_DEVICENODE *pBlockDeviceList;
	int					cnt = 0;
	int 					ret = -1;

	for(cnt = 0; cnt < MAX_DEVICE_NUM; cnt++)
	{
		pBlockDeviceList = pGetBlockDeviceList(cnt);
		if (pBlockDeviceList->mInserted == 1)
		{
			if (strncmp(pBlockDeviceList->mNodeName, pNodeName, strlen(pNodeName)) == 0)
			{
				ret = cnt;	
				break;
			}
		}	
	}

	return ret;
}

static int SearchMountedBlockDeviceIndex(char *pNodeName)
{
	tBLOCK_DEVICENODE *pBlockDeviceList;
	int 					cnt = 0;
	int					ret = -1;

	for(cnt = 0; cnt < MAX_DEVICE_NUM; cnt++)
	{
		pBlockDeviceList = pGetBlockDeviceList(cnt);
		if (pBlockDeviceList->mMounted == 1)
		{
			if (strncmp(pBlockDeviceList->mNodeName, pNodeName, strlen(pNodeName)) == 0)
			{
				ret = cnt;	
				break;
			}
		}
	}

	return ret;
}

static void send_connect_message(char *str)
{
	int index;

	index = SearchInsertedBlockDeviceIndex(str);
	if (index < 0)	// if not exist same device
	{
		DEBUG_HOTPLUG_THREAD_PRINTF("CONNECT (%s)\n", str);
		DeviceConnected(str);		//Connect --> Demon signal
		SetBlockDeviceInserted(str);
	}
	else
	{
		DEBUG_HOTPLUG_THREAD_PRINTF("Connect Fail : Exist Device index (%d), name (%s)\n", index, str);
	}
}

static void send_disconnect_message(char *str)
{
	int 	index = -1;

	index = SearchInsertedBlockDeviceIndex(str);
	if (index >= 0)
	{
		DeviceDisconnected(str);	//DisConnect --> Daemon signal
		ClearBlockDeviceList(index);
		DEBUG_HOTPLUG_THREAD_PRINTF("DISCONNECT (%s), index (%d)\n", str, index);
	}
	else
	{
		//fprintf(stderr, "%s: wrong index (%d), node name\n", __FUNCTION__, index, str);
	}
}

static void send_change_message(char *str)
{
	int 					index = -1;
	tBLOCK_DEVICENODE	*pBlockDeviceList;

	index = SearchInsertedBlockDeviceIndex(str);
	if (index >= 0)
	{
		pBlockDeviceList = pGetBlockDeviceList(index);
		DeviceDisconnected(pBlockDeviceList->mNodeName);	//DisConnect --> Daemon signal
		ClearBlockDeviceList(index);
		DEBUG_HOTPLUG_THREAD_PRINTF("CHANGE (%s), index (%d)\n", str, index);
	}
	else
	{
		//fprintf(stderr, "%s: wrong index (%d), node name\n", __FUNCTION__, index, str);
	}
}	

static void DoActionField(tUDEV_DEVICE *udev)
{
	tUDEV_DEVICE	*pUdevDevice;

	pUdevDevice = udev;	
	
	if (strncmp(pUdevDevice->action, "add", SS_ADD) == 0)
	{
		send_connect_message(pUdevDevice->devname);
	}
	else if (strncmp(pUdevDevice->action, "remove", SS_REMOVE) == 0)
	{
		send_disconnect_message(pUdevDevice->devname);
	}
	else if (strncmp(pUdevDevice->action, "change", SS_CHANGE) == 0)
	{
		pUdevDevice ->change_flag = 0;
		//send_change_message(pUdevDevice->devname); //Don't support USB-Hub
	}
	else
	{
		fprintf(stderr, "unknown action (%s), devname (%s)\n", pUdevDevice->action, pUdevDevice->devname);
	}
}

static void ParseUdevMessage(char *str)
{
	tUDEV_DEVICE	*pUdevDevice;

	pUdevDevice = pGetUdevDevice();
	
	if (strncmp(str, "ACTION=", SS_ACTION) == 0) 
	{
		free(pUdevDevice->action);
		pUdevDevice->action = strdup(&str[SS_ACTION]);
	}
	else if (strncmp(str, "SUBSYSTEM=", SS_SUBSYSTEM) == 0) 
	{
		free(pUdevDevice->subsystem);
		pUdevDevice->subsystem= strdup(&str[SS_SUBSYSTEM]);
	}
	else if (strncmp(str, "DEVNAME=", SS_DEVNAME) == 0) 
	{
		free(pUdevDevice->devname);
		pUdevDevice->devname = strdup(&str[SS_DEVNAME]);
		if ((strncmp(pUdevDevice->action, "change", SS_CHANGE) == 0) && (pUdevDevice ->change_flag))
		{
			//DoActionField(pUdevDevice); //Don't support USB-Hub
		}
	}
	else if (strncmp(str, "DEVTYPE=", SS_DEVTYPE) == 0) 
	{
		free(pUdevDevice->devtype);
		pUdevDevice->devtype = strdup(&str[SS_DEVTYPE]);
		//if (strncmp(pUdevDevice->devtype, "partition", SS_PARTITION) == 0) //ID103A1-233, Support Non-partition UFD
		if (strncmp(pUdevDevice->devtype, "disk", SS_PARTITION) == 0)
		{
			DoActionField(pUdevDevice);
		}
	}
	else if (strncmp(str, "NPARTS=", SS_NPARTS) == 0)
	{
		int num;
		free(pUdevDevice->nparts);
		pUdevDevice->nparts = strdup(&str[SS_NPARTS]);
		num = atoi(pUdevDevice->nparts);
		if ((strncmp(pUdevDevice->subsystem, "block", SS_BLOCK) == 0) && (num == 0))
		{
			//DoActionField(pUdevDevice); //Don't notice UFD with partition Info, support only disk Info.
		}
	}
	else if (strncmp(str, "DISK_MEDIA_CHANGE=1", SS_DISK_MEDIA_CHANGE) == 0)
	{
		pUdevDevice->change_flag = 1;
	}
}

static void *hotplug_thread_function(void *arg)
{
	struct sockaddr_nl  nladdr;
	struct msghdr       smsg;
	struct iovec        iov;
    struct timeval      timeout;
    fd_set      readfds;
	int         sock;
	char        buf[MAX_UDEV_MSG_SIZE];
	int         len = 0;
	int         ret;

	sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
	if(sock == -1) 
	{
		perror("socket");
	}
	else
	{
		nladdr.nl_family = AF_NETLINK;
		nladdr.nl_groups = UDEV_MONITOR_KERNEL; // | UDEV_MONITOR_UDEV;
		//nladdr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

		if(bind(sock, (struct sockaddr*)&nladdr, sizeof(nladdr))) 
		{
			perror("bind");
			close(sock);
		}
		else
		{
			while (g_hotplug_run) 
			{
				memset(buf, 0, sizeof(buf));
				iov.iov_base = &buf;
				iov.iov_len = sizeof(buf);    

				memset(&smsg, 0, sizeof(struct msghdr));
				smsg.msg_name = &nladdr;                    /* optional address */
				smsg.msg_namelen = sizeof(nladdr);          /* size of address */
				smsg.msg_iov = &iov;                        /* scatter/gather array */
				smsg.msg_iovlen = 1;                        /* # elements in msg_iov */
				smsg.msg_control = NULL;                    /* ancillary data, see below */
				smsg.msg_controllen = 0;                    /* ancillary data buffer len */
				smsg.msg_flags = 0;                         /* flags on received message */

                FD_ZERO(&readfds);          // initialize fd_set datatype
                FD_SET(sock, &readfds);     // set the server socket inpected by means of using select()

                timeout.tv_sec = 0;
                timeout.tv_usec = 500000;   // 500msec

                ret = select(sock+1, &readfds, NULL, NULL, &timeout);
                if (ret < 0)
                {
                    fprintf(stderr, "%s: select error (%d)\n", __FUNCTION__, ret);
                }
                else
                {
                    if (FD_ISSET(sock, &readfds))
                    {
        				len = recvmsg(sock, &smsg, 0); 	// hotplug_interrupt_receive
        				if (len)
        				{	
        					char *key;
        					size_t keylen;
        					ssize_t bufpos = 0;

        					while (bufpos < len) 
        					{
        						key = &buf[bufpos];
        						keylen = strlen(key);
        						//fprintf(stderr, "%s\n", key);
        						DEBUG_UDEV_MESSAGE_PRINTF("%s\n", key);
        						bufpos += keylen + 1;
        						ParseUdevMessage(key);
        					}
        					//fprintf(stderr, "\n");
        					DEBUG_UDEV_MESSAGE_PRINTF("\n");
        					memset(buf, 0, MAX_UDEV_MSG_SIZE);
        				}
                    }
                }
  			}
		}
	}
	
	pthread_exit("hotplug thread exit\n");
    close(sock);
	(void)arg;
	return NULL;
}

int hotplug_thread_initialize(void)
{
	int 		err;
	int 		ret = 0;

	DEBUG_HOTPLUG_THREAD_PRINTF("\n");
	g_hotplug_run = 1;
	err = pthread_create(&hotplug_thread, NULL, hotplug_thread_function, NULL); 
	if (err == 0)
	{
		ret = 1;
	}
	else
	{
		perror("create hotplug thread failed: ");
		g_hotplug_run = 0;
		ret = 0;
	}

	return ret;
}

void hotplug_thread_release(void)
{
	int 		err;
	void 	*res;

	DEBUG_HOTPLUG_THREAD_PRINTF("\n");
	if (g_hotplug_run)
	{
		g_hotplug_run = 0;
		err = pthread_join(hotplug_thread, &res);
		if (err != 0)
		{
			perror("hotplug thread join faild: ");
		}
	}
}

