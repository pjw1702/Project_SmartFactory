#ifndef __RCP_H__
#define __RCP_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <mqueue.h>
#include <stdint.h>
#include <time.h>

#include "model/nmx_channel_data.h"

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
#define RESPONSE_CODE_WORKER_AUTH_SUCCESS               230
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
#define RESPONSE_CODE_AUTH_FAIL         510

/* Command */
#define COMMAND_GREP        "RCT GREP"
#define COMMAND_RELEASE     "RCT RELEASE"

/* 크기 정보 */
#define SIZE_RCPHDR  sizeof(Rcphdr)
#define SIZE_RCDATA  sizeof(Rcdata)
#define STRING_HASH_LENGTH  sizeof(unsigned int)
#define SHA_DIGEST_LENGTH 32
#define DATA_SIZE 1024
#define EPOLL_SIZE 50
#define MANAGER_PORT 8080
#define MAX_PAYLOAD_SIZE 1024
#define MAX_EVENTS  50

#define MAX_CLNT 32      
#define ID_SIZE 10     
#define ARR_CNT 5     

#define BUF_SIZE 1024
#define PORT 8080
#define THREAD_POOL_SIZE 4
#define MAX_TASKS 10
#define WORKER_MAX_CLNT 32
#define WORKER_ID_SIZE 30

#define MAX_BUF_SIZE 256
#define RCP_PROTO_VER_SIZE  8
#define RCP_MESSAGE_TYPE_SIZE   16
#define RCP_DATA_SIZE   20

#define MAX_PAYLOAD_SIZE 1024
#define MAX_MSG_ID 4294967295  // uint32_t의 최대값

/* 토픽 정보 */
#define TOPIC_NAME "/nmx_channel_01"

/* 스레드 ID */
#define CONVEYER_ID_01  1
#define ROBOT_ID_01     1

/* 스레드 타입 */
#define MACHINE_TYPE_UNKNOWN         0
#define MACHINE_TYPE_CONVEYER        1
#define MACHINE_TYPE_ROBOT           2

/* Status */
#define READY                   1
#define RECEIVE                 2
#define TRANSMIT                3
#define IDLE                    4
#define PROCESSING              5
#define WAITING_FOR_RESPONSE    6
#define ERROR                   7

/* 탐지 여부 플래그 */
#define FALSE   0
#define TRUE    1

/* 색깔 데이터 */
#define RED     1
#define GREEN   2
#define BLUE    3

/* 반환 값 */
#define FALSE   0
#define TRUE    1
#define FAIL        -1
#define SUCCESS     1

#pragma pack(push, 1)

/* 메시지 정보 */
typedef struct {
      char fd;
      char *from;
      char *to;
      char *msg;
      int len;
} MSG_INFO;

/* 워커의 디바이스 정보 */
typedef struct {
      int index;
      int fd;
      char ip[20];
      char id[ID_SIZE];
      char pw[ID_SIZE];
} DEVICE_INFO;

/* 헤더 */
typedef struct {
    char proto_ver[8];
    char msg_type[16];
    uint16_t err;
    uint16_t p_size;
    uint16_t check;
    uint64_t timestamp;
    uint32_t d_len;
} Rcphdr;

/* 페이로드 */
typedef struct _robot_control_data {
    float yaw;
    float pitch;
    float roll;
    float x_pos;
    float y_pos; 
    float z_pos; 
    uint8_t robot_id;
    uint8_t conveyer_id;
    uint8_t detect_flag;
    uint8_t color;
    char data[20];
} Rcdata;

/* PDU */
typedef struct _pdu {
    Rcphdr rcp_header;
    Rcdata rc_data;
} PDU;

typedef PDU rct_instance;

/*워커로 부터 송신 및 수신하는 메타 데이터 */
typedef struct {
    short worker_fd;
    uint8_t machine_type;
    uint8_t detect_flag;
    uint8_t color;
} worker_task_t;

/* 각 워커에 대한 스레드의 파일 디스크립터 배열 */
typedef struct fd_list_t {
    int conveyer_fd[THREAD_POOL_SIZE];
    int robot_fd[THREAD_POOL_SIZE];
} FDList;

#pragma pack(pop)

size_t serialize(rct_instance *__src_rct_pdu, uint8_t *__dst_buffer);
void deserialize(const uint8_t *__src_buffer, rct_instance *__dst_rct_pdu);
uint16_t calculate_checksum(void *data, size_t size);
void send_confirm_ready_message(int __worker_fd);
void send_command(int __robot_fd, uint8_t __detect_flag, uint8_t __color);
void error_handling(char *__msg);
void log_file(char * __msg_contents);

void init_device_info();
uint8_t auth_worker_connect(struct sockaddr_in __worker_adr, int __worker_sock, char __device_info[]);
void *conveyer_handler(void *__arg);
void *robot_handler(void *__arg);
void* rct_brocker_handler(void *__arg);


#endif