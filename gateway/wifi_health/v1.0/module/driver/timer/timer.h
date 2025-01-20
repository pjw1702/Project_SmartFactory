#ifndef __KERNEL_TIMER_H__
#define __KERNEL_TIMER_H__

#include <linux/kernel.h>     
#include <linux/gpio.h>

#include "/home/ubuntu/nexusmeshx/v1.1/module/model/module_data.h"
#include "/home/ubuntu/nexusmeshx/v1.1/module/driver/gpio/led_switch.h"

/* For mod timer's time */

// typedef struct {
// 	unsigned long timer_val;
// } __attribute__((packed)) keyled_data;

void kerneltimer_registertimer(unsigned long __time_over);
void kernel_timer_func(struct timer_list *t);

#endif
