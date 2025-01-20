
#include "nexusmeshx_app.h"

pthread_mutex_t dev_mutex;
pthread_mutex_t thread_pool_mutex;
pthread_mutex_t wifi_info_mutex;
pthread_t wifi_thread_pool[THREAD_POOL_SIZE];

Trinfo thread;

char *server_ip;

Wifi *wifi_info;
Netlink nl_info;

int interval_time;
long usleep_time;

// LED 점등 수를 구하기 위한 헬퍼 함수
char get_wifi_network_level(double *__wifi_attr, int __wifi_attr_type) {
    char led_no;

    switch(__wifi_attr_type) {
        case LINK_QUALITY:
            if(*__wifi_attr < 90)
                led_no = LV_1;
            else if(90 <= *__wifi_attr && *__wifi_attr < 92)
                led_no = LV_2;
            else if(92 <= *__wifi_attr && *__wifi_attr < 94)
                led_no = LV_2;
            else if(94 <= *__wifi_attr && *__wifi_attr < 95)
                led_no = LV_3;
            else if(95 <= *__wifi_attr && *__wifi_attr < 96)
                led_no = LV_4;
            else if(96 <= *__wifi_attr && *__wifi_attr < 97)
                led_no = LV_5;
            else if(97 <= *__wifi_attr && *__wifi_attr < 98)
                led_no = LV_6;
            else if(98 <= *__wifi_attr && *__wifi_attr < 99)
                led_no = LV_7;
            else led_no = LV_FULL;
            break;
        case CURRENT_TRAFFIC:
            if(*__wifi_attr < 0.1)
                led_no = LV_1;
            else if(0.1 <= *__wifi_attr && *__wifi_attr < 0.3)
                led_no = LV_2;
            else if(0.3 <= *__wifi_attr && *__wifi_attr < 0.4)
                led_no = LV_2;
            else if(0.4 <= *__wifi_attr && *__wifi_attr < 0.5)
                led_no = LV_3;
            else if(0.5 <= *__wifi_attr && *__wifi_attr < 0.6)
                led_no = LV_4;
            else if(0.6 <= *__wifi_attr && *__wifi_attr < 1.0)
                led_no = LV_5;
            else if(1.0 <= *__wifi_attr && *__wifi_attr < 1.2)
                led_no = LV_6;
            else if(1.3 <= *__wifi_attr && *__wifi_attr < 1.4)
                led_no = LV_7;
            else led_no = LV_FULL;
            break;
    }

    return led_no;
}

// Wi-Fi 품질 체크 핸들러
void check_wifi_quality_handler(void *__dev) {
    double wifi_quality;
    char led_no;

    pthread_mutex_lock(&dev_mutex);
    led_no = 0x00;
    if (write(*(int *)__dev, &led_no, sizeof(led_no)) < 0) {
        perror("write failed");
        pthread_mutex_unlock(&dev_mutex);
        pthread_exit(NULL);
    }
    pthread_mutex_unlock(&dev_mutex);

    while (1) {
        wifi_quality = check_wifi_quality(wifi_info);

        led_no = get_wifi_network_level(&wifi_quality, LINK_QUALITY);

        pthread_mutex_lock(&dev_mutex);
        if (write(*(int *)__dev, &led_no, sizeof(led_no)) < 0) {
            perror("write failed");
            pthread_mutex_unlock(&dev_mutex);
            break;
        }
        pthread_mutex_unlock(&dev_mutex);

        usleep(usleep_time);
    }
}

// Wi-Fi 송신 트래픽를 확인하는 스레드
void check_wifi_traffic_tx_handler(void *__dev) {
    char led_no;
    double wifi_traffic_tx_load;
    //Traffic traffic;

//===================================================================================================================
    pthread_mutex_lock(&wifi_info_mutex);
    get_wifi_status(&nl_info, wifi_info);
    pthread_mutex_unlock(&wifi_info_mutex);
//===================================================================================================================

    pthread_mutex_lock(&dev_mutex);
    led_no = 0x00;
    if (write(*(int *)__dev, &led_no, sizeof(led_no)) < 0) {
        perror("write failed");
        pthread_mutex_unlock(&dev_mutex);
        pthread_exit(NULL);
    }
    pthread_mutex_unlock(&dev_mutex);

    while (1) {
        //bandwidth = get_rx_tx_bandwidth(run_iperf3_client, server_ip);

        //wifi_info->traffic = get_rx_tx_traffic();
        //traffic = get_rx_tx_traffic();
        wifi_info->traffic->tx_bitrate_mbps = get_current_traffic(wifi_info, interval_time, TRAFFIC_TX);

        wifi_traffic_tx_load = (wifi_info->traffic->tx_bitrate_mbps / wifi_info->bandwidth->tx_bitrate_mbps) * 100;

        led_no = get_wifi_network_level(&wifi_traffic_tx_load, CURRENT_TRAFFIC);

        pthread_mutex_lock(&dev_mutex);
        if (write(*(int *)__dev, &led_no, sizeof(led_no)) < 0) {
            perror("write failed");
            pthread_mutex_unlock(&dev_mutex);
            break;
        }
        pthread_mutex_unlock(&dev_mutex);

        printf("TX Traffic Load: %.2lf %%\n", wifi_traffic_tx_load);

        usleep(usleep_time);
    }
}

