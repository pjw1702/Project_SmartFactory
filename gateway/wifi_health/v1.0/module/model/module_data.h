#ifndef __MODULE_DATA_H__
#define __MODULE_DATA_H__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/timer.h> 

#define GPIOCNT 8

void set_timer_led(struct timer_list *__cur_led_timer);
void set_time_val(int __cur_time_val);
void set_led_num(int __cur_led_num);

int* get_gpio_led(void);
int* get_gpio_key(void);
struct timer_list *get_timer_led(void);
int get_time_val(void);
int get_led_num(void);
struct mutex *get_irq_key_mutex(void);

#endif
