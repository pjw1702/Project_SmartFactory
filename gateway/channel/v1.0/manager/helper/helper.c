#include "helper.h"

// 직렬화 함수
size_t serialize(rct_instance *__src_rct_pdu, uint8_t *__dst_buffer) {
    size_t size = sizeof(Rcphdr) + sizeof(Rcdata);
    if (size > sizeof(PDU)) {
        fprintf(stderr, "Error: Buffer overflow in serialization.\n");
        exit(1);
    }
    memcpy(__dst_buffer, &__src_rct_pdu->rcp_header, sizeof(Rcphdr));
    memcpy(__dst_buffer + sizeof(Rcphdr), &__src_rct_pdu->rc_data, sizeof(Rcdata));

    return size;
}

// 역직렬화 함수
void deserialize(const uint8_t *__src_buffer, rct_instance *__dst_rct_pdu) {
    memcpy(&__dst_rct_pdu->rcp_header, __src_buffer, sizeof(Rcphdr));
    memcpy(&__dst_rct_pdu->rc_data, __src_buffer + sizeof(Rcphdr), sizeof(Rcdata));
}

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

// 준비 완료 확인 메시지 전송 함수
void send_confirm_ready_message(int __worker_fd) {
    rct_instance ready_confirm_msg;
    uint8_t buffer[BUF_SIZE];
    memset(&ready_confirm_msg, 0, sizeof(rct_instance));

    // 준비 완료 메시지 작성
    strncpy(ready_confirm_msg.rcp_header.proto_ver, "rcp/1.1", sizeof(ready_confirm_msg.rcp_header.proto_ver));
    strncpy(ready_confirm_msg.rcp_header.msg_type, MSG_TYPE_CONFIRM, sizeof(ready_confirm_msg.rcp_header.msg_type));
    ready_confirm_msg.rcp_header.timestamp = time(NULL);

    // 직렬화
    size_t serialized_size = serialize(&ready_confirm_msg, buffer);

    // 메시지 전송
    if (write(__worker_fd, buffer, serialized_size) == -1) {
        perror("write failed");
        return;
    }

    printf("Sent response to worker: System is ready.\n");
    printf("Response code: 1 (confirm)\n");
}

void send_command(int __robot_fd, uint8_t __detect_flag, uint8_t __color) {
    //PDU 구조체 생성 및 초기화
    PDU pdu = {0};
    
    // 헤더 설정
    strncpy(pdu.rcp_header.proto_ver, "rcp/1.1", sizeof(pdu.rcp_header.proto_ver));
    strncpy(pdu.rcp_header.msg_type, "COMMAND", sizeof(pdu.rcp_header.msg_type));
    pdu.rcp_header.err = 0;
    pdu.rcp_header.p_size = sizeof(PDU);
    pdu.rcp_header.check = 0; // 체크섬은 이후 설정
    pdu.rcp_header.timestamp = (uint64_t)time(NULL);
    pdu.rcp_header.d_len = sizeof(Rcdata);

    // Rcdata 설정
    pdu.rc_data.yaw = 1.0;    // 예제 값
    pdu.rc_data.pitch = 2.0;  // 예제 값
    pdu.rc_data.roll = 3.0;   // 예제 값
    pdu.rc_data.x_pos = 4.0;  // 예제 값
    pdu.rc_data.y_pos = 5.0;  // 예제 값
    pdu.rc_data.z_pos = 6.0;  // 예제 값
    
    pdu.rc_data.robot_id = ROBOT_ID_01;
    pdu.rc_data.conveyer_id = CONVEYER_ID_01;
    pdu.rc_data.detect_flag = __detect_flag;
    pdu.rc_data.color = __color;
    snprintf(pdu.rc_data.data, sizeof(pdu.rc_data.data), "ControlMsg");

    // 체크섬 계산 및 설정
    pdu.rcp_header.check = calculate_checksum(&pdu, sizeof(rct_instance));

    // 데이터 전송
    if (write(__robot_fd, &pdu, sizeof(PDU)) == -1) {
        perror("Write to worker of robot failed");
    } else {
        printf("Sent PDU to worker of robot \n");
    }
}

void error_handling(char *__msg) {
      fputs(__msg, stderr);
      fputc('\n', stderr);
      exit(1);
}

void log_file(char * __msg_contents) {
      fputs(__msg_contents, stdout);
}