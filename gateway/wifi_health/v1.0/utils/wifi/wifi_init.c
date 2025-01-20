#include "wifi_check.h"

static Wifi *wifi_info = NULL;

// wifi 정보 구조체를 초기화 하기 위한 함수
Wifi * alloc_wifi_info_space() {
    wifi_info = malloc(sizeof(Wifi));
    if (!wifi_info) {
        perror("Failed to allocate memory for wifi_info");
        exit(EXIT_FAILURE);
    }

    wifi_info->bandwidth = malloc(sizeof(Bandwidth));
    wifi_info->packets = malloc(sizeof(Packet));
    wifi_info->traffic = malloc(sizeof(Traffic));
    wifi_info->interface = malloc(sizeof(Interface));
    wifi_info->connection = malloc(sizeof(Connection));

    if (!wifi_info->bandwidth || !wifi_info->traffic || !wifi_info->interface || !wifi_info->connection || !wifi_info->packets) {
        perror("Failed to allocate memory for wifi_info members");
        free_wifi_info_space(wifi_info);
        exit(EXIT_FAILURE);
    }

    // 초기값 설정
    strcpy(wifi_info->interface->if_name, "unknown");
    wifi_info->interface->if_index = -1;

    wifi_info->bandwidth->rx_bitrate_mbps = 0.0;
    wifi_info->bandwidth->tx_bitrate_mbps = 0.0;

    wifi_info->packets->tx_packets = 0;
    wifi_info->packets->rx_packets = 0;

    wifi_info->traffic->rx_bitrate_bps = 0.0;
    wifi_info->traffic->tx_bitrate_bps = 0.0;

    wifi_info->connection->signal = 0;
    wifi_info->connection->quality = 0;
    wifi_info->connection->frequency = 0.0;

    return wifi_info;
}

// wifi 정보 구조체를 반환하기 위한 함수
void free_wifi_info_space(Wifi * __wifi_info) {
    if (!__wifi_info) return;

    if (__wifi_info->interface) {
        free(__wifi_info->interface);
    }
    free(__wifi_info->bandwidth);
    free(__wifi_info->packets);
    free(__wifi_info->traffic);
    free(__wifi_info->connection);
    free(__wifi_info);

    // 호출 함수에서 NULL을 설정하도록 해야 함
    __wifi_info = NULL;
}

// Netlink 메시지 송수신 처리를 수행하는 콜백 함수를 초기화 하기 위한 함수
int init_wifi_attr(Netlink *__nl_info, Wifi *__wifi_info) {
	// Netlink 소켓 할당
    __nl_info->sock = nl_socket_alloc();
	if (!__nl_info->sock) {
		perror("Failed to allocate netlink socket.\n");

		return -ENOMEM;
	}

    // Netlink 소켓의 송신 및 수신 버퍼를 설정하기 위한 함수
    // 버퍼 크기는 8192바이트로 지정
	nl_socket_set_buffer_size(__nl_info->sock, NETLINK_SOCKET_BUF_SIZE, NETLINK_SOCKET_BUF_SIZE);

    // 사용자 공간의 Netlink 소켓을 커널 공간의 cfg80211 모듈과 연결하기 위한 함수
	if (genl_connect(__nl_info->sock)) {
		perror("Failed to connect to netlink socket.\n");
		nl_close(__nl_info->sock);
		nl_socket_free(__nl_info->sock);

		return -ENOLINK;
	}

    // Netlink 소켓을 통해 nl80211 인터페이스의 ID를 추출하기 위한 함수
	__nl_info->id = genl_ctrl_resolve(__nl_info->sock, "nl80211");
	if (__nl_info->id < 0) {
		perror("Nl80211 interface not found.\n");
		nl_close(__nl_info->sock);
		nl_socket_free(__nl_info->sock);

		return -ENOENT;
	}

    // WiFi 인터페이스와 스테이션 정보에 대한 송수신 처리를 수행하는 Netlink 콜백 함수가 위치한 메모리 공간을 할당받기 위한 함수
	__nl_info->cb_func_wifi_if = nl_cb_alloc(NL_CB_DEFAULT);
	__nl_info->cb_func_wifi_sta = nl_cb_alloc(NL_CB_DEFAULT);
	if ((!__nl_info->cb_func_wifi_if) || (!__nl_info->cb_func_wifi_sta)) {
		perror("Failed to allocate netlink callback.\n");
		nl_close(__nl_info->sock);
		nl_socket_free(__nl_info->sock);

		return ENOMEM;
	}

    /* Netlink 콜백 함수 등록
      * NL_CB_VALID: 유효성을 검증하는 콜백 함수를 의미하는 매크로
      * NL_CB_FINISH: 작업 완료 상태를 처리하는 콜백 함수를 의미하는 매크로
      * NL_CB_CUSTOM: 사용자 정의 콜백 함수를 의미하는 매크로
     */

    // Netlink 콜백 함수를 각각의 역할을 나누어 갖고 등록하기 위한 함수
	nl_cb_set(__nl_info->cb_func_wifi_if, NL_CB_VALID, NL_CB_CUSTOM, get_wifi_if_name, __wifi_info);
	nl_cb_set(__nl_info->cb_func_wifi_if, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &(__nl_info->res_wifi_if));
	nl_cb_set(__nl_info->cb_func_wifi_sta, NL_CB_VALID, NL_CB_CUSTOM, get_wifi_info, __wifi_info);
	nl_cb_set(__nl_info->cb_func_wifi_sta, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &(__nl_info->res_wifi_sta));

    // 이후 Netlink 통신에서 특정 명령어를 요청하기 위해, 인터페이스 ID를 반환
	return __nl_info->id;
}