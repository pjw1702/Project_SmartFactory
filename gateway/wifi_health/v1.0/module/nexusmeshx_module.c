#include "nexusmeshx_module.h"

/* 타이머 및 LED 관련 전역 변수 */

// 타이머 기본 설정 = 1s
static int timerVal = 100;    // f=100HZ, T=1/100 = 10ms, 100*10ms = 1Sec
static int ledVal = 0;

// 타이머 구조체 선언
static struct timer_list timerLed;

/* 커널 모듈 파라미터 선언 */
module_param(timerVal, int ,0);
module_param(ledVal, int ,0);

// ioctl 기반 처리를 위한 I/O 이벤트에 대한 콜백 함수 등록
struct file_operations ledkey_fops = {
//    .owner    = THIS_MODULE,
    .open     = irq_key_open,     
    .read     = irq_key_read,     
    .write    = irq_led_write,
#if USE_MEGIC_KEY
    .unlocked_ioctl = wifi_ioctl,
#endif
	.poll	  = poll_key,
    .release  = irq_key_release,  
};

// 커널 모듈을 삽입하기 위함
// insmod
int init_nexusmeshx_module(void) {
#if DEBUG
    printk("timerVal: %d, sec: %d\n", timerVal, timerVal/HZ);
#endif
    int result;
    struct mutex *irq_key_mutex = get_irq_key_mutex();

    printk( "call ledkey_init \n" );    

    mutex_init(irq_key_mutex);

    result = gpio_led_init();
    if(result < 0) {
        printk("GPIO LED 초기화 실패: %d\n", result);
        return result;
    }

    result = gpio_key_init();
    if(result < 0) {
        printk("GPIO Key 초기화 실패: %d\n", result);
        return result;
    }

    result = register_chrdev( NEXUSMESHX_DEV_MAJOR, NEXUSMESHX_DEV_NAME, &ledkey_fops);
    if (result < 0) {
        printk("디바이스 적재 실패: %d\n", result);
        return result;
    }

    // 커널 타이머 구동
    kerneltimer_registertimer(timerVal);

    pr_info("the module of nexusmesh initialized!\n");

    return 0;
}

// 커널 모듈 해제시, OS로 부터 할당 받았던 자원을 반납하기 위함
// rmmod
void release_nexusmeshx_module(void) {
    struct mutex *irq_key_mutex = get_irq_key_mutex();
    
    printk( "call ledkey_exit \n" );    

    unregister_chrdev( NEXUSMESHX_DEV_MAJOR, NEXUSMESHX_DEV_NAME );

    if(timer_pending(&timerLed))
        del_timer(&timerLed);

    gpio_led_set(0x00);

    gpio_led_free();
    gpio_key_free();

    mutex_destroy(irq_key_mutex);
}

// 커널 모듈 생성을 위함
module_init(init_nexusmeshx_module);
// 커널 모듈 해제를 위함
module_exit(release_nexusmeshx_module);

// 해당 커널 모듈에 대한 메타데이터 입력
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("PJW");
MODULE_DESCRIPTION("nexusmesh network driver module");
