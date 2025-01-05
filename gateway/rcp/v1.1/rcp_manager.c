/* Apply rcp to this program */

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

#define MAX_EVENTS 10
#define BUF_SIZE 1024
#define PORT 8080
#define THREAD_POOL_SIZE 4
#define MQ_NAME "/rcp_task_queue"
#define MAX_TASKS 10
#define MAX_CLNT 32      // user 수
#define ID_SIZE 30      // ID의 길이
#define ARR_CNT 5      // ??

#define MAX_BUF_SIZE 256
#define RCP_PROTO_VER_SIZE  8
#define RCP_MESSAGE_TYPE_SIZE   16
#define RCP_DATA_SIZE   20

#define CONVEYER_ID_01  1
#define ROBOT_ID_01     1

#define MACHINE_TYPE_UNKNOWN         0
#define MACHINE_TYPE_CONVEYER        1
#define MACHINE_TYPE_ROBOT           2

#define MSG_TYPE_CONFIRM    "CONFIRM"
#define MSG_TYPE_READY    "READY"

#define FALSE   0
#define TRUE    1

typedef struct {      // msg의 정보를 가지는 구조체
      char fd;
      char *from;
      char *to;
      char *msg;
      int len;
} __attribute__((packed)) MSG_INFO;

typedef struct {      // user의 정보를 가지고 있는 구조체
      int index;
      int fd;
      char ip[20];
      char id[ID_SIZE];
      char pw[ID_SIZE];
} __attribute__((packed)) CLIENT_INFO;


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

// temp descriptor of client
typedef struct {
    short client_fd;
    uint8_t machine_type; // 0: Conveyer, 1: Robot
    uint8_t detect_flag;
    uint8_t color;
} __attribute__((packed)) client_task_t;

// file descripter
typedef struct fd_list_t {
    int conveyer_fd[THREAD_POOL_SIZE];
    int robot_fd[THREAD_POOL_SIZE];
} __attribute__((packed)) FDList;

// 메시지 큐 속성
struct mq_attr mq_attr = {
    .mq_flags = 0,
    .mq_maxmsg = MAX_TASKS,
    //.mq_msgsize = sizeof(client_task_t)+1,
    .mq_msgsize = BUF_SIZE,
    .mq_curmsgs = 0
};

// 함수들 사용을 위한 정의
void * clnt_connection(void * arg);
void send_msg(MSG_INFO * msg_info, CLIENT_INFO * first_client_info);
void error_handling(char * msg);
void log_file(char * msgstr);
void getlocaltime(char * buf);

// 서버 데이터
FDList fd_data = {{0}, {0}};

// user 수
int clnt_cnt=0;
int conveyer_fd, robot_fd;

uint8_t isFirst = TRUE;

// mutex 공용체 정의
pthread_mutex_t user_mutx;
pthread_mutex_t fd_data_mutx;  
pthread_mutex_t thread_pool_mutx; 

CLIENT_INFO client_info[MAX_CLNT];

