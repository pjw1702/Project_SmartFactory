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

#include "net.h"

enum {LV_1, LV_2, LV_3, LV_4, LV_5, LV_6, LV_7};

static int gpioLed[GPIOCNT] = {518,519,520,521,522,523,524,525};
static int gpioKey[GPIOCNT] = {528,529,530,531,532,533,534,535};

static int openFlag = 0;

/* timer */
static int timerVal = 100;	//f=100HZ, T=1/100 = 10ms, 100*10ms = 1Sec
static int ledVal = 0;
struct timer_list timerLed;

module_param(timerVal,int ,0);
module_param(ledVal,int ,0);

/* functions for gpios */
static int gpioLedInit(void);
static void gpioLedSet(long val);
static void gpioLedFree(void);
static int gpioKeyInit(void);
//static int gpioKeyGet(void);
static void gpioKeyFree(void);

/* functions for interrupts */
static int irqKeyInit(keyDataStruct * pkeyData);
static void irqKeyFree(keyDataStruct * pKeyData);

/* functions for kernel timer */
void kerneltimer_func(struct timer_list *t);
void kerneltimer_registertimer(unsigned long timeover) {
	// 10ms*100=1sec
	timerLed.expires = get_jiffies_64()+timeover;
	timer_setup(&timerLed, kerneltimer_func, 0);
	add_timer(&timerLed);
}

static DEFINE_MUTEX(keyMutex);
static DECLARE_WAIT_QUEUE_HEAD(waitQueueRead);

