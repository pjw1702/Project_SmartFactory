#include "/home/ubuntu/nexusmeshx/v1.1/module/driver/timer/timer.h"

static struct timer_list cur_timer_led;

// 커널 타이머 동작 시, 커널 타이머 기반으로 드라이버 기능을 수행하기 위함
void kernel_timer_func(struct timer_list *__timer) {
	int cur_led_num = get_led_num();
	int cur_time_val = get_time_val();
#if DEBUG
	printk("cur_led_num: %#04x\n", (unsigned int)(cur_led_num));
#endif

	// LED 점등 수행
	gpio_led_set(cur_led_num);

	// 타이머 시간 변동 시, 해당 시간으로 타이머를 구동하기 위함
	mod_timer(__timer, get_jiffies_64()+cur_time_val);
}

// 커널 타이머 동작
void kerneltimer_registertimer(unsigned long __time_over) {
	// 10ms*100=1sec
	cur_timer_led.expires = get_jiffies_64()+__time_over;
	timer_setup(&cur_timer_led, kernel_timer_func, 0);
	add_timer(&cur_timer_led);

	set_timer_led(&cur_timer_led);
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("PJW");
MODULE_DESCRIPTION("nexusmesh network driver module");