/* Helper Functions */

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
void send_confirm_ready_message(int __client_fd) {
    rct_instance ready_confirm_msg;
    uint8_t buffer[BUF_SIZE];
    memset(&ready_confirm_msg, 0, sizeof(rct_instance));

    // 준비 완료 메시지 작성
    strncpy(ready_confirm_msg.rcp_header.proto_ver, "rcp/1.1", sizeof(ready_confirm_msg.rcp_header.proto_ver));
    strncpy(ready_confirm_msg.rcp_header.msg_type, MSG_TYPE_CONFIRM, sizeof(ready_confirm_msg.rcp_header.msg_type));
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

void send_command(int __robot_fd, uint8_t __detect_flag, uint8_t __color) {
    // PDU 구조체 생성 및 초기화
    // PDU *pdu = (PDU*)malloc(sizeof(PDU));
    // pdu->rcp_header = (Rcphdr *)malloc(sizeof(Rcphdr));
    // pdu->rc_data = (Rcdata *)malloc(sizeof(Rcdata));
    // if (!pdu->rcp_header || !pdu->rc_data) {
    //     perror("Failed to allocate memory for PDU fields");
    //     free(pdu->rcp_header);
    //     free(pdu->rc_data);
    //     free(pdu);
    //     exit(EXIT_FAILURE);
    // }

    // memset(pdu, 0, sizeof(PDU));
    
    // // 헤더 설정
    // strncpy(pdu->rcp_header->proto_ver, "rcp/1.1", RCP_PROTO_VER_SIZE);
    // strncpy(pdu->rcp_header->msg_type, "COMMAND", RCP_MESSAGE_TYPE_SIZE);
    // pdu->rcp_header->err = 0;
    // pdu->rcp_header->p_size = sizeof(PDU);
    // pdu->rcp_header->check = 0; // 체크섬은 이후 설정
    // pdu->rcp_header->timestamp = (uint64_t)time(NULL);
    // pdu->rcp_header->d_len = sizeof(Rcdata);
    // //memset(pdu.rcp_header.hmac, 0, sizeof(pdu.rcp_header.hmac)); // 예제에서는 0으로 초기화

    // // Rcdata 설정
    // pdu->rc_data->yaw = 1.0;    // 예제 값
    // pdu->rc_data->pitch = 2.0;  // 예제 값
    // pdu->rc_data->roll = 3.0;   // 예제 값
    // pdu->rc_data->x_pos = 4.0;  // 예제 값
    // pdu->rc_data->y_pos = 5.0;  // 예제 값
    // pdu->rc_data->z_pos = 6.0;  // 예제 값
    // // id는 원래 strtok를 통해 추출해서 저장해야 함
    // pdu->rc_data->robot_id = ROBOT_ID_01;
    // pdu->rc_data->conveyer_id = CONVEYER_ID_01;
    // pdu->rc_data->detect_flag = __detect_flag;
    // pdu->rc_data->color = __color;
    // snprintf(pdu->rc_data->data, RCP_DATA_SIZE, "ControlMsg");

    // // 체크섬 계산 및 설정
    // pdu->rcp_header->check = calculate_checksum(pdu, sizeof(PDU));

    // // 데이터 전송
    // if (write(__robot_fd, pdu, sizeof(PDU)) == -1) {
    //     perror("Write to client B failed");
    // } else {
    //     printf("Sent PDU to client B\n");
    // }

    // free(pdu->rcp_header);
    // free(pdu->rc_data);
    // free(pdu);

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
    memset(pdu.rcp_header.hmac, 0, sizeof(pdu.rcp_header.hmac)); // 예제에서는 0으로 초기화

    // Rcdata 설정
    pdu.rc_data.yaw = 1.0;    // 예제 값
    pdu.rc_data.pitch = 2.0;  // 예제 값
    pdu.rc_data.roll = 3.0;   // 예제 값
    pdu.rc_data.x_pos = 4.0;  // 예제 값
    pdu.rc_data.y_pos = 5.0;  // 예제 값
    pdu.rc_data.z_pos = 6.0;  // 예제 값
    // id는 원래 strtok를 통해 추출해서 저장해야 함
    pdu.rc_data.robot_id = ROBOT_ID_01;
    pdu.rc_data.conveyer_id = CONVEYER_ID_01;
    pdu.rc_data.detect_flag = __detect_flag;
    pdu.rc_data.color = __color;
    snprintf(pdu.rc_data.data, sizeof(pdu.rc_data.data), "ControlMsg");

    // 체크섬 계산 및 설정
    pdu.rcp_header.check = calculate_checksum(&pdu, sizeof(rct_instance));

    // 데이터 전송
    if (write(__robot_fd, &pdu, sizeof(PDU)) == -1) {
        perror("Write to client B failed");
    } else {
        printf("Sent PDU to client B\n");
    }
}

/* Main Processes */

void init_user_info() {
    // 총 32개의 user 정보를 가지는 구조체. 현재는 지정된 id와 지정된 pw만 사용
    // account
    CLIENT_INFO user_info[MAX_CLNT] = {{0,-1,"","PJW_ARD","PASSWD"}, \
        {0,-1,"","PJW_LIN","PASSWD"},  {0,-1,"","PJW_AND","PASSWD"}, \
        {0,-1,"","PJW_STM32","PASSWD"},  {0,-1,"","PJW_BT","PASSWD"}, \
        {0,-1,"","PJW_SQL","PASSWD"},  {0,-1,"","7","PASSWD"}, \
        {0,-1,"","SCM_SQL","PASSWD"},  {0,-1,"","11","PASSWD"}, \
        {0,-1,"","SCM_CONVEYER","PASSWD"},  {0,-1,"","KEG_ROBOT","PASSWD"}, \
        {0,-1,"","14","PASSWD"},  {0,-1,"","15","PASSWD"}, \
        {0,-1,"","16","PASSWD"},  {0,-1,"","17","PASSWD"}, \
        {0,-1,"","18","PASSWD"},  {0,-1,"","19","PASSWD"}, \
        {0,-1,"","20","PASSWD"},  {0,-1,"","21","PASSWD"}, \
        {0,-1,"","22","PASSWD"},  {0,-1,"","23","PASSWD"}, \
        {0,-1,"","24","PASSWD"},  {0,-1,"","25","PASSWD"}, \
        {0,-1,"","26","PASSWD"},  {0,-1,"","27","PASSWD"}, \
        {0,-1,"","28","PASSWD"},  {0,-1,"","29","PASSWD"}, \
        {0,-1,"","30","PASSWD"},  {0,-1,"","31","PASSWD"}, \
        {0,-1,"","HM_CON","PASSWD"}};

    // 전역 변수에 값 복사
    for (int i = 0; i < MAX_CLNT; i++) {
        client_info[i] = user_info[i];
    }
}


// 클라이언트 소켓에 대한 파일 디스크립터가 필요 함
uint8_t auth_login(struct sockaddr_in clnt_adr, int clnt_sock, char idpasswd[]) {
    int i;
    char *pToken;
    char *pArray[ARR_CNT] = {0};
    char *idToken;
    char *idArray[ARR_CNT] = {0};
    uint8_t machine_type;
    char msg[BUF_SIZE];

    // idpasswd로부터 ':'나 '['나 ']'로 구분된 토큰을 추출
    pToken = strtok(idpasswd, "[:]");
    i = 0;
    while(pToken != NULL) {
        if (i >= ARR_CNT) {
            break;  // 배열 크기를 넘지 않도록 방지
        }
        pArray[i] = pToken;
        i++;
        pToken = strtok(NULL, "[:]");
    }

    printf("Tokenized data: pArray[0] = %s, pArray[1] = %s\n", pArray[0], pArray[1]);

    // 클라이언트 목록에서 해당 아이디 찾기
    for(i = 0; i < MAX_CLNT; i++) {
        // 아이디 확인
        if (!strcmp(client_info[i].id, pArray[0])) {
            // 해당 유저가 접속 중인지 아닌지 판별
            if (client_info[i].fd != -1) {
                // 이미 로그인된 경우
                sprintf(msg, "[%s] Already logged!\n", pArray[0]);
                write(clnt_sock, msg, strlen(msg));
                log_file(msg);
                shutdown(clnt_sock, SHUT_WR);
#if 1   // for MCU
                client_info[i].fd = -1;
#endif  
                break;
            }

            // // 해당 클라이언트가 컨베이어인지 로봇인지 판별
            idArray[0] = pArray[0]; // ID만 저장
            idToken = strtok(idArray[0], "_");  // "_"로 구분하여 나누기

            i = 0;
            while(idToken != NULL) {
                if (i >= ARR_CNT) {
                    break;  // 배열 크기를 넘지 않도록 방지
                }
                idArray[i] = idToken;
                i++;
                idToken = strtok(NULL, "_");
            }

            // 머신 타입을 판단
            if (!strcmp(idArray[1], "SQL"))
                machine_type = MACHINE_TYPE_CONVEYER;
            else if (!strcmp(idArray[1], "ROBOT"))
                machine_type = MACHINE_TYPE_ROBOT;
            else
                machine_type = MACHINE_TYPE_UNKNOWN;

            // 비밀번호 확인
            if (!strcmp(client_info[i].pw, pArray[1])) {
                // 인증 성공, 클라이언트 정보 갱신
                strcpy(client_info[i].ip, inet_ntoa(clnt_adr.sin_addr));
                pthread_mutex_lock(&user_mutx);
                client_info[i].index = i; 
                client_info[i].fd = clnt_sock; 
                clnt_cnt++;
                pthread_mutex_unlock(&user_mutx);

                sprintf(msg, "[%s] New connected! (ip:%s, fd:%d, sockcnt:%d)\n", pArray[0], inet_ntoa(clnt_adr.sin_addr), clnt_sock, clnt_cnt);
                log_file(msg);
                printf("%s", msg);

                sprintf(msg, "Authentication successful\n");
                log_file(msg);
                write(clnt_sock, msg, strlen(msg));

                break;
            }
        }
    }

    // 인증 실패
    if (i == MAX_CLNT) {
        sprintf(msg, "[%s] Authentication Error!\n", pArray[0]);
        write(clnt_sock, msg, strlen(msg));
        log_file(msg);
        shutdown(clnt_sock, SHUT_WR);
    }

    printf("%d\n", machine_type);
    return machine_type;
}

// 컨베이어 수신 스레드
// 스레드 풀에서 호출
// 컨베이어 파일 디스크립터를 파라미터로 받음
void *conveyer_handler(void *arg) {
    //char buffer[BUF_SIZE];
    ssize_t nbytes;
    client_task_t task;

    // 컨베이어 파일 디스크립터 추출
    // 클라이언트 파일 디스크립터
    int client_fd;
    char buffer[BUF_SIZE];

    //printf("get FD = %d\n", client_fd);

    //mqd_t mq = mq_open(MQ_NAME, O_CREAT | O_RDWR, 0644, &mq_attr);
    mqd_t mq = mq_open(MQ_NAME, O_RDWR | O_NONBLOCK);
    if (mq == (mqd_t)-1) {
        perror("Failed to open message queue in conveyer_handler");
        return NULL;
    }

    while (1) {
        client_fd = *(int*)arg;
        //if(task.machine_type == MACHINE_TYPE_CONVEYER) {
            // 컨베이어로 부터 메시지 수신
            // ssize_t bytes_read = mq_receive(mq, (char *)&task, sizeof(client_task_t), NULL);
            // if (bytes_read == -1) {
            //     //perror("Failed to receive from message queue");
            //     perror("1");
            //     continue;
            // }
            if (client_fd < 0) {
                perror("accept failed");
                close(client_fd);
                exit(EXIT_FAILURE);
                continue;
            }

            // 컨베이어로부터 데이터 수신
            // 수신할 때까지 대기
            nbytes = read(client_fd, buffer, BUF_SIZE - 1);
            if (nbytes <= 0) {
                if (nbytes == 0) {
                    fputs("Conveyer disconnected.\n", stdout);
                } else {
                    perror("Read error from Conveyer");
                }
                close(client_fd);
                //break;
                continue;
            }
            buffer[nbytes] = '\0';
            printf("Received from Conveyer: %s\n", buffer);

            // 수신된 탐지 플래그, 컬러 프래그 추출
            int detect_flag, color;
            if (sscanf(buffer, "%d %d", &detect_flag, &color) != 2) {
                fprintf(stderr, "Invalid data format: %s\n", buffer);
                continue;
            }
            printf("Received from conveyer: Detect flag=%d, Color=%d\n", detect_flag, color);

            // 로봇 스레드에 데이터 전달
            //task = {client_fd, MACHINE_TYPE_ROBOT, detect_flag, color};
            task.client_fd = client_fd, task.machine_type = MACHINE_TYPE_ROBOT, task.detect_flag = detect_flag, task.color = color;
            printf("%d, %d, %d, %d\n", task.client_fd, task.machine_type, task.detect_flag, task.color);
            // 메시지 큐에 데이터를 전달할 때, 컨베이어에 대한 파일 디스크립터도 같이 전달
            if (mq_send(mq, (const char *)&task, sizeof(client_task_t), 0) == -1) {
                perror("Failed to send task to message queue");
                break;
            } else {
                printf("Conveyer thread is done sucessfully.\n");
                if (write(client_fd, "receive successfully", strlen("receive successfully")) == -1) {
                    perror("Write to robot thread failed");
                } else {
                    printf("Write to robot thread Success\n");
                }
            }
        ///}
    }

    mq_close(mq);
    return NULL;
}

// 로봇 송신 스레드
// 작업 처리 함수
// 스레드 풀에서 호출
void *robot_handler(void *arg) {
    // 로봇 파일 디스크립터 추출
    // 클라이언트 파일 디스크립터
    int client_fd;
    char buffer[BUF_SIZE];
    PDU pdu;

    //printf("get FD = %d\n", client_fd);

    //mqd_t mq = mq_open(MQ_NAME, O_CREAT | O_RDWR, 0644, &mq_attr);
    //mqd_t mq = mq_open(MQ_NAME, O_RDONLY | O_NONBLOCK);
    mqd_t mq = mq_open(MQ_NAME, O_RDONLY);
    if (mq == -1) {
        perror("Worker failed to open message queue");
        return NULL;
    }

    client_task_t task;

    while (1) {
        client_fd = *(int*)arg;
        if (client_fd < 0) {
            perror("accept failed");
            close(client_fd);
            exit(EXIT_FAILURE);
        }

        // 컨베이어 파일 디스크립터 추출
        // int client_fd = task.client_fd;
        // char buffer[BUF_SIZE];

        // 컨베이어 파일 디스크립터로 부터 데이터 수신
        // 컨베이어로 부터 수신 받은 데이터가 없으면 계속 대기하는 방식으로 흐름 제어
        // ssize_t nbytes = read(client_fd, buffer, BUF_SIZE - 1);
        // if (nbytes <= 0) {
        //     perror("Read error or connection closed");
        //     close(client_fd);
        //     continue;
        // }
        // buffer[nbytes] = '\0';

        // 탐지 플래그, 컬러 프래그 수신
        // int detect_flag, color;
        // if (sscanf(buffer, "%d %d", &flag, &color) != 2) {
        //     fprintf(stderr, "Invalid data format: %s\n", buffer);
        //     continue;
        // }
        // printf("Received from client A: Flag=%d, Color=%d\n", flag, color);


        // 여기서 부터 로봇에 대한 흐름제어 로직 구현

        // 로봇으로 데이터 송신
        //if(task.machine_type == MACHINE_TYPE_ROBOT) {


        // 컨베이어 스레드로 부터 메시지 수신
        // 큐는 FIFO 자료구조이므로 순서가 꼬일 일은 없음
        // 원래는 스레드가 하나 밖에 없어서 컨베이어인지 로봇인지 구분하기 위해 타입과 파일 디스크립터를 전달 받기 위해 사용 됨
        // 컨베이어로 부터 수신 받은 데이터가 없으면 계속 대기하는 방식으로 흐름 제어
        while(1) {
            ssize_t bytes_read = mq_receive(mq, (char *)&task, BUF_SIZE, NULL);
            if (bytes_read == -1) {
                perror("Failed to receive from message queue");
                continue;
            }
            break;
        }

        // 로봇으로 부터 메시지 수신
        // READY 메시지를 받기 위함
        // if(!isFirst) {
        //     int cnt;
        //     //send_confirm_ready_message(client_fd);
        //     while(1) {
        //         //if(cnt > 10)
        //         //    send_confirm_ready_message(client_fd);
        //         ssize_t received_size = read(client_fd, buffer, BUF_SIZE);
        //         if (received_size <= 0) {
        //             perror("read failed");
        //             cnt++;
        //             continue;
        //         }
        //         deserialize((uint8_t *)buffer, &pdu);
        //         printf("message receive successfully from Robot1: %s", pdu.rcp_header.msg_type);
        //         break;
        //     }
        // }

        printf("Robot thread is received sucessfully.\n");
        printf("%d, %d, %d, %d\n", task.client_fd, task.machine_type, task.detect_flag, task.color);
        printf("Robot thread' machine type is robot.\n");
        // 흐름 제어 필요
        // 탐지 플래그가 TRUE인 경우에만 데이터 전송
        printf("detect_flag: %d\n", task.detect_flag);
        if (task.detect_flag == TRUE && client_fd > 0) {
            // 처음에는 바로 데이터 전송
            if(isFirst) {
                send_command(client_fd, task.detect_flag, task.color);
                printf("It is first.\n");
                isFirst = FALSE;
                continue;
            } else {
                int cnt = 0;
                ssize_t received_size = read(client_fd, buffer, BUF_SIZE);
                if (received_size <= 0) {
                    perror("read failed");
                    cnt++;
                    continue;
                }
                deserialize((uint8_t *)buffer, &pdu);
                printf("message receive successfully from Robot1: %s", pdu.rcp_header.msg_type);
                //printf("message receive successfully from Robot1: %s", buffer);

                // 두 번째 부터는 DONE 메시지를 받았을 경우에만 데이터 전송
                if(!strcmp(pdu.rcp_header.msg_type, MSG_TYPE_READY)) {
                    // PDU 구조체 생성 및 초기화
                    // PDU pdu = {0};
                    
                    // // 헤더 설정
                    // strncpy(pdu.rcp_header.proto_ver, "v1.0", sizeof(pdu.rcp_header.proto_ver));
                    // strncpy(pdu.rcp_header.msg_type, "RobotCtrl", sizeof(pdu.rcp_header.msg_type));
                    // pdu.rcp_header.err = 0;
                    // pdu.rcp_header.p_size = sizeof(PDU);
                    // pdu.rcp_header.check = 0; // 체크섬은 이후 설정
                    // pdu.rcp_header.timestamp = (uint64_t)time(NULL);
                    // pdu.rcp_header.d_len = sizeof(Rcdata);
                    // memset(pdu.rcp_header.hmac, 0, sizeof(pdu.rcp_header.hmac)); // 예제에서는 0으로 초기화

                    // // Rcdata 설정
                    // pdu.rc_data.yaw = 1.0;    // 예제 값
                    // pdu.rc_data.pitch = 2.0;  // 예제 값
                    // pdu.rc_data.roll = 3.0;   // 예제 값
                    // pdu.rc_data.x_pos = 4.0;  // 예제 값
                    // pdu.rc_data.y_pos = 5.0;  // 예제 값
                    // pdu.rc_data.z_pos = 6.0;  // 예제 값
                    // // id는 원래 strtok를 통해 추출해서 저장해야 함
                    // pdu.rc_data.robot_id = ROBOT_ID_01;
                    // pdu.rc_data.conveyer_id = CONVEYER_ID_01;
                    // pdu.rc_data.detect_flag = (uint8_t)task.detect_flag;
                    // pdu.rc_data.color = (uint8_t)task.color;
                    // snprintf(pdu.rc_data.data, sizeof(pdu.rc_data.data), "ControlMsg");

                    // // 체크섬 계산 및 설정
                    // pdu.rcp_header.check = calculate_checksum(&pdu, sizeof(rct_instance));

                    // // 데이터 전송
                    // if (write(client_fd, &pdu, sizeof(PDU)) == -1) {
                    //     perror("Write to client B failed");
                    // } else {
                    //     printf("Sent PDU to client B\n");
                    // }
                    send_command(client_fd, task.detect_flag, task.color);
                    printf("It is not first.\n");
                }
            }
        } else {
            printf("Flag is 0 or client B is not connected\n");
        }
        //}
    }

    mq_close(mq);
    return NULL;
}

void* rct_brocker_handler(void *arg) {
    // socket, epoll, thread, file descriptor
    int server_fd, epfd, nfds, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    struct epoll_event ev, events[MAX_EVENTS];
    int str_len;

    char idpasswd[(ID_SIZE*2)+3];

    uint8_t machine_type, conveyer_id, robot_id;

    (void)arg;

    // Set accounts of user
    init_user_info();

    // 메시지 큐 생성
    mqd_t mq = mq_open(MQ_NAME, O_CREAT | O_RDWR, 0644, &mq_attr);
    if (mq == -1) {
        perror("Failed to create message queue");
        exit(EXIT_FAILURE);
    }

    // 서버 소켓 생성
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) == -1) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if ((epfd = epoll_create1(0)) == -1) {
        perror("Epoll creation failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        perror("Epoll_ctl failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 스레드 풀 생성
    // pthread_t thread_pool[THREAD_POOL_SIZE];
    // for (int i = 0; i < THREAD_POOL_SIZE; i++) {
    //     if (pthread_create(&thread_pool[i], NULL, worker, NULL) != 0) {
    //         perror("Thread creation failed");
    //         exit(EXIT_FAILURE);
    //     }
    // }

    // 컨베이어 스레드 풀 생성
    pthread_t conveyer_thread_pool[THREAD_POOL_SIZE];

    // 로봇 스레드 풀 생성
    pthread_t robot_thread_pool[THREAD_POOL_SIZE];

    printf("Server is running on port %d...\n", PORT);

    while (1) {
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("Epoll_wait failed");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            // 서버 소켓에 이벤트가 발생한 경우
            pthread_mutex_lock(&fd_data_mutx);
            if (events[i].data.fd == server_fd) {
                // 새 클라이언트 연결
                client_addr_len = sizeof(client_addr);
                client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
                if (client_fd == -1) {
                    perror("Accept failed");
                    continue;
                }
                printf("New connection accepted: FD = %d\n", client_fd);
                pthread_mutex_unlock(&fd_data_mutx);

                // strtok를 통해 접속한 클라이언트가 컨베이어인지 로봇인지 구분해야 함
                // 여기에서 컨베이어인지 로봇인지 구분하는 로직을 구현
                memset(idpasswd, 0, sizeof(idpasswd));
                str_len = read(client_fd, idpasswd, sizeof(idpasswd));
                idpasswd[str_len] = '\0';
                machine_type = auth_login(client_addr, client_fd, idpasswd);


                // 머신 타입 판별
                if(machine_type == MACHINE_TYPE_CONVEYER)
                    conveyer_id = CONVEYER_ID_01;
                else if(machine_type == MACHINE_TYPE_ROBOT)
                    robot_id = ROBOT_ID_01;
                else {
                    perror("Unknown type of machine");
                    continue;
                }

                // 해당 id의 컨베이어가 기존에 연결되어 있지 않은 경우
                // 컨베이어 스레드 생성
                if(machine_type == MACHINE_TYPE_CONVEYER) {
                    pthread_mutex_lock(&thread_pool_mutx);
                    if (fd_data.conveyer_fd[conveyer_id] == 0) {
                        fd_data.conveyer_fd[conveyer_id] = client_fd;
                        // 원래는 컨베이어 측에서 conveyer_id 필드에 해당 컨베이어의 넘버를 기입 후 전송해야 함
                        if (pthread_create(&conveyer_thread_pool[conveyer_id], NULL, conveyer_handler, &fd_data.conveyer_fd[conveyer_id]) != 0) {
                            perror("Thread creation failed");
                            exit(EXIT_FAILURE);
                        }
                        printf("Conveyer 1 is connected: FD = %d\n", client_fd);
                    } else
                        printf("Conveyer 1 is already connected: FD = %d\n", client_fd);
                    pthread_mutex_unlock(&thread_pool_mutx);
                    
                // 해당 id의 로봇이 기존에 연결되어 있지 않은 경우
                // 로봇 스레드 생성
                } else if(machine_type == MACHINE_TYPE_ROBOT) {
                    pthread_mutex_lock(&thread_pool_mutx);
                    if (fd_data.robot_fd[robot_id] == 0) {
                        fd_data.robot_fd[robot_id] = client_fd;
                        // 원래는 로봇 측에서 robot_id 필드에 해당 컨베이어의 넘버를 기입 후 전송해야 함
                        if (pthread_create(&robot_thread_pool[robot_id], NULL, robot_handler, &fd_data.robot_fd[robot_id]) != 0) {
                            perror("Thread creation failed");
                            exit(EXIT_FAILURE);
                        }
                        printf("Robot 1 is connected: FD = %d\n", client_fd);
                    } else
                        printf("Robot 1 is already connected: FD = %d\n", client_fd);
                    pthread_mutex_unlock(&thread_pool_mutx);
                // 이미 연결되어 있는 경우
                } else {
                    printf("Too many clients. Closing connection: FD = %d\n", client_fd);
                    close(client_fd);
                    continue;
                }

                ev.events = EPOLLIN;
                ev.data.fd = client_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);

            // 클라이언트 소켓에 이벤트가 발생한 경우
            // 스레드에서 처리
            } else {
                // client_task_t task = {
                //     .client_fd = events[i].data.fd,
                //     // 지금은 프로젝트라 컨베이어와 로봇 모두 1번으로 고정되어 있기 때문에 임시로 이렇게 판단하도록 작성
                //     // 원래는 이렇게 하면 안되고 유저로 부터 들어오는 id정보에 strtok를 이용하여 판단
                //     .machine_type = (events[i].data.fd == fd_data.conveyer_fd[CONVEYER_ID_01]) ? MACHINE_TYPE_CONVEYER : MACHINE_TYPE_ROBOT
                // };

                // if (mq_send(mq, (const char *)&task, sizeof(client_task_t), 0) == -1) {
                //     perror("Failed to send task to message queue");
                // }

                //task.machine_type = -1;
                pthread_mutex_lock(&thread_pool_mutx);
                for(int j = 0; j < THREAD_POOL_SIZE; j++) {
                    pthread_join(conveyer_thread_pool[j], NULL);
                    pthread_join(robot_thread_pool[j], NULL);
                }
                pthread_mutex_unlock(&thread_pool_mutx);
            }
        }
    }

    mq_unlink(MQ_NAME);
    for(int i = 0; THREAD_POOL_SIZE; i++) {
        close(fd_data.conveyer_fd[i]);
        close(fd_data.robot_fd[i]);
    }
    close(server_fd);
    close(epfd);
}

int main() {
    pthread_t rct_brocker_handler_id;

    // 뮤텍스 생성
    if(pthread_mutex_init(&user_mutx, NULL))
        error_handling("mutex init error");
    if(pthread_mutex_init(&fd_data_mutx, NULL))
        error_handling("mutex init error");
    if(pthread_mutex_init(&thread_pool_mutx, NULL))
        error_handling("mutex init error");

    // 서버 스레드 생성
    if (pthread_create(&rct_brocker_handler_id, NULL, rct_brocker_handler, NULL) != 0) {
        perror("Server thread creation failed");
        exit(EXIT_FAILURE);
    }

    pthread_join(rct_brocker_handler_id, NULL);  // 메인 스레드에서 서버 스레드 종료 대기
    return 0;
}

void error_handling(char *msg)
{
      fputs(msg, stderr);
      fputc('\n', stderr);
      exit(1);
}

void log_file(char * msgstr)
{
      fputs(msgstr,stdout);
}