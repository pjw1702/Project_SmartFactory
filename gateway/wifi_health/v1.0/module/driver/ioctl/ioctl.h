#ifndef __IOCTL_H__
#define __IOCTL_H__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>          
#include <linux/errno.h>       
#include <linux/types.h>       
#include <linux/fcntl.h>       

#include <linux/gpio.h>
#include <linux/moduleparam.h>

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/poll.h>

#include "/home/ubuntu/nexusmeshx/v1.1/module/driver/gpio/led_switch.h"

/* Flag to give meaning */
#define FALSE   0
#define TRUE    1

#define NOT_SET 0
#define SET 1

/* For conditional compile  */
#define DEBUG                   TRUE
#define USE_MEGIC_KEY           FALSE

/* Set magic number if compile used magic */
#if USE_MEGIC_KEY
#define IOCTLTEST_MAGIC 		'6'
#define IOCTLTEST_MAXNR			7

#define GET_WIFI_QUALITY    _IOW(IOCTLTEST_MAGIC, 0, wifi_quality_data)
#define GET_WIFI_TRAFFIC    _IOW(IOCTLTEST_MAGIC, 1, wifi_traffic_tx_data)
#define SET_WIFI_QUALITY    _IOR(IOCTLTEST_MAGIC, 2, wifi_quality_data)
#define SET_WIFI_TRAFFIC    _IOR(IOCTLTEST_MAGIC, 3, wifi_traffic_tx_data)
#define TIMER_START 	    _IO(IOCTLTEST_MAGIC, 4) 
#define TIMER_STOP			_IO(IOCTLTEST_MAGIC, 5) 
#define TIMER_VALUE	 		_IOW(IOCTLTEST_MAGIC, 6, keyled_data)
#endif

/* For save data of wifi attribute via user side */
#if USE_MEGIC_KEY
typedef double wifi_quality_data;
typedef double wifi_traffic_tx_data;
typedef double wifi_traffic_rx_data;
#endif

long wifi_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

#endif
