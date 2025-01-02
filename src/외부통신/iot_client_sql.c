#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <mysql/mysql.h>

#define BUF_SIZE 100
#define NAME_SIZE 20
#define ARR_CNT 4

typedef struct {
    int primary_sock;
    int secondary_sock;
} SOCKS;

void* send_msg(void* arg);
void* recv_msg(void* arg);
void error_handling(char* msg);
void save_to_db(MYSQL *conn, const char *message);

char name[NAME_SIZE] = "[Default]";
char msg[BUF_SIZE];
MYSQL *conn;
int done_flag = 1;

void finish_with_error(MYSQL *con) {
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
    exit(1);
}

int main(int argc, char* argv[]) {
    int primary_sock, secondary_sock;
    struct sockaddr_in primary_addr, secondary_addr;
    pthread_t snd_thread, rcv_thread;
    void* thread_return;

    if (argc != 6) {
        printf("Usage : %s <Primary_IP> <Primary_Port> <Secondary_IP> <Secondary_Port> <ID>\n", argv[0]);
        exit(1);
    }

    sprintf(name, "%s", argv[5]);

    // Primary 서버 연결
    primary_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (primary_sock == -1)
        error_handling("Primary socket() error");

    memset(&primary_addr, 0, sizeof(primary_addr));
    primary_addr.sin_family = AF_INET;
    primary_addr.sin_addr.s_addr = inet_addr(argv[1]);
    primary_addr.sin_port = htons(atoi(argv[2]));

    if (connect(primary_sock, (struct sockaddr*)&primary_addr, sizeof(primary_addr)) == -1)
        error_handling("Primary connect() error");

    printf("Connected to primary server: %s:%s\n", argv[1], argv[2]);

    // Secondary 서버 연결
    secondary_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (secondary_sock == -1)
        error_handling("Secondary socket() error");

    memset(&secondary_addr, 0, sizeof(secondary_addr));
    secondary_addr.sin_family = AF_INET;
    secondary_addr.sin_addr.s_addr = inet_addr(argv[3]);
    secondary_addr.sin_port = htons(atoi(argv[4]));

    if (connect(secondary_sock, (struct sockaddr*)&secondary_addr, sizeof(secondary_addr)) == -1)
        error_handling("Secondary connect() error");

    printf("Connected to secondary server: %s:%s\n", argv[3], argv[4]);

    // Primary 서버에 ID 전송
    sprintf(msg, "[%s:PASSWD]", name);
    write(primary_sock, msg, strlen(msg));
    printf("Sent ID to primary server: %s\n", msg);

    // Secondary 서버에 ID 전송
    write(secondary_sock, msg, strlen(msg));
    printf("Sent ID to secondary server: %s\n", msg);

    SOCKS socks = {primary_sock, secondary_sock};

    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&socks);
    pthread_create(&snd_thread, NULL, send_msg, (void*)&socks);

    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);

    close(primary_sock);
    close(secondary_sock);

    return 0;
}

void* send_msg(void* arg) {
    SOCKS* socks = (SOCKS*)arg;
    int primary_sock = socks->primary_sock;
    int secondary_sock = socks->secondary_sock;
    char name_msg[NAME_SIZE + BUF_SIZE + 2];
    char query[256];
    MYSQL_RES* res;
    MYSQL_ROW row;
    static int last_sent_id = -1;  // 마지막으로 전송된 ID 저장
    int current_status = -1;      // 현재 IR 상태
    int last_status = -1;         // 이전 IR 상태

    MYSQL* conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "localhost", "iot", "pwiot", "iotdb", 3306, NULL, 0)) {
        fprintf(stderr, "ERROR: %s\n", mysql_error(conn));
        exit(1);
    } else {
        printf("Connected to MariaDB in send_msg.\n");
    }

    while (1) {
        // IRStatus가 1인 데이터 검색
        snprintf(query, sizeof(query),
                 "SELECT ID, IRStatus, color FROM ObjectDetection WHERE IRStatus=1 LIMIT 1");

        if (mysql_query(conn, query)) {
            fprintf(stderr, "DB Query Error: %s\n", mysql_error(conn));
            continue;
        }

        res = mysql_store_result(conn);
        if (res == NULL) {
            fprintf(stderr, "DB Result Error: %s\n", mysql_error(conn));
            continue;
        }

        row = mysql_fetch_row(res);
        if (row != NULL) {
            int id = atoi(row[0]);
            current_status = atoi(row[1]);
            char* color = row[2];
            int col = 0;

            // 색상에 따른 코드 지정
            if (strcmp(color, "red") == 0) {
                col = 1;
            } else if (strcmp(color, "green") == 0) {
                col = 2;
            } else if (strcmp(color, "blue") == 0) {
                col = 3;
            }

            // 현재 상태가 1이고, 마지막으로 전송된 ID와 다른 경우 메시지 전송
            if (current_status == 1 && id != last_sent_id && done_flag == 1) {
                snprintf(name_msg, sizeof(name_msg), "%d %d\n", current_status, col);
                if (write(primary_sock, name_msg, strlen(name_msg)) <= 0) {
                    fprintf(stderr, "Failed to send message to primary server.\n");
                } else {
                    printf("Sent to primary server: %s\n", name_msg);
                }

                if (write(secondary_sock, name_msg, strlen(name_msg)) <= 0) {
                    fprintf(stderr, "Failed to send message to secondary server.\n");
                } else {
                    printf("Sent to secondary server: %s\n", name_msg);
                }

                done_flag = 0;
                last_sent_id = id;  // 전송된 ID 업데이트
                last_status = current_status;  // 상태 업데이트
            }
        } else {
            current_status = 0;  // 결과가 없을 때 상태를 0으로 설정
        }

        mysql_free_result(res);

        // 상태가 변화했을 경우 처리 (추가 로직 필요 시 사용)
        if (current_status != last_status) {
            printf("IRStatus changed: %d -> %d\n", last_status, current_status);
            last_status = current_status;
        }

        // 1초 대기
        sleep(1);
    }

    mysql_close(conn);
    return NULL;
}

