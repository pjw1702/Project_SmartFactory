#include "/home/ubuntu/nexusmeshx/v1.1/utils/wifi/wifi_check.h"

static volatile int keep_running = 1;

void ctrl_c_handler(int __dummy) {
	keep_running = 0;
}


int main() {
    //check_current_traffic_load();

	Netlink nl_info;
	Wifi *wifi_info;

	signal(SIGINT, ctrl_c_handler);

	// wifi 정보 메모리 할당
    wifi_info = alloc_wifi_info_space();

    // wifi 정보 제어
	nl_info.id = init_wifi_attr(&nl_info, wifi_info);
	if (nl_info.id < 0) {
		fprintf(stderr, "Error initializing netlink 802.11\n");
		return -1;
	}
    
	while (keep_running) {
		get_wifi_status(&nl_info, wifi_info);
		printf("Interface: %s | signal: %d dB | tx bandwith: %.2f MBit/s  | rx bandwidth: %.2f MBit/s | tx packets: %ld p/s| rx packets: %ld p/s\n",
		       wifi_info->interface->if_name, 
               wifi_info->connection->signal, 
               wifi_info->bandwidth->tx_bitrate_mbps, wifi_info->bandwidth->rx_bitrate_mbps, 
               wifi_info->packets->tx_packets, wifi_info->packets->rx_packets);
        sleep(1);
	}

	// Netlink 리소스 반환
	printf("\nExiting gracefully... ");
	nl_cb_put(nl_info.cb_func_wifi_if);
	nl_cb_put(nl_info.cb_func_wifi_sta);
	nl_close(nl_info.sock);
	nl_socket_free(nl_info.sock);

	// wifi 정보 메모리 반환
	free_wifi_info_space(wifi_info);

	printf("OK\n");

    return 0;
}