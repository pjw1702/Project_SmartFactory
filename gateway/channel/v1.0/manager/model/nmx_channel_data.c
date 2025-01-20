#include "nmx_channel_data.h"

// 뮤텍스 변수
static pthread_mutex_t device_mutx;
static pthread_mutex_t fd_data_mutx;
static pthread_mutex_t thread_pool_mutx;

// Getter 함수
pthread_mutex_t *get_device_mutex() {
    return &device_mutx;
}

pthread_mutex_t *get_fd_data_mutex() {
    return &fd_data_mutx;
}

pthread_mutex_t *get_thread_pool_mutex() {
    return &thread_pool_mutx;
}

// Setter 함수
void set_device_mutex(pthread_mutex_t *__new_mutex) {
    device_mutx = *__new_mutex;
}

void set_fd_data_mutex(pthread_mutex_t *__new_mutex) {
    fd_data_mutx = *__new_mutex;
}

void set_thread_pool_mutex(pthread_mutex_t *__new_mutex) {
    thread_pool_mutx = *__new_mutex;
}