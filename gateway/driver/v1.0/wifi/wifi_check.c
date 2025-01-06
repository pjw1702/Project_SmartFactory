#include "wifi_check.h"

char* get_eth0_ip() {
    FILE *fp;
    char path[1035];
    static char ip[INET_ADDRSTRLEN];  // static으로 선언하여 반환된 주소가 유효하도록 처리

    // ip a 명령어 실행하여 결과 가져오기
    fp = popen("ip a show eth0", "r");
    if (fp == NULL) {
        perror("Failed to run ip command");
        return NULL;
    }

    // 출력된 결과에서 IP 주소 추출
    while (fgets(path, sizeof(path)-1, fp) != NULL) {
        // "inet"을 포함하는 라인에서 IP 주소 추출
        if (strstr(path, "inet ") != NULL) {
            char *ip_start = strstr(path, "inet ") + 5;  // "inet " 이후부터 IP 주소 시작
            char *ip_end = strchr(ip_start, '/');        // 서브넷 마스크 구분자 '/' 앞까지
            if (ip_end != NULL) {
                *ip_end = '\0';  // IP 주소의 끝을 null로 처리
                strncpy(ip, ip_start, sizeof(ip));
                pclose(fp);
                return ip;  // 추출한 IP 반환
            }
        }
    }

    pclose(fp);
    return NULL;  // IP 주소를 찾을 수 없으면 NULL 반환
}

// 주파수 정보를 처리하는 함수
int parse_response(struct nl_msg *msg, void *arg) {
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    int channel;
    float frequency;
    int channel_width;

    // 디버깅: 메시지 덤프
    //nl_msg_dump(msg, stdout);

    // 응답에서 필요한 속성 추출
    if (nlmsg_parse(nlmsg_hdr(msg), GENL_HDRLEN, tb, NL80211_ATTR_MAX, NULL) < 0) {
        fprintf(stderr, "Failed to parse netlink message\n");
        return NL_SKIP;
    }

    printf("=================================== Wi-Fi Frequencies ===================================\n");

    // 대역폭 정보 추출 (20 MHz, 40 MHz, 80 MHz 등)
    if (tb[NL80211_ATTR_CHANNEL_WIDTH]) {
        channel_width = nla_get_u32(tb[NL80211_ATTR_CHANNEL_WIDTH]);
        switch (channel_width) {
            case NL80211_CHAN_WIDTH_20:
                printf("Channel Width: 20 MHz\n");
                break;
            case NL80211_CHAN_WIDTH_40:
                printf("Channel Width: 40 MHz\n");
                break;
            case NL80211_CHAN_WIDTH_80:
                printf("Channel Width: 80 MHz\n");
                break;
            case NL80211_CHAN_WIDTH_160:
                printf("Channel Width: 160 MHz\n");
                break;
            default:
                printf("Unknown Channel Width\n");
        }
    }

    // 채널 정보 추출
    if (tb[NL80211_ATTR_CHANNEL_WIDTH]) {
        channel = nla_get_u32(tb[NL80211_ATTR_CHANNEL_WIDTH]);
        printf("Number of channel: %d\n", channel);
    } else {
        printf("Channel information not available\n");
    }

    // 주파수 추출
    if (tb[NL80211_ATTR_WIPHY_FREQ]) {
        frequency = nla_get_u32(tb[NL80211_ATTR_WIPHY_FREQ]) / 1000.0;
        printf("Frequency: %.3f MHz\n", frequency);
    } else {
        printf("Frequency information not available\n");
    }

    return NL_OK;
}

