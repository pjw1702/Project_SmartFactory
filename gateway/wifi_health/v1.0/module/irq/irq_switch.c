#include "/home/ubuntu/nexusmeshx/v1.1/module/irq/irq_switch.h"

static int openFlag = 0;

static DECLARE_WAIT_QUEUE_HEAD(waitQueueRead);

// 키 입력 이벤트 발생해 대한 ISR
irqreturn_t keyIsr(int irq, void * data) {
	int i;
	struct mutex *irq_key_mutex = get_irq_key_mutex();
	keyDataStruct * pKeyData = (keyDataStruct *)data;
	for(i=0;i<GPIOCNT;i++) {
		if(irq == pKeyData->irqKey[i])
		{
			if(mutex_trylock(irq_key_mutex) != 0)
			{
				pKeyData->keyNum = i + 1;
				mutex_unlock(irq_key_mutex);
				break;
			}
		}
	}
#if DEBUG
	printk("keyIsr() irq : %d, keyNum : %d\n",irq, pKeyData->keyNum);
#endif
	wake_up_interruptible(&waitQueueRead);
	return IRQ_HANDLED;
}

// IRQ Switch 관련 자원을 초기화하고 인터럽트를 설정하기 위함
int irq_key_open (struct inode *inode, struct file *filp) {
	int result=0;
	keyDataStruct * pKeyData;
	char * irqName[GPIOCNT] = {"irqKey0","irqKey1","irqKey2","irqKey3","irqKey4","irqKey5","irqKey6","irqKey7",};
#if DEBUG
    int num0 = MAJOR(inode->i_rdev); 
    int num1 = MINOR(inode->i_rdev); 
    printk( "call open -> major : %d\n", num0 );
    printk( "call open -> minor : %d\n", num1 );
#endif

	pKeyData = (keyDataStruct *)kmalloc(sizeof(keyDataStruct),GFP_KERNEL);
	if(!pKeyData)
		return -ENOMEM;

	memset(pKeyData,0,sizeof(keyDataStruct));

	result = irq_key_init(pKeyData);
	if(result < 0)
		return result;

	for(int i=0;i<GPIOCNT;i++) {
		result = request_irq(pKeyData->irqKey[i],keyIsr,IRQF_TRIGGER_RISING,irqName[i],pKeyData);
		if(result < 0 )
			return result;
	}

	if(openFlag)
		return -EBUSY;
	else
		openFlag = SET;

	if(!try_module_get(THIS_MODULE))
		return -ENODEV;

	filp->private_data = pKeyData;
    return 0;
}

// 폴링 방식이 아닌 인터럽트 방식으로 Switch 입력에 대한 이벤트를 처리하기 위함
ssize_t irq_key_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {
	keyDataStruct * pKeyData = (keyDataStruct *)filp->private_data;
	struct mutex *irq_key_mutex = get_irq_key_mutex();
	
	if(!(filp->f_flags & O_NONBLOCK)) {
		wait_event_interruptible(waitQueueRead, pKeyData->keyNum);	
//		wait_event_interruptible_timeout(waitQueueRead, gpioKeyGet(), 100);	 // 100/HZ = 1sec
	}
#if DEBUG
    printk( "call read -> key : %#04x\n", pKeyData->keyNum );
#endif

#if 1
	put_user(pKeyData->keyNum,buf);
#else
	int ret=copy_to_user(buf,&(pKeyData->keyNum),sizeof(pKeyData->keyNum));
	if(ret < 0)
        return ret;
#endif

	if(mutex_trylock(irq_key_mutex) != 0) {
		if(pKeyData->keyNum != 0)
			pKeyData->keyNum = 0;
	}
	mutex_unlock(irq_key_mutex);

    return sizeof(pKeyData->keyNum);
}

// 인터럽트 방식으로 Switch 입력에 대한 LED 점등 제어를 하기 위함
ssize_t irq_led_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos) {
	char kbuf;

#if 1
    get_user(kbuf,buf);
#else
    int ret;
    ret=copy_from_user(&kbuf,buf,sizeof(kbuf));
#endif

#if DEBUG
    printk( "call write -> led : %04X\n", kbuf );
#endif
	//ledVal = kbuf;
	set_led_num(kbuf);
    //gpioLedSet(kbuf);

    return sizeof(kbuf);
}

// 사용자 공간에서 poll 시스템 콜을 사용하여 Switch 입력에 대한 이벤트 처리를 대기하고, 커널에서는 해당 Switch GPIO 파일 디스크립터가 읽기 가능한 상태인지 확인
__poll_t poll_key(struct file * filp, struct poll_table_struct * wait) {

    unsigned int mask=0;
    keyDataStruct * pKeyData = (keyDataStruct *)filp->private_data;
#ifdef DEBUG
    printk("_key : %u\n",(wait->_key & POLLIN));
#endif
    if(wait->_key & POLLIN)
        poll_wait(filp, &waitQueueRead, wait);
    if(pKeyData->keyNum > 0)
        mask = POLLIN;

    return mask;

}
// 파일을 닫을 때 관련된 리소스를 해제하고 타이머를 정리하며 모듈 참조를 감소하기 위함
int irq_key_release (struct inode *inode, struct file *filp) {
	keyDataStruct * pKeyData = (keyDataStruct *)filp->private_data;
    printk( "call release \n" );

	struct timer_list *cur_timer_led = get_timer_led();

	irq_key_free(pKeyData);

	if(timer_pending(cur_timer_led))
		del_timer(cur_timer_led);

	module_put(THIS_MODULE);
	openFlag = 0;

	if(filp->private_data)
		kfree(filp->private_data);
    return 0;
}

// 인터럽트 기반의 키 입력 이벤트 처리를 위해, GPIO 핀을 IRQ(Interrupt Request)와 연결하기 위함
int irq_key_init(keyDataStruct * __key_data) {
	int i;
	int ret=0;
	int *gpioKey = get_gpio_key();
	for(i=0;i<GPIOCNT;i++) {
		__key_data->irqKey[i] = gpio_to_irq(gpioKey[i]);
		if(__key_data->irqKey[i] < 0) {
			printk("Failed gpio_to_irq() gpio%d error\n",gpioKey[i]);
			return __key_data->irqKey[i];
		}
#if DEBUG
		else
			printk("gpio_to_irq() gpio%d (irq%d) \n",gpioKey[i],__key_data->irqKey[i]);
#endif
	}
	return ret;
}

// 프로그램 종료 시, GPIO 핀과 IRQ와의 연결을 해제하기 위함
void irq_key_free(keyDataStruct * __key_data) {
	int i;
	for(i=0;i<GPIOCNT;i++)
		free_irq(__key_data->irqKey[i], __key_data);
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("PJW");
MODULE_DESCRIPTION("nexusmesh network driver module");