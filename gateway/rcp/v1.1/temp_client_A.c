// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>

// #define BUF_SIZE 10
// #define PORT 8080

// int main() {
//     int sockfd;
//     struct sockaddr_in server_addr;
//     char buffer[BUF_SIZE];
//     int flag, color;

//     // 서버 주소 설정
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_addr.s_addr = inet_addr("10.10.14.54");  // 서버 IP (로컬호스트)
//     server_addr.sin_port = htons(PORT);

//     // 소켓 생성
//     if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
//         perror("Socket creation failed");
//         exit(EXIT_FAILURE);
//     }

//     // 서버에 연결
//     if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
//         perror("Connect failed");
//         close(sockfd);
//         exit(EXIT_FAILURE);
//     }

//     printf("Client A connected to server\n");

//     // 클라이언트 A에서 지속적으로 서버로 데이터 전송
//     while (1) {
//         // flag와 color 값을 입력받음
//         printf("Enter flag (1 for sending to B): ");
//         scanf("%d", &flag);
//         printf("Enter color value: ");
//         scanf("%d", &color);

//         snprintf(buffer, BUF_SIZE, "%d %d", flag, color);
//         if (write(sockfd, buffer, strlen(buffer)) == -1) {
//             perror("Write failed");
//             close(sockfd);
//             exit(EXIT_FAILURE);
//         }

//         printf("Sent to server: %d %d\n", flag, color);

//         // 서버로부터 응답 받기
//         //ssize_t nbytes = read(sockfd, buffer, BUF_SIZE - 1);
//         // if (nbytes <= 0) {
//         //     perror("Read error or connection closed");
//         //     close(sockfd);
//         //     exit(EXIT_FAILURE);
//         // }

//         // buffer[nbytes] = '\0';
//         // printf("Received from server: %s\n", buffer);
//     }

//     close(sockfd);
//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024  // 버퍼 크기를 충분히 크게 설정
#define PORT 8080

#define FAIL        0
#define SUCCESS     1

typedef uint8_t Bool;

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
        snprintf(buffer, BUF_SIZE, "%s:%s", id, password);
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

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];
    int flag, color;
    
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

    printf("Client A connected to server\n");

    try_login(sockfd);

    // 인증 후 지속적으로 데이터 전송
    while (1) {
        // flag와 color 값을 입력받음
        printf("Enter flag (1 for sending to B): ");
        scanf("%d", &flag);
        printf("Enter color value: ");
        scanf("%d", &color);

        snprintf(buffer, BUF_SIZE, "%d %d", flag, color);
        if (write(sockfd, buffer, strlen(buffer)) == -1) {
            perror("Write failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        printf("Sent to server: %d %d\n", flag, color);

        // 서버로부터 응답 받기
        ssize_t nbytes = read(sockfd, buffer, BUF_SIZE - 1);
        if (nbytes <= 0) {
            perror("Read error or connection closed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        buffer[nbytes] = '\0';
        printf("Received from server: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}