// Wi-Fi 주파수 측정 함수
void check_wifi_frequency() {
    struct nl_sock *sk;
    struct nl_msg *msg;
    int ret;
    int ifindex;

    // 소켓 생성
    sk = nl_socket_alloc();
    if (!sk) {
        perror("Failed to allocate netlink socket");
        exit(EXIT_FAILURE);
    }

    // nl80211을 지원하는 제네릭 netlink ID 얻기
    ret = genl_connect(sk);
    if (ret < 0) {
        perror("Failed to connect to netlink");
        exit(EXIT_FAILURE);
    }

    // 인터페이스 인덱스 얻기
    ifindex = if_nametoindex("wlan0");
    if (ifindex == 0) {
        perror("Failed to get interface index");
        exit(EXIT_FAILURE);
    }

    // 메시지 생성
    msg = nlmsg_alloc();
    if (!msg) {
        perror("Failed to allocate netlink message");
        exit(EXIT_FAILURE);
    }

    // nl80211 명령 전송 (인터페이스 속성 요청)
    genlmsg_put(msg, 0, 0, genl_ctrl_resolve(sk, "nl80211"), 0, 0, NL80211_CMD_GET_INTERFACE, 0);
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifindex);

    // 명령 전송
    ret = nl_send_auto(sk, msg);
    if (ret < 0) {
        perror("Failed to send netlink message");
        exit(EXIT_FAILURE);
    }

    // 응답 처리
    struct nl_cb *cb = nl_cb_alloc(NL_CB_DEFAULT);
    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, parse_response, NULL);
    ret = nl_recvmsgs(sk, cb);
    if (ret < 0) {
        perror("Failed to receive netlink message");
        exit(EXIT_FAILURE);
    }

    // 후속 처리 후 콜백 객체 해제
    nl_cb_put(cb);
    // 소켓 종료
    nl_socket_free(sk);
}

// Wi-Fi 품질 측정 함수
double check_wifi_quality() {
    FILE *fp;
    char buffer[256];
    int link_quality = 0, max_quality = 70; // 기본적으로 Wi-Fi 품질은 0~70 사이 값으로 표현됨
    double quality_percent;

    // /proc/net/wireless 파일 열기
    fp = fopen("/proc/net/wireless", "r");
    if (fp == NULL) {
        perror("Failed to open /proc/net/wireless");
        exit(EXIT_FAILURE);
    }

    // 파일에서 각 라인 읽기
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // 인터페이스 이름을 찾기 위해 확인
        if (strstr(buffer, INTERFACE)) {
            // 품질 정보 추출
            sscanf(buffer, "%*s %*d %d", &link_quality);
            break;
        }
    }

    // Wi-Fi 품질 백분율 계산
    quality_percent = ((float)link_quality / max_quality) * 100.0;

    printf("=================================== wifi link qulity ===================================\n");

    // Wi-Fi 품질 출력
    printf("Wi-Fi 품질:\n");
    printf("링크 품질: %d/%d (%.2f%%)\n", link_quality, max_quality, quality_percent);

    // 품질이 낮을 경우 경고 출력
    if (quality_percent < 50.0) {
        printf("경고: Wi-Fi 품질이 낮습니다! (%.2f%%)\n", quality_percent);
    }

    // 파일 닫기
    fclose(fp);

    return quality_percent;
}


// run_iperf3_client 함수는 JSON 출력을 iperf3.json 파일로 저장하도록 수정
const char* run_iperf3_client(const char *__server_ip) {
    struct iperf_test *test;

    // iperf3 결과를 iperf3.json 파일로 저장
    FILE *json_file = fopen("iperf3.json", "w");
    if (json_file == NULL) {
        perror("Failed to open iperf3.json file");
        //return NULL;
        exit(EXIT_FAILURE);
    }

    freopen("iperf3.json", "w", stdout);  // 표준 출력 리디렉션

    test = iperf_new_test();
    if (test == NULL) {
        fprintf(stderr, "iperf3 test initialization failed\n");
        fclose(json_file);
        //return NULL;
        exit(EXIT_FAILURE);
    }

    iperf_defaults(test);
    iperf_set_test_role(test, 'c');
    iperf_set_test_server_hostname(test, __server_ip);
    iperf_set_test_server_port(test, 5201);
    iperf_set_test_duration(test, 5);
    iperf_set_test_json_output(test, 1);

    // iperf3 결과를 json_file에 출력
    if (iperf_run_client(test) < 0) {
        fprintf(stderr, "Error: %s\n", iperf_strerror(i_errno));
        iperf_free_test(test);
        fclose(json_file);
        //return NULL;
        exit(EXIT_FAILURE);
    }

    const char *json_output = iperf_get_test_json_output_string(test);
    if (json_output == NULL) {
        printf("iperf3 did not return JSON output\n");
        iperf_free_test(test);
        fclose(json_file);
        //return NULL;
        exit(EXIT_FAILURE);
    }

    fprintf(json_file, "%s", json_output);  // JSON 내용을 iperf3.json 파일에 저장
    fclose(json_file);

    iperf_free_test(test);
    return json_output;
}