// Interrupt Service Routine when accur event when pushed key
irqreturn_t keyIsr(int irq, void * data) {
	int i;
	keyDataStruct * pKeyData = (keyDataStruct *)data;
	for(i=0;i<GPIOCNT;i++)
	{
		if(irq == pKeyData->irqKey[i])
		{
			if(mutex_trylock(&keyMutex) != 0)
			{
				pKeyData->keyNum = i + 1;
				mutex_unlock(&keyMutex);
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


static int ledkey_open (struct inode *inode, struct file *filp) {
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

	result = irqKeyInit(pKeyData);
	if(result < 0)
		return result;

	for(int i=0;i<GPIOCNT;i++)
	{
		result = request_irq(pKeyData->irqKey[i],keyIsr,IRQF_TRIGGER_RISING,irqName[i],pKeyData);
		if(result < 0 )
			return result;
	}

	if(openFlag)
		return -EBUSY;
	else
		openFlag = 1;

	if(!try_module_get(THIS_MODULE))
		return -ENODEV;

	filp->private_data = pKeyData;
    return 0;
}

static ssize_t ledkey_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {
	keyDataStruct * pKeyData = (keyDataStruct *)filp->private_data;


	if(!(filp->f_flags & O_NONBLOCK))
	{
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

	if(mutex_trylock(&keyMutex) != 0)
	{
		if(pKeyData->keyNum != 0)
		{
			pKeyData->keyNum = 0;
		}
	}
	mutex_unlock(&keyMutex);

    return sizeof(pKeyData->keyNum);
}

static ssize_t ledkey_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos) {
	char kbuf;

#if 1
    // get percentage
	get_user(kbuf,buf);
#else
	int ret;
	ret=copy_from_user(&kbuf,buf,sizeof(kbuf));
#endif

#if DEBUG
    printk( "call write -> led : %#04x\n", kbuf );
#endif
    if(kbuf < 30)
        ledVal = LV_1;
    else if(30 <= kbuf && kbuf < 40)
        ledVal = LV_2;
    else if(40 <= kbuf && kbuf < 50)
        ledVal = LV_2;
    else if(50 <= kbuf && kbuf < 60)
        ledVal = LV_3;
    else if(60 <= kbuf && kbuf < 70)
        ledVal = LV_4;
    else if(70 <= kbuf && kbuf < 80)
        ledVal = LV_5;
    else if(80 <= kbuf && kbuf < 90)
        ledVal = LV_6;
    else if(90 <= kbuf)
        ledVal = LV_7;

	//ledVal = kbuf;
	//gpioLedSet(kbuf);
	return sizeof(kbuf);
}

static __poll_t ledkey_poll(struct file * filp, struct poll_table_struct * wait) {

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

static int ledkey_release (struct inode *inode, struct file *filp) {
	keyDataStruct * pKeyData = (keyDataStruct *)filp->private_data;
    printk( "call release \n" );

	irqKeyFree(pKeyData);

	if(timer_pending(&timerLed))
		del_timer(&timerLed);

	module_put(THIS_MODULE);
	openFlag = 0;

	if(filp->private_data)
		kfree(filp->private_data);
    return 0;
}

static int gpioLedInit(void)
{
	int i;
	int ret=0;
	char gpioName[10];
	for(i=0;i<GPIOCNT;i++)
	{
		sprintf(gpioName,"led%d",i);
		ret = gpio_request(gpioLed[i], gpioName);
		if(ret < 0)
		{
			printk("Failed request gpio%d error\n",gpioLed[i]);
			return ret;
		}
		ret = gpio_direction_output(gpioLed[i],OFF);
		if(ret < 0)
		{
			printk("Failed directon_output gpio%d error\n",gpioLed[i]);
			return ret;
		}
	}
	return ret;
}

static void gpioLedSet(long val) {
    int i;
    for(i = 0; i < GPIOCNT; i++) {
        if (val & (1 << i)) {
            gpio_set_value(gpioLed[i], 1);  // LED ON
        } else {
            gpio_set_value(gpioLed[i], 0);  // LED OFF
        }
    }
}

static void gpioLedFree(void) {
	int i;
	for(i=0;i<GPIOCNT;i++)
	{
		gpio_free(gpioLed[i]);
	}
}

static int gpioKeyInit(void) {
	int i;
	int ret=0;
	char gpioName[10];
	for(i=0;i<GPIOCNT;i++)
	{
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
static int irqKeyInit(keyDataStruct * pkeyData)
{
	int i;
	int ret=0;
	for(i=0;i<GPIOCNT;i++)
	{
		pkeyData->irqKey[i] = gpio_to_irq(gpioKey[i]);
		if(pkeyData->irqKey[i] < 0)
		{
			printk("Failed gpio_to_irq() gpio%d error\n",gpioKey[i]);
			return pkeyData->irqKey[i];
		}
#if DEBUG
		else
			printk("gpio_to_irq() gpio%d (irq%d) \n",gpioKey[i],pkeyData->irqKey[i]);
#endif
	}
	return ret;
}

static void irqKeyFree(keyDataStruct * pKeyData)
{
	int i;
	for(i=0;i<GPIOCNT;i++)
	{
		free_irq(pKeyData->irqKey[i], pKeyData);
	}
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

static void gpioKeyFree(void)
{
	int i;
	for(i=0;i<GPIOCNT;i++)
	{
		gpio_free(gpioKey[i]);
	}
}

/* process ioctl from app */

// ioctl function for timer
// static long ledkey_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
// {

// //    printk( "call ioctl -> cmd : %08X, arg : %08X \n", cmd, (unsigned int)arg );
// 	int err=0;
// 	int size=0;
// 	keyled_data info = {0};

// 	if(_IOC_TYPE(cmd) != IOCTLTEST_MAGIC) return -EINVAL;
// 	if(_IOC_NR(cmd) >= IOCTLTEST_MAXNR) return -EINVAL;

// 	size = _IOC_SIZE(cmd);
// 	if(size)   // _IOW
// 	{
// 		if(_IOC_DIR(cmd) & _IOC_WRITE) {
// 			// Check this room is accessible
// 			err = access_ok((void *)arg, size);
// 			if(!err) return err;
// 		}
// 	}
// 	switch(cmd)
// 	{
// 		case TIMER_START:
// 			if(!timer_pending(&timerLed))
// 				kerneltimer_registertimer(timerVal);
// 			break;
// 		case TIMER_STOP:
// 			if(timer_pending(&timerLed))
// 				del_timer(&timerLed);
// 			break;
// 		case TIMER_VALUE:
// 			copy_from_user((void*)&info, (void*)arg, sizeof(keyled_data));
// 			timerVal = info.timer_val;
// 			break;
// 	}
//     return err;
// }
static long wifi_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int err=0;
    int size=0;
    float quality;
    Traffic traffic;

	if(_IOC_TYPE(cmd) != IOCTLTEST_MAGIC) return -EINVAL;
	if(_IOC_NR(cmd) >= IOCTLTEST_MAXNR) return -EINVAL;

	size = _IOC_SIZE(cmd);
    // _IOW
	if(size) {
		if(_IOC_DIR(cmd) & _IOC_WRITE) {
			// Check this room is accessible
			err = access_ok((void *)arg, size);
			if(!err) return err;
		}
	}

    // Magic Number
    switch(cmd) {
        case SET_WIFI_QUALITY:
            // Wi-Fi 품질 정보를 커널 내부에서 처리하고 전달
            if(copy_to_user((float *)arg, &quality, sizeof(quality)))
                return -EFAULT;
            break;
        case SET_WIFI_TRAFFIC:
            // Wi-Fi 트래픽 정보를 커널 내부에서 처리하고 전달
            if(copy_to_user((Traffic *)arg, &traffic, sizeof(traffic)))
                return -EFAULT;
            break;
        default:
            return -EINVAL;
    }

    return 0;
}


void kerneltimer_func(struct timer_list *t) {
	//int result;
#if DEBUG
	printk("ledVal: %#04x\n", (unsigned int)(ledVal));
#endif
	// if (result < 0) return result;

	//gpioLedInit();
	//gpioLedKeyInit();

	gpioLedSet(ledVal);

	//ledVal = register_chrdev( LEDKEY_DEV_MAJOR, LEDKEY_DEV_NAME, &ledkey_fops);
	//ledVal = ~ledVal & 0xff;
	mod_timer(t, get_jiffies_64()+timerVal);
}

struct file_operations ledkey_fops =
{
//    .owner    = THIS_MODULE,
    .open     = ledkey_open,     
    .read     = ledkey_read,     
    .write    = ledkey_write,
	//.unlocked_ioctl = ledkey_ioctl,   
    .unlocked_ioctl = wifi_ioctl,
	.poll	  = ledkey_poll,
    .release  = ledkey_release,  
};

/* Initialization */
static int ledkey_kerneltimer_init(void) {
#if DEBUG
	printk("timerVal: %d, sec: %d\n", timerVal, timerVal/HZ);
#endif
    int result;

	//kerneltimer_registertimer(timerVal);

    printk( "call ledkey_init \n" );    

	mutex_init(&keyMutex);

	result = gpioLedInit();
	if(result < 0)
		return result;

	result = gpioKeyInit();
	if(result < 0)
		return result;


    result = register_chrdev( LEDKEY_DEV_MAJOR, LEDKEY_DEV_NAME, &ledkey_fops);
    if (result < 0) return result;

    pr_info("nexusmesh device initialized\n");

    return 0;
}

/* Destroy */
static void ledkey_kerneltimer_exit(void)
{

	// if(timer_pending(&timerLed))
	// 	del_timer(&timerLed);

    printk( "call ledkey_exit \n" );    

	unregister_chrdev( LEDKEY_DEV_MAJOR, LEDKEY_DEV_NAME );

	if(timer_pending(&timerLed))
		del_timer(&timerLed);

	gpioLedSet(0x00);

	gpioLedFree();
	gpioKeyFree();
	mutex_destroy(&keyMutex);

}

/* Close module */

module_init(ledkey_kerneltimer_init);
module_exit(ledkey_kerneltimer_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("kcci");
MODULE_DESCRIPTION("nexusmesh network driver module");
