#ifndef _RCP__H
#define _RCP__H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
#include "hash/hash.h"
#include "helper/helper.h"

#define DEBUG

/* Message Type */
#define MSG_TYPE_READY "READY"
#define MSG_TYPE_CMD "COMMAND"
#define MSG_TYPE_DONE "DONE"
#define MSG_TYPE_CONFIRM "CONFIRM"
#define MSG_TYPE_REQUEST "REQUEST"
#define MSG_TYPE_ACK "ACK"

/* Respponse code */
// 2xx
#define RESPONSE_CODE_READY                             200
#define RESPONSE_CODE_EXEC_DONE                         201
#define RESPONSE_CODE_CONFIRM_READY                     202
#define RESPONSE_CODE_MANAGER_CONNECTED                 211
#define RESPONSE_CODE_WORKER_CONNECTED                  212
#define RESPONSE_CODE_COMMAND_COMPLETE                  240
#define RESPONSE_CODE_MANAGER_SUCCESS_TRANMIT           262
#define RESPONSE_CODE_MANAGER_SUCCESS_RECEIVE           263
#define RESPONSE_CODE_WORKER_SUCCESS_TRANMIT            262
#define RESPONSE_CODE_WORKER_SUCCESS_RECEIVE            263

// 4xx
#define RESPONSE_CODE_NOT_READY                         410
#define RESPONSE_CODE_MANAGER_DISCONNECTED              411
#define RESPONSE_CODE_WORKER_DISCONNECTED               412
#define RESPONSE_CODE_WORKER_COMMAND_SUSPENDED          413
#define RESPONSE_CODE_WORKER_COMMAND_NOT_PERPFORMED     414
#define RESPONSE_CODE_ERROR                             430
#define RESPONSE_CODE_MANAGER_NOT_RESPONSE              451
#define RESPONSE_CODE_WORKER_NOT_RESPONSE               452
#define RESPONSE_CODE_MANAGER_FAIL_TRANMIT              462
#define RESPONSE_CODE_MANAGER_FAIL_RECEIVE              463
#define RESPONSE_CODE_WORKER_FAIL_TRANMIT               462
#define RESPONSE_CODE_WORKER_FAIL_RECEIVE               463

// 5xx
#define RESPONSE_CODE_WRONG_COMMAND     500
#define RESPONSE_CODE_NO_COMMAND        501

#define SHA256_DIGEST_LENGTH    32
#define MAX_PAYLOAD_SIZE 1024
#define MAX_MSG_ID 4294967295  // uint32_t의 최대값

/* Command */
#define COMMAND_GREP        "RCT GREP"
#define COMMAND_RELEASE     "RCT RELEASE"

/* Size */
#define SIZE_RCPHDR  sizeof(Rcphdr)
#define SIZE_RCDATA  sizeof(Rcdata)
#define STRING_HASH_LENGTH  sizeof(unsigned int)
#define SHA_DIGEST_LENGTH 32
#define DATA_SIZE 1024
#define EPOLL_SIZE 50
#define SERVER_PORT 8080
#define MAX_PAYLOAD_SIZE 1024
#define MAX_EVENTS  50

#define BUF_SIZE 100   // msg 버퍼 크기
#define MAX_CLNT 32      // user 수
#define ID_SIZE 10      // ID의 길이
#define ARR_CNT 5      // ??

// 상위 계층에서 데이터 전송 요청
// 하위 계층이 Request에 대한 작업 완료 결과
// 하위 계층이 상위 계층으로 이벤트나 상태 변경을 알림
// 하위 계층에서 Request 작업이 완료되었음을 상위 계층에 알림

/* Status */
#define READY                   1
#define RECEIVE                 2
#define TRANSMIT                3
#define IDLE                    4
#define PROCESSING              5
#define WAITING_FOR_RESPONSE    6
#define ERROR                   7

/* Return */
#define FAIL        -1
#define SUCCESS     1

/* Signal of detect */
#define FALSE   0
#define TRUE    1

/* Data of color */
#define RED     1
#define GREEN   2
#define BLUE    3

//typedef unsigned short Type;

/* Remove padding for serialization of data */
#pragma pack(push, 1)

// 헤더 정의
typedef struct {
    char proto_ver[8];
    char msg_type[16];
    uint16_t err;
    uint16_t p_size;
    uint16_t check;
    //uint16_t secret_key;
    uint64_t timestamp;
    //uint32_t hash_value;
    uint32_t d_len;
    uint8_t hmac[32];  // HMAC 필드 추가
} Rcphdr;

/* Payload */
typedef struct _robot_control_data {
    // value of spin for z-axis
    float yaw;
    // value of spin for y-axis
    float pitch;
    // value of spin for x-axis
    float roll;
    // coordinate value for x-axis
    float x_pos;
    // coordinate value for y-axis
    float y_pos;
    // coordinate value for z-axis
    float z_pos;
    // signal of detect
    uint8_t detect_flag;
    // information of color
    uint8_t color;
    // instruction etc
    char data[20];
} Rcdata;

/* PDU */
typedef struct _pdu {
    Rcphdr rcp_header;
    Rcdata rc_data;
} PDU;

typedef PDU rct_instance;

typedef struct epoll_event Epoll_Event;
typedef struct sockaddr_in Socket_Addr;

typedef struct _channel {
    int serv_sock, new_clnt_sock;
    Socket_Addr serv_addr;
    Socket_Addr clnt_addr;
    socklen_t addr_len;
    Epoll_Event event;
    Epoll_Event *ep_events;
    int str_len, epfd, event_cnt;
    pthread_t listener_channel_id, controller_channel_id;
    char buf[DATA_SIZE];

} rct_channel;

typedef pthread_mutex_t mutex;

typedef struct _rct_shared_data {
    int detect_flag;
    int color;
    int new_clnt_sock;
    mutex lock;
    pthread_cond_t cond;
} rct_shared_data;

// information for user

// message
typedef struct {
      char fd;
      char *from;
      char *to;
      char *msg;
      int len;
}MSG_INFO;

// user
typedef struct {     
      int index;
      int fd;
      char ip[20];
      char id[ID_SIZE];
      char pw[ID_SIZE];
}CLIENT_INFO;

#pragma pack(pop)

// void error_handling(char *message);
// void set_mode_non_blocking(int __fd);
// void set_client_socket(int __client_fd);
// int get_client_socket();
// void set_rct_data(rct_instance __rct_data);
// rct_instance get_rct_data();
// void* channel_listener(void* __arg);
// void* channel_controller(void* __arg);
// size_t serialize(rct_instance *pdu, uint8_t *buffer);
// void deserialize(const uint8_t *buffer, rct_instance *pdu);
// void send_confirm_ready_message(int __client_fd);
// void send_command(int client_fd, rct_instance ___cmd);
// int rct_new_channel_create(char *__srv_port);
// int rct_channel_handler(int __serv_sock);
// void rct_channel_serve(int __epoll_fd);



#endif