// Wi-Fi 수신 트래픽를 확인하는 스레드
void check_wifi_traffic_rx_handler(void *__dev) {
    char led_no;
    double wifi_traffic_rx_load;
    //Traffic traffic;
//===================================================================================================================
	// wifi 정보 메모리 할당
    // wifi_info = alloc_wifi_info_space();

    // // wifi 정보 제어
	// nl_info.id = init_wifi_attr(&nl_info, wifi_info);
	// if (nl_info.id < 0) {
	// 	fprintf(stderr, "Error initializing netlink 802.11\n");
	// 	return -1;
	// }

    // get_wifi_status(&nl_info, wifi_info);

    pthread_mutex_lock(&wifi_info_mutex);
    get_wifi_status(&nl_info, wifi_info);
    pthread_mutex_unlock(&wifi_info_mutex);
//===================================================================================================================

    pthread_mutex_lock(&dev_mutex);
    led_no = 0x00;
    if (write(*(int *)__dev, &led_no, sizeof(led_no)) < 0) {
        perror("write failed");
        pthread_mutex_unlock(&dev_mutex);
        pthread_exit(NULL);
    }
    pthread_mutex_unlock(&dev_mutex);

    while (1) {
        //bandwidth = get_rx_tx_bandwidth(run_iperf3_client, server_ip);

        //wifi_info->traffic = get_rx_tx_traffic();
        //traffic = get_rx_tx_traffic();
        wifi_info->traffic->rx_bitrate_mbps = get_current_traffic(wifi_info, interval_time, TRAFFIC_RX);

        wifi_traffic_rx_load = (wifi_info->traffic->rx_bitrate_mbps / wifi_info->bandwidth->rx_bitrate_mbps) * 100;

        led_no = get_wifi_network_level(&wifi_traffic_rx_load, CURRENT_TRAFFIC);

        pthread_mutex_lock(&dev_mutex);
        if (write(*(int *)__dev, &led_no, sizeof(led_no)) < 0) {
            perror("write failed");
            pthread_mutex_unlock(&dev_mutex);
            break;
        }
        pthread_mutex_unlock(&dev_mutex);

        printf("RX Traffic Load: %.2lf %%\n", wifi_traffic_rx_load);

        usleep(usleep_time);
    }
}

// 스레드 실행 도중 키 입력 시, 이전에 실행되었던 스레드를 완전히 소멸시키기 위한 헬퍼 함수
void clean_handler() {
    if(thread.check_quality == RUNNING) {
        pthread_cancel(wifi_thread_pool[MODE_QUALITY_CHECK]);
        pthread_join(wifi_thread_pool[MODE_QUALITY_CHECK], NULL);
        thread.check_quality = NOT_RUNNING;
    } else if(thread.check_traffic_tx == RUNNING) {
        pthread_cancel(wifi_thread_pool[MODE_QUALITY_CHECK]);
        pthread_join(wifi_thread_pool[MODE_QUALITY_CHECK], NULL);
        thread.check_traffic_tx = NOT_RUNNING;
    } else if(thread.check_traffic_rx == RUNNING) {
        pthread_cancel(wifi_thread_pool[MODE_QUALITY_CHECK]);
        pthread_join(wifi_thread_pool[MODE_QUALITY_CHECK], NULL);
        thread.check_traffic_rx = NOT_RUNNING;
    }
}

