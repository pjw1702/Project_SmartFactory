#ifndef __LED_SWITCH_H__
#define __LED_SWITCH_H__

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

#include "/home/ubuntu/nexusmeshx/v1.1/module/model/module_data.h"

/* Flag to give meaning */
#define FALSE   0
#define TRUE    1

#define OFF 0
#define ON 1

#define NOT_SET 0
#define SET 1

#define GPIOCNT 8

/* For conditional compile  */
#define DEBUG                   TRUE

int gpio_key_init(void);
int gpio_led_init(void);
void gpio_led_set(long val);
void gpio_led_free(void);
void gpio_key_free(void);

#endif
