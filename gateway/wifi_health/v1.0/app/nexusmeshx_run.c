#include "nexusmeshx_app.h"

int main(int argc, char *argv[]) {
    Wifi *wifi_info;
    Netlink nl_info;

	// wifi 정보 메모리 할당
    wifi_info = alloc_wifi_info_space();

    // wifi 정보 제어
	nl_info.id = init_wifi_attr(&nl_info, wifi_info);
	if (nl_info.id < 0) {
		fprintf(stderr, "Error initializing netlink 802.11\n");
		return -1;
	}

    // 앱 실행
    int dev = run_nexusmeshx_app(wifi_info, nl_info, argc, argv);

	// Netlink 리소스 반환
	printf("\nExiting gracefully... ");
	nl_cb_put(nl_info.cb_func_wifi_if);
	nl_cb_put(nl_info.cb_func_wifi_sta);
	nl_close(nl_info.sock);
	nl_socket_free(nl_info.sock);

	// wifi 정보 메모리 반환
	free_wifi_info_space(wifi_info);

    // 앱 리소스 반환
    destroy_nexusmesh_app(dev);

    return 0;
}