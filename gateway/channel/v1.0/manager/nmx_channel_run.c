#include "rcp.h"
#include "model/nmx_channel_data.h"

int main() {
    pthread_t rct_brocker_handler_id;

    pthread_mutex_t __device_mutx;
    pthread_mutex_t __fd_data_mutx;  
    pthread_mutex_t __thread_pool_mutx; 

    // 뮤텍스 생성
    if(pthread_mutex_init(&__device_mutx, NULL))
        error_handling("mutex init error");
    if(pthread_mutex_init(&__fd_data_mutx, NULL))
        error_handling("mutex init error");
    if(pthread_mutex_init(&__thread_pool_mutx, NULL))
        error_handling("mutex init error");

    set_device_mutex(&__device_mutx);
    set_fd_data_mutex(&__fd_data_mutx);
    set_thread_pool_mutex(&__thread_pool_mutx);

    //  스레드 생성
    if (pthread_create(&rct_brocker_handler_id, NULL, rct_brocker_handler, NULL) != 0) {
        perror("Manager thread creation failed");
        exit(EXIT_FAILURE);
    }

    // 메인 스레드에서 매니저 스레드 종료 대기
    pthread_join(rct_brocker_handler_id, NULL);
    return 0;
}