// get_rx_tx_bandwidth는 iperf3.json 파일을 읽어서 처리하도록 수정
Bandwidth get_rx_tx_bandwidth(const char *(*iperf3_func)(const char *), const char *server_ip) {
    Bandwidth bandwidth;
    char *json_stream;
    //while(1) {
        const char *json_output = iperf3_func(server_ip);
        if (json_output == NULL) {
            printf("Failed to get JSON output.\n");
            //return;
            exit(EXIT_FAILURE);
        }

        // 다시 표준 출력을 화면으로 리디렉션
        freopen("/dev/tty", "w", stdout);

        // iperf3.json 파일 읽기
        FILE *json_file = fopen("iperf3.json", "r");
        if (json_file == NULL) {
            printf("Failed to open iperf3.json for reading\n");
            //return;
            exit(EXIT_FAILURE);
        }

        // 파일 내용 읽기
        fseek(json_file, 0, SEEK_END);
        long file_size = ftell(json_file);
        fseek(json_file, 0, SEEK_SET);

        char *file_contents = (char *)malloc(file_size + 1);
        if (file_contents == NULL) {
            printf("Memory allocation failed\n");
            fclose(json_file);
            //return;
            exit(EXIT_FAILURE);
        }
        json_stream = (char *)malloc(file_size + 1);
        if (json_stream == NULL) {
            printf("Memory allocation failed\n");
            fclose(json_file);
            //return;
            exit(EXIT_FAILURE);
        }
        json_stream = file_contents;

        fread(file_contents, 1, file_size, json_file);
        file_contents[file_size] = '\0';  // 문자열 종료 처리

        fclose(json_file);

        // JSON 파싱
        struct json_object *parsed_json = json_tokener_parse(file_contents);
        if (parsed_json == NULL) {
            printf("Failed to parse JSON\n");
            free(file_contents);
            //return;
            exit(EXIT_FAILURE);
        }

    printf("=================================== Wi-Fi Bandwidth ===================================\n");

    struct json_object *end, *sum_sent, *sum_received;
    if (json_object_object_get_ex(parsed_json, "end", &end)) {
        if (json_object_object_get_ex(end, "sum_sent", &sum_sent)) {
            struct json_object *sent_bitrate;
            if (json_object_object_get_ex(sum_sent, "bits_per_second", &sent_bitrate)) {
                bandwidth.tx_bitrate_bps = json_object_get_double(sent_bitrate);
                bandwidth.tx_bitrate_gbps = bandwidth.tx_bitrate_bps / 1000000000;  // Gbps로 변환
                bandwidth.tx_bitrate_mbps = bandwidth.tx_bitrate_bps / 1000000;     // Mbps로 변환
                printf("Sender Bitrate: %.2lf Mbps\n", bandwidth.tx_bitrate_mbps);
            }
        }

        if (json_object_object_get_ex(end, "sum_received", &sum_received)) {
            struct json_object *received_bitrate;
            if (json_object_object_get_ex(sum_received, "bits_per_second", &received_bitrate)) {
                bandwidth.rx_bitrate_bps = json_object_get_double(received_bitrate);
                bandwidth.rx_bitrate_gbps = bandwidth.rx_bitrate_bps / 1000000000;  // Gbps로 변환
                bandwidth.rx_bitrate_mbps = bandwidth.rx_bitrate_bps / 1000000;     // Mbps로 변환
                printf("Receiver Bitrate: %.2lf Mbps\n", bandwidth.rx_bitrate_mbps);
            }
        }
    } else {
        printf("No 'end' object found in JSON\n");
    }

    free(json_stream);
    return bandwidth;
}

