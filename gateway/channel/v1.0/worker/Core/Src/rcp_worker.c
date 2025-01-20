#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "stm32f4xx_hal.h"
#include "rcp.h"
#include "esp.h"

uint16_t checksum;

// 체크섬 계산 함수
uint16_t calculate_checksum(void *data, size_t size) {
    uint16_t *ptr = (uint16_t *)data;
    uint32_t sum = 0;

    // 16비트 단위로 합산
    for (size_t i = 0; i < size / 2; i++) {
        sum += *ptr++;
    }

    // 홀수 길이일 경우 마지막 바이트를 추가
    if (size % 2 != 0) {
        sum += *((uint8_t *)ptr);
    }

    // 16비트 오버플로우 처리
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum; // 1의 보수 반환
}

// 체크섬 검증 함수
int verify_checksum(RCT_Instance *rct_data) {
    uint16_t received_check = rct_data->rcp_header.check;

    // 확인용
    checksum = rct_data->rcp_header.check;

    // 체크섬을 0으로 설정하여 재계산
    rct_data->rcp_header.check = 0;

    // 계산된 체크섬과 받은 체크섬 비교
    if (calculate_checksum(rct_data, sizeof(RCT_Instance)) == received_check) {
        return 1; // 체크섬이 일치함
    }

    return 0; // 체크섬이 일치하지 않음
}

void test_rcp(RCT_Instance *rct_data) {
    // 받은 PDU 체크섬 검증
    if (verify_checksum(rct_data)) {
        // 체크섬이 유효할 때만 데이터 출력
        printf("Received from server:\n");
        printf("Proto Ver: %s\n", rct_data->rcp_header.proto_ver);
        printf("Msg Type: %s\n", rct_data->rcp_header.msg_type);
        printf("Error: %d\n", rct_data->rcp_header.err);
        printf("Payload Size: %d\n", rct_data->rcp_header.p_size);
        printf("Check: %d\n", checksum);
        printf("Timestamp: %llu\n", rct_data->rcp_header.timestamp);
        printf("Data Length: %ld\n", rct_data->rcp_header.d_len);
        printf("Color: %d\n", rct_data->rc_data.color);
        printf("Data: %s\n", rct_data->rc_data.data);

        // 좌표와 회전 값 출력
        printf("Yaw: %.2f\n", rct_data->rc_data.yaw);
        printf("Pitch: %.2f\n", rct_data->rc_data.pitch);
        printf("Roll: %.2f\n", rct_data->rc_data.roll);
        printf("X Position: %.2f\n", rct_data->rc_data.x_pos);
        printf("Y Position: %.2f\n", rct_data->rc_data.y_pos);
        printf("Z Position: %.2f\n", rct_data->rc_data.z_pos);
        printf("Flag: %d", rct_data->rc_data.detect_flag);
        printf("Color: %d", rct_data->rc_data.color);
    } else {
        printf("Checksum validation failed. Data discarded.\n");
    }
}

void deserialize_rct_data(uint8_t *__src_buffer, RCT_Instance *__dst_rct_pdu) {
    memcpy(__dst_rct_pdu, __src_buffer, sizeof(RCT_Instance));  // 데이터를 RCT_Instance 구조체로 복사
//    memcpy(__dst_rct_pdu->rcp_header, __src_buffer, sizeof(Rcphdr));
//    memcpy(__dst_rct_pdu->rc_data, __src_buffer + sizeof(Rcphdr), sizeof(Rcdata));

    // 네트워크 바이트 순서 -> 호스트 바이트 순서로 변환 (필요시)
//    pdu->rcp_header.err = ntohs(rct_data->rcp_header.err);
//    pdu->rcp_header.p_size = ntohs(rct_data->rcp_header.p_size);
//    pdu->rcp_header.check = ntohs(rct_data->rcp_header.check);
//    pdu->rcp_header.d_len = ntohl(rct_data->rcp_header.d_len);
//    pdu->rcp_header.timestamp = ntohll(rct_data->rcp_header.timestamp);  // 64-bit 필드의 경우
}

size_t serialize_rct_data(RCT_Instance *__dst_rct_pdu, char *__dst_buffer) {
    size_t size = sizeof(Rcphdr) + sizeof(Rcdata);
    if (size > DATA_SIZE) {
        fprintf(stderr, "Error: Buffer overflow in serialization.\n");
        exit(1);
    }
    memcpy(__dst_buffer, &__dst_rct_pdu->rcp_header, sizeof(Rcphdr));
    memcpy(__dst_buffer + sizeof(Rcphdr), &__dst_rct_pdu->rc_data, sizeof(Rcdata));

    return size;
}

// 준비 완료 확인 메시지 전송 함수
void rct_send_confirm_ready_message(char* __sendBuf) {
	RCT_Instance ready_confirm_msg;
    memset(&ready_confirm_msg, 0, sizeof(RCT_Instance));

    // 준비 완료 메시지 작성
    strncpy(ready_confirm_msg.rcp_header.proto_ver, "rcp/1.1", sizeof(ready_confirm_msg.rcp_header.proto_ver));
    strncpy(ready_confirm_msg.rcp_header.msg_type, MSG_TYPE_READY, sizeof(ready_confirm_msg.rcp_header.msg_type));
    ready_confirm_msg.rcp_header.err = RESPONSE_CODE_READY;
    ready_confirm_msg.rcp_header.timestamp = 0;

    // 직렬화
    serialize_rct_data(&ready_confirm_msg, __sendBuf);

    // 메시지 전송
	esp_send_data(__sendBuf);

    printf("%d Sent response to manager: System is ready.\n", RESPONSE_CODE_WORKER_SUCCESS_TRANMIT);
    printf("%d Response code: ready\n", RESPONSE_CODE_READY);
}
