#include "rcp.h"

// 매니저 데이터
FDList fd_data = {{0}, {0}};

// device 수
int worker_cnt=0;
int conveyer_fd, robot_fd;

uint8_t isFirst = TRUE;

// 메시지 큐 속성
struct mq_attr mq_attr = {
    .mq_flags = 0,
    .mq_maxmsg = MAX_TASKS,
    //.mq_msgsize = sizeof(worker_task_t)+1,
    .mq_msgsize = BUF_SIZE,
    .mq_curmsgs = 0
};

DEVICE_INFO device[WORKER_MAX_CLNT];

void init_device_info() {
    // account
    DEVICE_INFO __device[WORKER_MAX_CLNT] = {{0,-1,"","PJW_ARD","PASSWD"}, \
        {0,-1,"","PJW_LIN","PASSWD"},  {0,-1,"","PJW_AND","PASSWD"}, \
        {0,-1,"","PJW_STM32","PASSWD"},  {0,-1,"","PJW_BT","PASSWD"}, \
        {0,-1,"","PJW_SQL","PASSWD"},  {0,-1,"","7","PASSWD"}, \
        {0,-1,"","SCM_SQL","PASSWD"},  {0,-1,"","11","PASSWD"}, \
        {0,-1,"","12","PASSWD"},  {0,-1,"","KEG_ROBOT","PASSWD"}, \
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
    for (int i = 0; i < WORKER_MAX_CLNT; i++) {
        device[i] = __device[i];
    }
}


// 워커 소켓에 대한 파일 디스크립터가 필요 함
uint8_t auth_worker_connect(struct sockaddr_in __worker_adr, int __worker_sock, char __device_info[]) {
    int i;
    char *pToken;
    char *pArray[ARR_CNT] = {0};
    char *idToken;
    char *idArray[ARR_CNT] = {0};
    uint8_t machine_type = 0;
    char msg[BUF_SIZE];

    pthread_mutex_t *__device_mutx = get_device_mutex();

    // device_info로부터 ':'나 '['나 ']'로 구분된 토큰을 추출
    pToken = strtok(__device_info, "[:]");
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

    // 워커 목록에서 해당 아이디 찾기
    for(i = 0; i < WORKER_MAX_CLNT; i++) {
        // 아이디 확인
        if (!strcmp(device[i].id, pArray[0])) {
            // 해당 유저가 접속 중인지 아닌지 판별
            if (device[i].fd != -1) {
                // 이미 로그인된 경우
                sprintf(msg, "[%s] Already logged!\n", pArray[0]);
                write(__worker_sock, msg, strlen(msg));
                log_file(msg);
                shutdown(__worker_sock, SHUT_WR);
#if 1   // for MCU
                device[i].fd = -1;
#endif  
                break;
            }

            // 해당 워커가 컨베이어인지 로봇인지 판별
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
            if (!strcmp(device[i].pw, pArray[1])) {
                // 인증 성공, 워커 정보 갱신
                strcpy(device[i].ip, inet_ntoa(__worker_adr.sin_addr));
                pthread_mutex_lock(__device_mutx);
                device[i].index = i; 
                device[i].fd = __worker_sock; 
                worker_cnt++;
                pthread_mutex_unlock(__device_mutx);

                sprintf(msg, "[%s] New connected! (ip:%s, fd:%d, sockcnt:%d)\n", pArray[0], inet_ntoa(__worker_adr.sin_addr), __worker_sock, worker_cnt);
                log_file(msg);
                printf("%s", msg);

                sprintf(msg, "Authentication successful\n");
                log_file(msg);
                write(__worker_sock, msg, strlen(msg));

                break;
            }
        }
    }

    // 디바이스 인증 실패
    if (i == WORKER_MAX_CLNT) {
        sprintf(msg, "[%s] Authentication Error!\n", pArray[0]);
        write(__worker_sock, msg, strlen(msg));
        log_file(msg);
        shutdown(__worker_sock, SHUT_WR);
    }

    printf("%d\n", machine_type);
    return machine_type;
}

// Publisher 스레드: 컨베이어
// 스레드 풀에서 호출
// 컨베이어 파일 디스크립터를 파라미터로 받음
void *conveyer_handler(void *__arg) {
    //char buffer[BUF_SIZE];
    ssize_t nbytes;
    worker_task_t task;

    // 컨베이어 파일 디스크립터 추출
    // 워커 파일 디스크립터
    int worker_fd;
    char buffer[BUF_SIZE];

    mqd_t mq = mq_open(TOPIC_NAME, O_RDWR | O_NONBLOCK);
    if (mq == (mqd_t)-1) {
        perror("Failed to open message queue in conveyer_handler");
        return NULL;
    }

    while (1) {
        worker_fd = *(int*)__arg;
            if (worker_fd < 0) {
                perror("accept failed");
                close(worker_fd);
                exit(EXIT_FAILURE);
                continue;
            }

            // 컨베이어로부터 데이터 수신
            // 수신할 때까지 대기
            nbytes = read(worker_fd, buffer, BUF_SIZE - 1);
            if (nbytes <= 0) {
                if (nbytes == 0) {
                    fputs("Conveyer disconnected.\n", stdout);
                } else {
                    perror("Read error from Conveyer");
                }
                close(worker_fd);
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
            //task = {worker_fd, MACHINE_TYPE_ROBOT, detect_flag, color};
            task.worker_fd = worker_fd, task.machine_type = MACHINE_TYPE_ROBOT, task.detect_flag = detect_flag, task.color = color;
            printf("%d, %d, %d, %d\n", task.worker_fd, task.machine_type, task.detect_flag, task.color);
            // 메시지 큐에 데이터를 전달할 때, 컨베이어에 대한 파일 디스크립터도 같이 전달
            if (mq_send(mq, (const char *)&task, sizeof(worker_task_t), 0) == -1) {
                perror("Failed to send task to message queue");
                break;
            } else {
                printf("Conveyer thread is done sucessfully.\n");
                if (write(worker_fd, "receive successfully", strlen("receive successfully")) == -1) {
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

// Subscriber 스레드: 로봇
// 작업 처리 함수
// 스레드 풀에서 호출
void *robot_handler(void *__arg) {
    // 로봇 파일 디스크립터 추출
    // 워커 파일 디스크립터
    int worker_fd;
    char buffer[BUF_SIZE];
    PDU pdu;

    mqd_t mq = mq_open(TOPIC_NAME, O_RDONLY);
    if (mq == -1) {
        perror("Worker failed to open message queue");
        return NULL;
    }

    worker_task_t task;

    while (1) {
        worker_fd = *(int*)__arg;
        if (worker_fd < 0) {
            perror("accept failed");
            close(worker_fd);
            exit(EXIT_FAILURE);
        }

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

        printf("Robot thread is received sucessfully.\n");
        printf("%d, %d, %d, %d\n", task.worker_fd, task.machine_type, task.detect_flag, task.color);
        printf("Robot thread' machine type is robot.\n");
        // 흐름 제어 필요
        // 탐지 플래그가 TRUE인 경우에만 데이터 전송
        printf("detect_flag: %d\n", task.detect_flag);
        if (task.detect_flag == TRUE && worker_fd > 0) {
            // 처음에는 바로 데이터 전송
            if(isFirst) {
                send_command(worker_fd, task.detect_flag, task.color);
                printf("It is first.\n");
                isFirst = FALSE;
                continue;
            } else {
                int cnt = 0;
                ssize_t received_size = read(worker_fd, buffer, BUF_SIZE);
                if (received_size <= 0) {
                    perror("read failed");
                    cnt++;
                    continue;
                }
                deserialize((uint8_t *)buffer, &pdu);
                printf("message receive successfully from Robot1: %s", pdu.rcp_header.msg_type);

                // 두 번째 부터는 DONE 메시지를 받았을 경우에만 데이터 전송
                if(!strcmp(pdu.rcp_header.msg_type, MSG_TYPE_READY)) {
                    send_command(worker_fd, task.detect_flag, task.color);
                    printf("It is not first.\n");
                }
            }
        } else {
            printf("Flag is 0 or worker of robot is not connected\n");
        }
    }

    mq_close(mq);
    return NULL;
}

void* rct_brocker_handler(void *__arg) {
    // socket, epoll, thread, file descriptor
    int manager_fd, epfd, nfds, worker_fd;
    struct sockaddr_in manager_addr, worker_addr;
    socklen_t worker_addr_len;
    struct epoll_event ev, events[MAX_EVENTS];
    int str_len;

    char device_info[(WORKER_ID_SIZE*2)+3];

    uint8_t machine_type, conveyer_id, robot_id;

    (void)__arg;

    // Set accounts of device
    init_device_info();

    // 메시지 큐 생성
    mqd_t mq = mq_open(TOPIC_NAME, O_CREAT | O_RDWR, 0644, &mq_attr);
    if (mq == -1) {
        perror("Failed to create message queue");
        exit(EXIT_FAILURE);
    }

    // 매니저 소켓 생성
    if ((manager_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(manager_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    manager_addr.sin_family = AF_INET;
    manager_addr.sin_addr.s_addr = INADDR_ANY;
    manager_addr.sin_port = htons(PORT);

    if (bind(manager_fd, (struct sockaddr *)&manager_addr, sizeof(manager_addr)) == -1) {
        perror("Bind failed");
        close(manager_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(manager_fd, 5) == -1) {
        perror("Listen failed");
        close(manager_fd);
        exit(EXIT_FAILURE);
    }

    if ((epfd = epoll_create1(0)) == -1) {
        perror("Epoll creation failed");
        close(manager_fd);
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = manager_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, manager_fd, &ev) == -1) {
        perror("Epoll_ctl failed");
        close(manager_fd);
        exit(EXIT_FAILURE);
    }

    // 컨베이어 스레드 풀 메모리 할당
    pthread_t conveyer_thread_pool[THREAD_POOL_SIZE];

    // 로봇 스레드 풀 메모리 할당
    pthread_t robot_thread_pool[THREAD_POOL_SIZE];

    // 스레드의 파일디스크립터 저장에 필요한 뮤텍스 변수를 가져오기 위함
    pthread_mutex_t *__fd_data_mutx = get_fd_data_mutex();

    // 스레드 풀 제어에 필요한 뮤텍스 변수를 가져오기 위함
    pthread_mutex_t *__thread_pool_mutx = get_thread_pool_mutex();

    printf("manager is running on port %d...\n", PORT);

    while (1) {
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("Epoll_wait failed");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            // 매니저 소켓에 이벤트가 발생한 경우
            pthread_mutex_lock(__fd_data_mutx);
            if (events[i].data.fd == manager_fd) {
                // 새 워커 연결
                worker_addr_len = sizeof(worker_addr);
                worker_fd = accept(manager_fd, (struct sockaddr *)&worker_addr, &worker_addr_len);
                if (worker_fd == -1) {
                    perror("Accept failed");
                    continue;
                }
                printf("New connection accepted: FD = %d\n", worker_fd);
                pthread_mutex_unlock(__fd_data_mutx);

                // strtok를 통해 접속한 워커가 컨베이어인지 로봇인지 구분해야 함
                // 여기에서 컨베이어인지 로봇인지 구분하는 로직을 구현
                memset(device_info, 0, sizeof(device_info));
                str_len = read(worker_fd, device_info, sizeof(device_info));
                device_info[str_len] = '\0';
                machine_type = auth_worker_connect(worker_addr, worker_fd, device_info);


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
                    pthread_mutex_lock(__thread_pool_mutx);
                    if (fd_data.conveyer_fd[conveyer_id] == 0) {
                        fd_data.conveyer_fd[conveyer_id] = worker_fd;
                        // 원래는 컨베이어 측에서 conveyer_id 필드에 해당 컨베이어의 넘버를 기입 후 전송해야 함
                        if (pthread_create(&conveyer_thread_pool[conveyer_id], NULL, conveyer_handler, &fd_data.conveyer_fd[conveyer_id]) != 0) {
                            perror("Thread creation failed");
                            exit(EXIT_FAILURE);
                        }
                        printf("Conveyer 1 is connected: FD = %d\n", worker_fd);
                    } else
                        printf("Conveyer 1 is already connected: FD = %d\n", worker_fd);
                    pthread_mutex_unlock(__thread_pool_mutx);
                    
                // 해당 id의 로봇이 기존에 연결되어 있지 않은 경우
                // 로봇 스레드 생성
                } else if(machine_type == MACHINE_TYPE_ROBOT) {
                    pthread_mutex_lock(__thread_pool_mutx);
                    if (fd_data.robot_fd[robot_id] == 0) {
                        fd_data.robot_fd[robot_id] = worker_fd;
                        // 원래는 로봇 측에서 robot_id 필드에 해당 컨베이어의 넘버를 기입 후 전송해야 함
                        if (pthread_create(&robot_thread_pool[robot_id], NULL, robot_handler, &fd_data.robot_fd[robot_id]) != 0) {
                            perror("Thread creation failed");
                            exit(EXIT_FAILURE);
                        }
                        printf("Robot 1 is connected: FD = %d\n", worker_fd);
                    } else
                        printf("Robot 1 is already connected: FD = %d\n", worker_fd);
                    pthread_mutex_unlock(__thread_pool_mutx);
                // 이미 연결되어 있는 경우
                } else {
                    printf("Too many workers. Closing connection: FD = %d\n", worker_fd);
                    close(worker_fd);
                    continue;
                }

                ev.events = EPOLLIN;
                ev.data.fd = worker_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, worker_fd, &ev);

            // 워커 소켓에 이벤트가 발생한 경우
            // 스레드에서 처리
            } else {
                pthread_mutex_lock(__thread_pool_mutx);
                for(int j = 0; j < THREAD_POOL_SIZE; j++) {
                    pthread_join(conveyer_thread_pool[j], NULL);
                    pthread_join(robot_thread_pool[j], NULL);
                }
                pthread_mutex_unlock(__thread_pool_mutx);
            }
        }
    }

    mq_unlink(TOPIC_NAME);
    for(int i = 0; THREAD_POOL_SIZE; i++) {
        close(fd_data.conveyer_fd[i]);
        close(fd_data.robot_fd[i]);
    }
    close(manager_fd);
    close(epfd);
}