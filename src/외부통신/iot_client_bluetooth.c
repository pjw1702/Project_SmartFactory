#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <mysql/mysql.h>

#define BUF_SIZE 100
#define NAME_SIZE 20
#define ARR_CNT 5

typedef struct {
    int sockfd;
    int btfd;
    char sendid[NAME_SIZE];
} DEV_FD;

char name[NAME_SIZE] = "[default]";
MYSQL *conn; // MariaDB 연결 객체

void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_handling(char *msg);
void init_db();
void save_to_db(const char *message);

int main(int argc, char *argv[]) {
    DEV_FD dev_fd;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    void *thread_return;
    int ret;
    struct sockaddr_rc addr = {0};
    //char dest[18] = "98:DA:60:09:60:4B"; // iot17
    char dest[18] = "20:16:03:10:91:20"; // iot17
    char msg[BUF_SIZE];

    if (argc != 4) {
        printf("Usage : %s <IP> <port> <name>\n", argv[0]);
        exit(1);
    }

    sprintf(name, "%s", argv[3]);

    // 서버 소켓 연결
    dev_fd.sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (dev_fd.sockfd == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(dev_fd.sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    sprintf(msg, "[%s:PASSWD]", name);
    write(dev_fd.sockfd, msg, strlen(msg));

    // Bluetooth 소켓 연결
    dev_fd.btfd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (dev_fd.btfd == -1) {
        perror("socket()");
        exit(1);
    }

    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t)1;
    str2ba(dest, &addr.rc_bdaddr);

    ret = connect(dev_fd.btfd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1) {
        perror("connect()");
        exit(1);
    }

    // MariaDB 초기화
    init_db();

    // 송수신 스레드 생성
    pthread_create(&rcv_thread, NULL, recv_msg, (void *)&dev_fd);
    pthread_create(&snd_thread, NULL, send_msg, (void *)&dev_fd);

    pthread_join(snd_thread, &thread_return);

    // MariaDB 연결 종료
    mysql_close(conn);

    close(dev_fd.sockfd);
    return 0;
}

void *send_msg(void *arg) { // Bluetooth --> Server
    DEV_FD *dev_fd = (DEV_FD *)arg;
    int ret;
    fd_set initset, newset;
    struct timeval tv;
    char msg[BUF_SIZE];
    int total = 0;

    FD_ZERO(&initset);
    FD_SET(dev_fd->btfd, &initset);

    while (1) {
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        newset = initset;
        ret = select(dev_fd->btfd + 1, &newset, NULL, NULL, &tv);

        if (FD_ISSET(dev_fd->btfd, &newset)) {
            ret = read(dev_fd->btfd, msg + total, BUF_SIZE - total);
            if (ret > 0) {
                total += ret;
            } else if (ret == 0) {
                dev_fd->sockfd = -1;
                return NULL;
            }

            if (msg[total - 1] == '\n') { // 메시지가 완료되었는지 확인
                msg[total] = 0;
                total = 0;
            } else {
                continue;
            }

            fputs("ARD:", stdout);
            fputs(msg, stdout);

            // 서버로 메시지 전송
            if (write(dev_fd->sockfd, msg, strlen(msg)) <= 0) {
                dev_fd->sockfd = -1;
                return NULL;
            }
            //if (strncmp(msg, "[SCM_SQL]DETECT", 14) == 0) {
           //   save_to_db("DETECT");
            //} else if (strncmp(msg, "[SCM_SQL]END_DETECT", 18) == 0) {
             //   save_to_db("END_DETECT");
            //} else {
             //   fprintf(stderr, "Unknown message format: %s\n", msg);
            //}
            // 메시지가 DETECT 또는 END_DETECT일 경우 MariaDB에 저장
            //if (strstr(msg, "DETECT") != NULL) {
            //    save_to_db("DETECT"); // DETECT 메시지 처리
            //} else if (strstr(msg, "END_DETECT") != NULL) {
            //    save_to_db("END_DETECT"); // END_DETECT 메시지 처리
            //}
        }

        if (ret == 0) { // 타임아웃 시
            if (dev_fd->sockfd == -1)
                return NULL;
        }
    }
}

void * recv_msg(void * arg)  // server --> bluetooth
{
    DEV_FD *dev_fd = (DEV_FD *)arg;
    char name_msg[NAME_SIZE + BUF_SIZE +1];
    int str_len;

    while (1) {
        memset(name_msg, 0x0, sizeof(name_msg)); // 버퍼 초기화
        str_len = read(dev_fd->sockfd, name_msg, NAME_SIZE + BUF_SIZE); // 서버로부터 메시지 읽기
        if (str_len <= 0) {
            dev_fd->sockfd = -1; // 소켓 연결 종료 처리
            return NULL;
        }

        name_msg[str_len] = 0; // 문자열 끝에 NULL 추가
        fputs("SRV:", stdout); // 서버로부터 받은 메시지 표시
        fputs(name_msg, stdout);

        // DETECT 또는 END_DETECT 메시지 감지 및 처리
        if (strstr(name_msg, "DETECT") != NULL) {
            write(dev_fd->btfd, "[SCM_SQL]DETECT\n", strlen("[SCM_SQL]DETECT\n")); // Bluetooth로 DETECT 전송
        } else if (strstr(name_msg, "END_DETECT") != NULL) {
            write(dev_fd->btfd, "[SCM_SQL]END_DETECT\n", strlen("[SCM_SQL]END_DETECT\n")); // Bluetooth로 END_DETECT 전송
        } else {
            write(dev_fd->btfd, name_msg, strlen(name_msg)); // 그 외 메시지 Bluetooth로 전송
        }
    }
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

void init_db() {
    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }

    // MariaDB에 연결
    if (mysql_real_connect(conn, "localhost", "iot", "pwiot", "iotdb", 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed\n");
        mysql_close(conn);
        exit(1);
    }
}
void save_to_db(const char *message) {
    char query[512];
    char subquery[256];
    MYSQL_RES *res;
    MYSQL_ROW row;
    int result;
    int target_id = -1;

    if (strcmp(message, "DETECT") == 0) {
        // 가장 처음 들어온 IRStatus가 NULL인 레코드의 ID 가져오기
        sprintf(subquery,
                "SELECT MIN(ID) FROM ObjectDetection WHERE IRStatus IS NULL AND IRTimestamp IS NULL");
    } else if (strcmp(message, "END_DETECT") == 0) {
        // 가장 처음 들어온 IRStatus가 1인 레코드의 ID 가져오기
        sprintf(subquery,
                "SELECT MIN(ID) FROM ObjectDetection WHERE IRStatus=1");
    } else {
        fprintf(stderr, "Unknown message format: %s\n", message);
        return;
    }

    // 서브쿼리 실행
    printf("Executing Subquery: %s\n", subquery); // 디버깅용 로그
    result = mysql_query(conn, subquery);
    if (result != 0) {
        fprintf(stderr, "Subquery failed: %s\nQuery: %s\n", mysql_error(conn), subquery);
        return;
    }

    // 서브쿼리 결과 처리
    res = mysql_store_result(conn);
    if (res == NULL) {
        fprintf(stderr, "Subquery result failed: %s\n", mysql_error(conn));
        return;
    }

    row = mysql_fetch_row(res);
    if (row && row[0]) {
        target_id = atoi(row[0]); // 가장 처음 들어온 레코드 ID
        printf("Target ID: %d\n", target_id); // 디버깅용 로그
    } else {
        printf("No target record found for update.\n");
        mysql_free_result(res);
        return; // 업데이트할 대상이 없으면 종료
    }
    mysql_free_result(res);

    // 업데이트 쿼리 생성
    if (strcmp(message, "DETECT") == 0) {
        // DETECT 상태: IRStatus=1, IRTimestamp를 현재 시각으로 업데이트
        sprintf(query,
                "UPDATE ObjectDetection "
                "SET IRStatus=1, IRTimestamp=NOW() "
                "WHERE ID=%d", target_id);
    } else if (strcmp(message, "END_DETECT") == 0) {
        // END_DETECT 상태: IRStatus=0, IRTimestamp를 현재 시각으로 업데이트
        sprintf(query,
                "UPDATE ObjectDetection "
                "SET IRStatus=0, IRTimestamp=NOW() "
                "WHERE ID=%d", target_id);
    }

    // 업데이트 쿼리 실행
    printf("Executing Update Query: %s\n", query); // 디버깅용 로그
    result = mysql_query(conn, query);
    if (result != 0) {
        fprintf(stderr, "Update Query failed: %s\nQuery: %s\n", mysql_error(conn), query);
    } else {
        printf("Update Query succeeded: %s\n", query);
    }
}
