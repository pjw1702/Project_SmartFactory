#ifndef __WIFI_CHECK_H__
#define __WIFI_CHECK_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <iperf_api.h>
#include <json-c/json.h>
#include <unistd.h>
#include <net/if.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <linux/nl80211.h>

#define INTERFACE "wlan0"  
#define AVG_WINDOW 30 

typedef struct __bandwidth_info {
    double rx_bitrate_bps;
    double tx_bitrate_bps;
    double rx_bitrate_mbps;
    double tx_bitrate_mbps;
    double rx_bitrate_gbps;
    double tx_bitrate_gbps;
} __attribute__((packed)) Bandwidth;

typedef struct __traffic_info {
    double rx_bitrate_bps;
    double tx_bitrate_bps;
    double rx_bitrate_mbps;
    double tx_bitrate_mbps;
    double rx_bitrate_gbps;
    double tx_bitrate_gbps;
} __attribute__((packed)) Traffic;

char* get_eth0_ip();
int parse_response(struct nl_msg *msg, void *arg);
void check_wifi_frequency();
double check_wifi_quality();
const char* run_iperf3_client(const char *__server_ip);
Bandwidth get_rx_tx_bandwidth(const char *(*iperf3_func)(const char *), const char *server_ip);
Traffic get_rx_tx_traffic();
void check_current_traffic_load();


#endif
