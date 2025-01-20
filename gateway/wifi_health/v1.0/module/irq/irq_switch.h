#ifndef __IRQ_SWITCH_H__
#define __IRQ_SWITCH_H__

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
#include "/home/ubuntu/nexusmeshx/v1.1/module/model/module_data.h"

/* Flag to give meaning */
#define FALSE   0
#define TRUE    1

#define OFF 0
#define ON 1

#define NOT_SET 0
#define SET 1

/* For set size */
#define GPIOCNT 8

/* For conditional compile  */
#define DEBUG                   TRUE
#define USE_MEGIC_KEY           FALSE
#define USE_TIMER_CONTROLLER    FALSE

typedef struct {
	int keyNum;
	int irqKey[GPIOCNT];
} __attribute__((packed)) keyDataStruct;

irqreturn_t keyIsr(int irq, void * data);
int irq_key_open (struct inode *inode, struct file *filp);
ssize_t irq_key_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t irq_led_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos);
 __poll_t poll_key(struct file * filp, struct poll_table_struct * wait);
int irq_key_release (struct inode *inode, struct file *filp);
int irq_key_init(keyDataStruct * __key_data);
void irq_key_free(keyDataStruct * __key_data);

#endif
