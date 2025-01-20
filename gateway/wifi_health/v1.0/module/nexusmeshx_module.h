#ifndef __NEXUSMESHX_MODULE__
#define __NEXUSMESHX_MODULE__

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

#include "/home/ubuntu/nexusmeshx/v1.1/module/driver/timer/timer.h"
#include "/home/ubuntu/nexusmeshx/v1.1/module/driver/gpio/led_switch.h"
#include "/home/ubuntu/nexusmeshx/v1.1/module/driver/ioctl/ioctl.h"
#include "/home/ubuntu/nexusmeshx/v1.1/module/irq/irq_switch.h"
#include "/home/ubuntu/nexusmeshx/v1.1/module/model/module_data.h"

/* Flag to give meaning */
#define FALSE   0
#define TRUE    1

#define OFF 0
#define ON 1

#define NOT_SET 0
#define SET 1

/* For conditional compile  */
#define DEBUG                   TRUE
#define USE_MEGIC_KEY           FALSE
#define USE_TIMER_CONTROLLER    FALSE

#define NEXUSMESHX_DEV_NAME  "nexusmeshx"   
#define NEXUSMESHX_DEV_MAJOR    230

int init_kernel_resource(void);
void release_kernel_resource(void);

#endif
