#include "ioctl.h"

// 필요시, 매직 키 기반의 사용자 레벨로 부터의 데이터를 전달 받고 처리하기 위함
#if USE_MEGIC_KEY
long wifi_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int err=0;
    int size=0;
    wifi_quality_data quality;
    wifi_traffic_tx_data traffic;

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

    switch(cmd) {
        case GET_WIFI_QUALITY:
            // Wi-Fi 품질 정보를 커널 내부에서 처리하고 전달
            if(copy_from_user((void *)&quality, (void *)arg, sizeof(wifi_quality_data)))
                return -EFAULT;
			qualVal = (int)quality;
			printk("quality: %d\n", qualVal);
            break;
        case GET_WIFI_TRAFFIC:
            // Wi-Fi 트래픽 정보를 커널 내부에서 처리하고 전달
            if(copy_from_user((void *)&traffic, (void *)arg, sizeof(wifi_traffic_tx_data)))
                return -EFAULT;
			trafVal = (int)traffic;
			printk("traffics: %d\n", trafVal);
            break;
        default:
            return -EINVAL;
    }

    return 0;
}
#endif

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("PJW");
MODULE_DESCRIPTION("nexusmesh network driver module");