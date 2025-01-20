/*
 * rcp.h
 *
 *  Created on: Dec 21, 2024
 *      Author: iot
 */

#ifndef INC_RCP_H_
#define INC_RCP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "stm32f4xx_hal.h"

// BUF_SIZE와 PORT는 서버와 동일하게 설정
#define BUF_SIZE 1024
#define PORT 8080

#define NO_DETECT	0
#define DETECT		1

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
#define MAX_PAYLOAD_SIZE 1024
#define MAX_MSG_ID 4294967295  // uint32_t의 최대값

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

#pragma pack(push, 1)

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

#pragma pack(pop)

uint16_t calculate_checksum(void *data, size_t size);
int verify_checksum(RCT_Instance *rct_data);
void test_rcp(RCT_Instance *rct_data);
void deserialize_rct_data(uint8_t *__src_buffer, RCT_Instance *__dst_rct_pdu);
size_t serialize_rct_data(RCT_Instance *__dst_rct_pdu, char *__dst_buffer);
void rct_send_confirm_ready_message(char* __sendBuf);

#endif /* INC_RCP_H_ */
