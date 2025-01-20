#include "module_data.h"

struct timer_list *info_led_timer;

static int info_time_val;
static int info_led_num;

int gpio_led_num[GPIOCNT] = {518,519,520,521,522,523,524,525};
int gpio_key_num[GPIOCNT] = {528,529,530,531,532,533,534,535};

static DEFINE_MUTEX(keyMutex);

/* Setter */

void set_timer_led(struct timer_list *__cur_led_timer) {
    info_led_timer = __cur_led_timer;
}
void set_time_val(int __cur_time_val) {
    info_time_val = __cur_time_val;
}
void set_led_num(int __cur_led_num) {
    info_led_num = __cur_led_num;
}

/* Getter */

// gpioLed 배열을 반환
int* get_gpio_led(void) {
    return gpio_led_num;
}
// gpioKey 배열을 반환
int* get_gpio_key(void) {
    return gpio_key_num;
}
struct timer_list *get_timer_led(void) {
    return info_led_timer;
}
int get_time_val(void) {
    return info_time_val;
}
int get_led_num(void) {
    return info_led_num;
}
// keyMutex에 대한 getter 함수
struct mutex *get_irq_key_mutex(void) {
    return &keyMutex;
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("PJW");
MODULE_DESCRIPTION("nexusmesh network driver module");


