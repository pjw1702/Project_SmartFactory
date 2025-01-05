#ifndef __NET_H__
#define __NET_H__

/* For app */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>
#include "wifi/wifi_check.h"

#define IOCTLTEST_MAGIC 		'6'
#define IOCTLTEST_MAXNR			7

// #define GET_WIFI_QUALITY		    _IO(IOCTLTEST_MAGIC, 0) 
// #define GET_WIFI_TRAFFIC			_IO(IOCTLTEST_MAGIC, 1) 
// #define TIMER_VALUE	 		        _IOW(IOCTLTEST_MAGIC, 2, keyled_data) 

#define GET_WIFI_QUALITY  _IOR('w', 0, double)
#define GET_WIFI_TRAFFIC  _IOR('w', 1, double)
#define SET_WIFI_QUALITY  _IOW('w', 2, double)
#define SET_WIFI_TRAFFIC  _IOW('w', 3, double)
#define TIMER_START 	    _IO(IOCTLTEST_MAGIC, 4) 
#define TIMER_STOP			_IO(IOCTLTEST_MAGIC, 5) 
#define TIMER_VALUE	 		_IOW(IOCTLTEST_MAGIC, 6, keyled_data)

#define DEBUG 1

#define   LEDKEY_DEV_NAME            "nexusmesh"
#define   LEDKEY_DEV_MAJOR            230      

#define OFF 0
#define ON 1
#define GPIOCNT 8

#define DEVICE_FILENAME "/dev/nexusmesh_dev"

typedef struct {
	unsigned long timer_val;
} __attribute__((packed)) keyled_data;

typedef struct {
	int keyNum;
	int irqKey[GPIOCNT];
} __attribute__((packed)) keyDataStruct;

typedef enum {
    MODE_NONE,
    MODE_QUALITY_CHECK,
    MODE_TRAFFIC_CHECK
} operation_mode;

#endif