void error_handling(char* msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

void save_to_db(MYSQL *conn, const char *message) {
    char query[512];
    char subquery[256];
    MYSQL_RES *res;
    MYSQL_ROW row;
    int result;
    int target_id = -1;

    if (strcmp(message, "DETECT") == 0) {
        sprintf(subquery,
                "SELECT MIN(ID) FROM ObjectDetection WHERE IRStatus IS NULL AND IRTimestamp IS NULL");
    } else if (strcmp(message, "END_DETECT") == 0) {
        sprintf(subquery,
                "SELECT MIN(ID) FROM ObjectDetection WHERE IRStatus=1");
    } else {
        fprintf(stderr, "Unknown message format: %s\n", message);
        return;
    }

    printf("Executing Subquery: %s\n", subquery);

    result = mysql_query(conn, subquery);
    if (result != 0) {
        fprintf(stderr, "Subquery failed: %s\nQuery: %s\n", mysql_error(conn), subquery);
        return;
    }

    res = mysql_store_result(conn);
    if (res == NULL) {
        fprintf(stderr, "Subquery result failed: %s\n", mysql_error(conn));
        return;
    }

    row = mysql_fetch_row(res);
    if (row && row[0]) {
        target_id = atoi(row[0]);
        printf("Target ID: %d\n", target_id);
    } else {
        printf("No target record found for update.\n");
        mysql_free_result(res);
        return;
    }
    mysql_free_result(res);

    if (strcmp(message, "DETECT") == 0) {
        snprintf(query, sizeof(query),
                 "UPDATE ObjectDetection SET IRStatus=1, IRTimestamp=NOW() WHERE ID=%d", target_id);
    } else if (strcmp(message, "END_DETECT") == 0) {
        snprintf(query, sizeof(query),
                 "UPDATE ObjectDetection SET IRStatus=0, IRTimestamp=NOW() WHERE ID=%d", target_id);
    }

    printf("Executing Update Query: %s\n", query);

    result = mysql_query(conn, query);
    if (result != 0) {
        fprintf(stderr, "Update Query failed: %s\nQuery: %s\n", mysql_error(conn), query);
    } else {
        printf("Update Query succeeded: %s\n", query);
    }
}

void* recv_msg(void* arg) {
    MYSQL* conn;
    MYSQL_ROW sqlrow;
    int res;
    char sql_cmd[200] = {0};
    char* host = "localhost";
    char* user = "iot";
    char* pass = "pwiot";
    char* dbname = "iotdb";

    SOCKS* socks = (SOCKS*)arg;
    int* primary_sock = &socks->primary_sock;
    int i;
    char* pToken;
    char* pArray[ARR_CNT] = {0};

    char name_msg[NAME_SIZE + BUF_SIZE + 1];
    int str_len;

    // MariaDB 연결
    conn = mysql_init(NULL);
    if (!(mysql_real_connect(conn, host, user, pass, dbname, 0, NULL, 0))) {
        fprintf(stderr, "ERROR : %s[%d]\n", mysql_error(conn), mysql_errno(conn));
        exit(1);
    } else {
        printf("Connection Successful!\n\n");
    }

    while (1) {
        memset(name_msg, 0x0, sizeof(name_msg));
        str_len = read(*primary_sock, name_msg, NAME_SIZE + BUF_SIZE);
        if (str_len <= 0) {
            *primary_sock = -1;
            return NULL;
        }
        fputs(name_msg, stdout);

        name_msg[str_len - 1] = 0;  // '\n' 제거

        pToken = strtok(name_msg, "[:@]");
        i = 0;
        while (pToken != NULL) {
            pArray[i] = pToken;
            if (++i >= ARR_CNT)
                break;
            pToken = strtok(NULL, "[:@]");
        }

        if (strcmp(pArray[1], "DONE") == 0){
            done_flag = 1;
        }
        // 메시지가 DETECT 또는 END_DETECT인 경우 save_to_db 호출
        if (strcmp(pArray[1], "DETECT") == 0) {
            save_to_db(conn, "DETECT"); // conn 추가
        } else if (strcmp(pArray[1], "END_DETECT") == 0) {
            save_to_db(conn, "END_DETECT"); // conn 추가
        }
    }
    mysql_close(conn);
}

