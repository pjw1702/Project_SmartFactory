#include "net.h"

int main(int argc, char *argv[])
{
    int dev;
    char key_no;
    char led_no;
    char timer_val;
    int ret;
    int loopFlag = 1;
    struct pollfd Events[2];
    char inputString[80];
    keyled_data info;
    operation_mode current_mode = MODE_NONE;

    Bandwidth bandwidth;
    Traffic traffic;
    double quality;

    char *server_ip = get_eth0_ip();

    if(argc != 3)
    {
        printf("Usage : %s [led_val(0x00~0xff)] [timer_val(1/100)]\n", argv[0]);
        return 1;
    }
    led_no = (char)strtoul(argv[1], NULL, 16);
    if(!((0 <= led_no) && (led_no <= 255)))
    {
        printf("Usage : %s [led_data(0x00~0xff)]\n", argv[0]);
        return 2;
    }
    printf("Author: PJW\n");
    timer_val = atoi(argv[2]);
    info.timer_val = timer_val;

    dev = open(DEVICE_FILENAME, O_RDWR);
    if(dev < 0)
    {
        perror("open");
        return 2;
    }

    ioctl(dev, TIMER_VALUE, &info);
    write(dev, &led_no, sizeof(led_no));
    ioctl(dev, TIMER_START);

    memset(Events, 0, sizeof(Events));

    Events[0].fd = dev;
    Events[0].events = POLLIN;
    Events[1].fd = fileno(stdin);
    Events[1].events = POLLIN;

    while(loopFlag)
    {
        ret = poll(Events, 2, 1000);
        if(ret == 0)
        {
            continue;
        }

        // Events when input key
        if(Events[0].revents & POLLIN)  // dev : keyled
        {
            read(dev, &key_no, sizeof(key_no));
            printf("key_no : %d\n", key_no);
            switch(key_no) {
                case 1:
                    ioctl(dev, GET_WIFI_QUALITY, &quality);  // Wi-Fi 품질 데이터를 커널로 전달
                    printf("MODE: QUALITY CHECK\n");
                    current_mode = MODE_QUALITY_CHECK;
                    break;
                case 2:
                    ioctl(dev, GET_WIFI_TRAFFIC, &traffic);  // Wi-Fi 트래픽 데이터를 커널로 전달
                    printf("MODE: TRAFFIC CHECK\n");
                    current_mode = MODE_TRAFFIC_CHECK;
                    break;
                case 8:
                    printf("APP CLOSE!\n");
                    loopFlag = 0;
                    break;
            }
        }
        // Events when input value
        else if(Events[1].revents & POLLIN) // keyboard
        {
            fgets(inputString, sizeof(inputString), stdin);
            if((inputString[0] == 'q') || (inputString[0] == 'Q'))
                break;
            inputString[strlen(inputString) - 1] = '\0';

            // check quality of wifi
			if(key_no == 1) {
                printf("Performing Wi-Fi Quality Check...\n");
                // 품질 체크 관련 작업 추가
                check_wifi_frequency();
                quality = check_wifi_quality();

                ioctl(dev, SET_WIFI_QUALITY, quality);
			}

            // check traffic of wifi
			if(key_no == 2) {
                printf("Performing Traffic Check...\n");
                // get bandwidth
                bandwidth = get_rx_tx_bandwidth(run_iperf3_client, server_ip);

                // get traffic
                traffic = get_rx_tx_traffic();

                ioctl(dev, SET_WIFI_TRAFFIC, (traffic.tx_bitrate_mbps/bandwidth.tx_bitrate_mbps)*100);

                printf("=================================== Wi-Fi Traffics ===================================\n");
                printf("rx traffics: %.2lf %%\n", (traffic.rx_bitrate_mbps/bandwidth.rx_bitrate_mbps)*100);
                printf("tx traffics: %.2lf %%\n", (traffic.tx_bitrate_mbps/bandwidth.tx_bitrate_mbps)*100);
			}

            key_no = 0;
        }
    }

    close(dev);
    return 0;
}