// nexusmeshx의 사용자 공간 스레드를 실행하기 위함
int run_nexusmeshx_app(Wifi *__wifi_info, Netlink __nl_info, int __argc, char *__argv[]) {
    int dev;
    char key_no;
    char led_no;
    char timer_val;
    int ret;
    int loopFlag = 1;
    struct pollfd Events[2];
    char inputString[80];
    keyled_data info;

//===================================================================================================================
    wifi_info = __wifi_info;
    nl_info = __nl_info;
//===================================================================================================================

    if(__argc != 4) {
        printf("Usage : %s [ledVal(0x00~0xff)] [timerVal(1/100)] [intervalTime(int)]\n", __argv[0]);
        return 1;
    }

    led_no = (char)strtoul(__argv[1], NULL, 16);
    if(!((0 <= led_no) && (led_no <= 255))) {
        printf("Usage : %s [led_data(0x00~0xff)]\n", __argv[0]);
        return 2;
    }

    // 타이머 시간 초기화
    printf("Author: PJW\n");
    timer_val = atoi(__argv[2]);
    info.timer_val = timer_val;

    // Wifi 상태 체크 시간 간격 초기화
    interval_time = atoi(__argv[3]);
    usleep_time = interval_time * 1000000;
    printf("Inverval Time: %ds\n", interval_time);

    dev = open( DEVICE_FILENAME, O_RDWR);
    if(dev < 0) {
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

	// wifi 정보 메모리 할당
    wifi_info = alloc_wifi_info_space();

    // wifi 정보 제어
	nl_info.id = init_wifi_attr(&nl_info, wifi_info);
	if (nl_info.id < 0) {
		fprintf(stderr, "Error initializing netlink 802.11\n");
		return -1;
	}

    get_wifi_status(&nl_info, wifi_info);

    while(loopFlag) {
        ret = poll(Events, 2, 1000);
        if(ret == 0) {
            continue;
        }

        // 키를 눌렀을 때의 이벤트 처리를 위함
        if(Events[0].revents & POLLIN) {
            read(dev, &key_no, sizeof(key_no));
            switch(key_no) {
                // Wi-Fi 품질 체크 모드
                case MODE_QUALITY_CHECK:
                    pthread_mutex_lock(&thread_pool_mutex);
                    clean_handler();
                    pthread_mutex_unlock(&thread_pool_mutex);

                    printf("MODE: QUALITY CHECK\n");
                    printf("Performing Wi-Fi Quality Check...\n");
                    // Wi-Fi 품질 데이터를 커널로 전달
                    pthread_mutex_lock(&thread_pool_mutex);
                    if (pthread_create(&wifi_thread_pool[MODE_QUALITY_CHECK], NULL, (void *)check_wifi_quality_handler, &dev) != 0) {
                        perror("Thread creation failed");
                        exit(EXIT_FAILURE);
                    }
                    thread.check_quality = RUNNING;
                    pthread_mutex_unlock(&thread_pool_mutex);
                    break;
                // Wi-Fi 송신 트래픽 체크 모드
                case MODE_TRAFFIC_TX_CHECK:
                    pthread_mutex_lock(&thread_pool_mutex);
                    clean_handler();
                    pthread_mutex_unlock(&thread_pool_mutex);

                    printf("MODE: TRAFFIC OF TX CHECK\n");
                    printf("Performing Traffic of tx Check...\n");
                    // Wi-Fi 송신 트래픽 데이터를 커널로 전달
                    pthread_mutex_lock(&thread_pool_mutex);
                    if (pthread_create(&wifi_thread_pool[MODE_TRAFFIC_TX_CHECK], NULL, (void *)check_wifi_traffic_tx_handler, &dev) != 0) {
                        perror("Thread creation failed");
                        exit(EXIT_FAILURE);
                    }
                    thread.check_traffic_tx = RUNNING;
                    pthread_mutex_unlock(&thread_pool_mutex);
                    break;
                // Wi-Fi 수신 트래픽 체크 모드
                case MODE_TRAFFIC_RX_CHECK:
                    pthread_mutex_lock(&thread_pool_mutex);
                    clean_handler();
                    pthread_mutex_unlock(&thread_pool_mutex);

                    printf("MODE: TRAFFIC OF RX CHECK\n");
                    printf("Performing traffic of rx Check...\n");
                    // Wi-Fi 수신 트래픽 데이터를 커널로 전달
                    pthread_mutex_lock(&thread_pool_mutex);
                    if (pthread_create(&wifi_thread_pool[MODE_TRAFFIC_RX_CHECK], NULL, (void *)check_wifi_traffic_rx_handler, &dev) != 0) {
                        perror("Thread creation failed");
                        exit(EXIT_FAILURE);
                    }
                    thread.check_traffic_rx = RUNNING;
                    pthread_mutex_unlock(&thread_pool_mutex);
                    break;

                // 8번 키를 눌렀을 때 정상 종료를 하기 위함
                case MODE_APP_SHUTDOWN:
                    pthread_mutex_lock(&thread_pool_mutex);
                    clean_handler();
                    pthread_mutex_unlock(&thread_pool_mutex);
                    printf("APP CLOSE!\n");
                    loopFlag = 0;
                    break;
            }
        }
        // 'q' 또는 'Q' 키보드 키를 눌렀을 때의 이벤트를 처리하기 위함
        else if(Events[1].revents & POLLIN) {
            fgets(inputString, sizeof(inputString), stdin);
            if((inputString[0] == 'q') || (inputString[0] == 'Q'))
                break;
            inputString[strlen(inputString) - 1] = '\0';

            key_no = 0;
        }
    }

    return dev;
}

// nuxusmeshx의 스레드 종료를 위함
void destroy_nexusmesh_app(int __dev) {
    char led_no = 0x00;

    if (write(__dev, &led_no, sizeof(led_no)) < 0)
        perror("write failed");
    close(__dev);
    pthread_mutex_destroy(&dev_mutex);
}
