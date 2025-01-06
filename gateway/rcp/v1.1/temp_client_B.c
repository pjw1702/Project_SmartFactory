/* verify checksum */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <time.h>

// BUF_SIZE와 PORT는 서버와 동일하게 설정
#define BUF_SIZE 1024
#define PORT 8080

#define FAIL        0
#define SUCCESS     1

#define MSG_TYPE_CONFIRM    "CONFIRM"
#define MSG_TYPE_READY      "READY"

typedef uint8_t Bool;

uint16_t checksum;

// rcp 헤더 정의
typedef struct {
    char proto_ver[8];
    char msg_type[16];
    uint16_t err;
    uint16_t p_size;
    uint16_t check;
    uint64_t timestamp;
    uint32_t d_len;
    uint8_t hmac[32];  // HMAC 필드 추가
} __attribute__((packed)) Rcphdr;

/* Payload */
typedef struct _robot_control_data {
    float yaw;   // Spin value for z-axis
    float pitch; // Spin value for y-axis
    float roll;  // Spin value for x-axis
    float x_pos; // Coordinate for x-axis
    float y_pos; // Coordinate for y-axis
    float z_pos; // Coordinate for z-axis
    uint8_t robot_id;
    uint8_t conveyer_id;
    uint8_t detect_flag;
    uint8_t color;
    char data[20]; // Instructions
} __attribute__((packed)) Rcdata;

/* PDU */
typedef struct _pdu {
    Rcphdr rcp_header;
    Rcdata rc_data;
} __attribute__((packed)) PDU;

typedef PDU rct_instance;

// 역직렬화 함수
void deserialize(const uint8_t *buffer, rct_instance *pdu) {
    memcpy(&pdu->rcp_header, buffer, sizeof(Rcphdr));
    memcpy(&pdu->rc_data, buffer + sizeof(Rcphdr), sizeof(Rcdata));
}

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

Bool try_login(int sockfd) {
    char id[50], password[50];  // 로그인에 사용할 변수
    char buffer[BUF_SIZE];

    // 로그인 절차
    while (1) {
        // 사용자로부터 아이디와 비밀번호 입력 받음
        printf("Enter username: ");
        scanf("%s", id);
        printf("Enter password: ");
        scanf("%s", password);

        // 로그인 정보를 서버에 전송
        snprintf(buffer, BUF_SIZE, "[%s:%s]", id, password);
        if (write(sockfd, buffer, strlen(buffer)) == -1) {
            perror("Write failed");
            close(sockfd);
            //exit(EXIT_FAILURE);
            continue;
        }

        // 서버에서 인증 응답 받기
        ssize_t nbytes = read(sockfd, buffer, BUF_SIZE - 1);
        if (nbytes <= 0) {
            perror("Read error or connection closed");
            close(sockfd);
            //exit(EXIT_FAILURE);
            continue;
        }

        buffer[nbytes] = '\0';  // 응답 문자열 종료 처리
        printf("Server response: %s\n", buffer);

        // 인증 성공이면 데이터 전송 시작
        if (strncmp(buffer, "Authentication successful", 24) == 0) {
            break;  // 인증 성공시 로그인 완료, 데이터 전송으로 넘어감
        } else {
            printf("Authentication failed, please try again.\n");
            continue;
        }
    }

    return SUCCESS;
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

// 체크섬 검증 함수
int verify_checksum(PDU *pdu) {
    uint16_t received_check = pdu->rcp_header.check;

    // 확인용
    checksum = pdu->rcp_header.check;
    
    // 체크섬을 0으로 설정하여 재계산
    pdu->rcp_header.check = 0;
    
    // 계산된 체크섬과 받은 체크섬 비교
    if (calculate_checksum(pdu, sizeof(rct_instance)) == received_check) {
        return 1; // 체크섬이 일치함
    }
    
    return 0; // 체크섬이 일치하지 않음
}

// 준비 완료 확인 메시지 전송 함수
void send_confirm_ready_message(int __client_fd) {
    rct_instance ready_confirm_msg;
    uint8_t buffer[BUF_SIZE];
    memset(&ready_confirm_msg, 0, sizeof(rct_instance));

    // 준비 완료 메시지 작성
    strncpy(ready_confirm_msg.rcp_header.proto_ver, "rcp/1.1", sizeof(ready_confirm_msg.rcp_header.proto_ver));
    strncpy(ready_confirm_msg.rcp_header.msg_type, MSG_TYPE_READY, sizeof(ready_confirm_msg.rcp_header.msg_type));
    //ready_confirm_msg.rcp_header.err = RESPONSE_CODE_MANAGER_SUCCESS_TRANMIT;
    ready_confirm_msg.rcp_header.timestamp = time(NULL);

    // 직렬화
    size_t serialized_size = serialize(&ready_confirm_msg, buffer);

    // 메시지 전송
    if (write(__client_fd, buffer, serialized_size) == -1) {
        perror("write failed");
        return;
    }

    printf("Sent response to client: System is ready.\n");
    printf("Response code: 1 (confirm)\n");
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    rct_instance pdu;
    uint8_t buffer[BUF_SIZE];

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("10.10.14.54");  // 서버 IP (로컬호스트)
    server_addr.sin_port = htons(PORT);

    // 소켓 생성
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 서버에 연결
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connect failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Client B connected to server\n");

    try_login(sockfd);

    // 서버로부터 메시지를 지속적으로 받음
    while (1) {
        ssize_t nbytes = read(sockfd, buffer, BUF_SIZE);  // PDU 구조체 크기만큼 읽기
        if (nbytes <= 0) {
            perror("Read error or connection closed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        deserialize(buffer, &pdu);

        // if(pdu.rcp_header.msg_type == MSG_TYPE_CONFIRM) {
        //     send_confirm_ready_message(sockfd);
        //     printf("received successfully\n");

        //     continue;
        // }

        // 받은 PDU 체크섬 검증
        if (verify_checksum(&pdu)) {
            // 체크섬이 유효할 때만 데이터 출력
            printf("Received from server:\n");
            printf("Proto Ver: %s\n", pdu.rcp_header.proto_ver);
            printf("Msg Type: %s\n", pdu.rcp_header.msg_type);
            printf("Error: %d\n", pdu.rcp_header.err);
            printf("Payload Size: %d\n", pdu.rcp_header.p_size);
            printf("Check: %d\n", checksum);
            printf("Timestamp: %lu\n", pdu.rcp_header.timestamp);
            printf("Data Length: %d\n", pdu.rcp_header.d_len);
            printf("Color: %d\n", pdu.rc_data.color);
            printf("Data: %s\n", pdu.rc_data.data);

            // 좌표와 회전 값 출력
            printf("Yaw: %.2f\n", pdu.rc_data.yaw);
            printf("Pitch: %.2f\n", pdu.rc_data.pitch);
            printf("Roll: %.2f\n", pdu.rc_data.roll);
            printf("X Position: %.2f\n", pdu.rc_data.x_pos);
            printf("Y Position: %.2f\n", pdu.rc_data.y_pos);
            printf("Z Position: %.2f\n", pdu.rc_data.z_pos);
            printf("Flag: %d", pdu.rc_data.detect_flag);
        } else {
            printf("Checksum validation failed. Data discarded.\n");
        }
    }

    close(sockfd);
    return 0;
}