// Wi-Fi 송수신 속도 측정 함수
Traffic get_rx_tx_traffic() {
    Traffic traffic;
    FILE *fp;
    char buffer[256];
    static unsigned long rx_bytes[AVG_WINDOW] = {0}, tx_bytes[AVG_WINDOW] = {0};
    static unsigned long prev_rx_bytes = 0, prev_tx_bytes = 0;
    static int count = 0;
    unsigned long rx_sum = 0, tx_sum = 0;
    double rx_speed, tx_speed, avg_rx_speed, avg_tx_speed;

    // /proc/net/dev 파일 열기
    fp = fopen("/proc/net/dev", "r");
    if (fp == NULL) {
        perror("Failed to open /proc/net/dev");
        exit(EXIT_FAILURE);
    }

    // 파일에서 각 라인 읽기
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // 인터페이스 이름을 찾기 위해 확인
        if (strstr(buffer, INTERFACE)) {
            unsigned long rx_bytes_current, tx_bytes_current;
            sscanf(buffer, "%*s %lu %*u %*u %*u %*u %*u %*u %*u %lu",
                   &rx_bytes_current, &tx_bytes_current);

            // 초당 전송 속도 계산
            rx_speed = (rx_bytes_current - prev_rx_bytes);
            tx_speed = (tx_bytes_current - prev_tx_bytes);

            // 이전 바이트 수 업데이트
            prev_rx_bytes = rx_bytes_current;
            prev_tx_bytes = tx_bytes_current;

            // 송수신 속도 배열에 저장
            rx_bytes[count % AVG_WINDOW] = rx_speed;
            tx_bytes[count % AVG_WINDOW] = tx_speed;

            // 최근 AVG_WINDOW 초 동안의 평균 계산
            rx_sum = 0;
            tx_sum = 0;
            
            for (int i = 0; i < AVG_WINDOW; i++) {
                rx_sum += rx_bytes[i];
                tx_sum += tx_bytes[i];
            }
            avg_rx_speed = rx_sum / AVG_WINDOW;
            avg_tx_speed = tx_sum / AVG_WINDOW;


            // bps
            traffic.rx_bitrate_bps = avg_rx_speed * 8;
            traffic.tx_bitrate_bps = avg_tx_speed * 8;

            traffic.rx_bitrate_mbps = avg_rx_speed * 8 / 1e6;
            traffic.tx_bitrate_mbps = avg_tx_speed * 8 / 1e6;

            traffic.rx_bitrate_gbps = avg_rx_speed * 8 / 1e9;
            traffic.tx_bitrate_gbps = avg_tx_speed * 8 / 1e9;

            // 속도 출력
            printf("=================================== Wi-Fi Bitrate ===================================\n");
            // printf("송신: %.2f KB/s (%.2f Mbps)\n", tx_speed / 1024, tx_speed * 8 / 1e6);
            // printf("수신: %.2f KB/s (%.2f Mbps)\n", rx_speed / 1024, rx_speed * 8 / 1e6);
            printf("송신: %.2f KB/s (%.2f Mbps)\n", tx_speed / 1024, tx_speed * 8 / 1e6);
            printf("수신: %.2f KB/s (%.2f Mbps)\n", rx_speed / 1024, rx_speed * 8 / 1e6);
            printf("최근 %d초 평균 송신 데이터 수: %.2f KB/s (%.2f Mbps)\n", AVG_WINDOW, avg_tx_speed / 1024, avg_tx_speed * 8 / 1e6);
            printf("최근 %d초 평균 수신 데이터 수: %.2f KB/s (%.2f Mbps)\n", AVG_WINDOW, avg_rx_speed / 1024, avg_rx_speed * 8 / 1e6);

            // 경고 출력
            if (rx_speed < avg_rx_speed * 0.7 || tx_speed < avg_tx_speed * 0.7) {
                printf("경고: 송수신 데이터 수가 평균보다 30%% 이상 감소했습니다! TX: %.2f B/s, RX: %.2f B/s\n", tx_speed, rx_speed);
            }

            count++;
            break;
        }
    }

    // 파일 닫기
    fclose(fp);
    return traffic;
}

void check_current_traffic_load() {
    Bandwidth bandwidth;
    Traffic traffic;

    char *server_ip = get_eth0_ip();
    if (server_ip == NULL) {
        printf("Failed to retrieve IP address for eth0\n");
        return;
    }
    printf("Server IP: %s\n", server_ip);  // IP 주소 출력

    while(1) {
        // get bandwidth
        bandwidth = get_rx_tx_bandwidth(run_iperf3_client, server_ip);

        // get traffic
        traffic = get_rx_tx_traffic();

        printf("=================================== Wi-Fi Traffics ===================================\n");
        printf("rx traffics: %.2lf %%\n", (traffic.rx_bitrate_mbps/bandwidth.rx_bitrate_mbps)*100);
        printf("tx traffics: %.2lf %%\n", (traffic.tx_bitrate_mbps/bandwidth.tx_bitrate_mbps)*100);

        // for debug
        // printf("rx bandwidth: %.2lf %%\n", bandwidth.rx_bitrate_mbps);
        // printf("tx bandwidth: %.2lf %%\n", bandwidth.tx_bitrate_mbps);
        // printf("rx traffics: %.2lf %%\n", traffic.rx_bitrate_mbps);
        // printf("tx traffics: %.2lf %%\n", traffic.tx_bitrate_mbps);

        check_wifi_frequency();
        check_wifi_quality();
        

        sleep(30);
    }
}
