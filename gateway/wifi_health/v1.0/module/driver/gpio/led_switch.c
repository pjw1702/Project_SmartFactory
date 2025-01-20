#include "/home/ubuntu/nexusmeshx/v1.1/module/driver/gpio/led_switch.h"

static int gpioLedInitFlag = NOT_SET;
//static int gpioKeyInitFlag = NOT_SET;

// 스위치를 통한 키 입력을 위해 OS로 부터 GPIO를 할당받기 위함
int gpio_key_init(void) {
	int i;
	int ret=0;
	char gpioName[10];
	int *gpioKey = get_gpio_key();
	for(i=0;i<GPIOCNT;i++) {
		sprintf(gpioName,"key%d",i);
		ret = gpio_request(gpioKey[i], gpioName);
		if(ret < 0) {
			printk("Failed Request gpio%d error\n", gpioKey[i]);
			return ret;
		}
		ret = gpio_direction_input(gpioKey[i]);
		if(ret < 0) {
			printk("Failed direction_output gpio%d error\n", gpioKey[i]);
       	 	return ret;
		}
	}

	return ret;
}

#if 0
static int	gpioKeyGet(void)
{
	int i;
	int ret;
	int keyData=0;
	for(i=0;i<GPIOCNT;i++)
	{
//		ret=gpio_get_value(gpioKey[i]) << i;
//		keyData |= ret;
		ret=gpio_get_value(gpioKey[i]);
		keyData = keyData | ( ret << i );
	}
	return keyData;
}
#endif 

// LED 점등을 위해 OS로 부터 GPIO를 할당받기 위함
int gpio_led_init(void) {
	int i;
	int ret=0;
	char gpioName[10];
	int *gpioLed = get_gpio_led();
	for(i=0;i<GPIOCNT;i++) {
		sprintf(gpioName,"led%d",i);
		ret = gpio_request(gpioLed[i], gpioName);
		if(ret < 0) {
			printk("Failed request gpio%d error\n",gpioLed[i]);
			return ret;
		}
		ret = gpio_direction_output(gpioLed[i],OFF);
		if(ret < 0) {
			printk("Failed directon_output gpio%d error\n",gpioLed[i]);
			return ret;
		}
	}

    gpioLedInitFlag = SET;
	return ret;
}

// LED를 점등하기 하기 위함
void gpio_led_set(long val) {
	int *gpioLed = get_gpio_led();
	int i;

	for(i=0;i<GPIOCNT;i++)
        gpio_set_value(gpioLed[i],(val & (0x01 << i)));
}

// 프로그램 종료 시, OS에 의해 할당되었던 GPIO를 정상적으로 반납하기 위함
void gpio_led_free(void) {
	int *gpioLed = get_gpio_led();
	int i;

	for(i=0;i<GPIOCNT;i++)
		gpio_free(gpioLed[i]);

    gpioLedInitFlag = NOT_SET;
}

// 프로그램 종료 시, OS에 의해 할당 받았던 스위치 용 GPIO를 반납하기 위함
void gpio_key_free(void) {
	int *gpioKey = get_gpio_key();
	int i;
	for(i=0;i<GPIOCNT;i++)
		gpio_free(gpioKey[i]);
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("PJW");
MODULE_DESCRIPTION("nexusmesh network driver module");