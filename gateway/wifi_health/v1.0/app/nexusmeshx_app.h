#ifndef __NUXUSMESHX_RUN_H__
#define __NEXUSMESHX_RUN_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>
#include <pthread.h>
#include "/home/ubuntu/nexusmeshx/v1.1/utils/wifi/wifi_check.h"

/* Flag to give meaning */
#define NOT_SET 0
#define SET 1

#define NOT_RUNNING 0
#define RUNNING     1

/* For conditional compile  */
#define DEBUG                   1
#define USE_MEGIC_KEY           0

/* Set number of magic for control timer */
#define NEXUSMESHX_TIMER_CONTROL_MAGIC_NUM 		'6'
#define NEXUSMESHX_MAGIC_MAX_NR			20

#define TIMER_START 		_IO(NEXUSMESHX_TIMER_CONTROL_MAGIC_NUM, 0) 
#define TIMER_STOP			_IO(NEXUSMESHX_TIMER_CONTROL_MAGIC_NUM, 1) 
#define TIMER_VALUE	 		_IOW(NEXUSMESHX_TIMER_CONTROL_MAGIC_NUM, 2, keyled_data) 

/* Set IO magic key if compile used magic */
#if USE_MEGIC_KEY
#define GET_WIFI_QUALITY    _IOW(IOCTLTEST_MAGIC, 0, wifi_quality_data)
#define GET_WIFI_TRAFFIC    _IOW(IOCTLTEST_MAGIC, 1, wifi_traffic_tx_data)
#define SET_WIFI_QUALITY    _IOR(IOCTLTEST_MAGIC, 2, wifi_quality_data)
#define SET_WIFI_TRAFFIC    _IOR(IOCTLTEST_MAGIC, 3, wifi_traffic_tx_data)
#endif

#define DEVICE_FILENAME "/dev/nexusmeshx"

#define THREAD_POOL_SIZE    8

/* level of led */
#define LV_1	0x01
#define	LV_2	0x03
#define LV_3	0x07
#define	LV_4	0x0f
#define LV_5	0x1f
#define	LV_6	0x3f
#define LV_7	0x7f
#define LV_FULL 0xff

typedef double wifi_quality_data;
typedef double wifi_traffic_tx_data;
typedef double wifi_traffic_rx_data;

typedef struct {
	unsigned long timer_val;
} __attribute__((packed)) keyled_data;

typedef struct __thread_running_info_ {
    uint8_t check_quality;
    uint8_t check_traffic_tx;
    uint8_t check_traffic_rx;
} __attribute__((packed)) Trinfo;

enum {
    MODE_NONE,
    MODE_QUALITY_CHECK,
    MODE_TRAFFIC_TX_CHECK,
    MODE_TRAFFIC_RX_CHECK,
    MOD_TIMER_STOP,
    MODE_TIMER_TIME_MOD,
    MODE_TIMER_START,
    MODE_APP_SHUTDOWN
};

char get_wifi_network_level(double *__wifi_attr, int __wifi_attr_type);
void check_wifi_quality_handler(void *__dev);
void check_wifi_traffic_tx_handler(void *__dev);
void check_wifi_traffic_rx_handler(void *__dev);
void clean_handler();
int run_nexusmeshx_app(Wifi *__wifi_info, Netlink __nl_info, int __argc, char *__argv[]);
void destroy_nexusmesh_app(int __dev);

#endif
