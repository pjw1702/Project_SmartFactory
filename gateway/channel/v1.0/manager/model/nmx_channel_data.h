#ifndef __MODEL_H__
#define __MODEL_H__

#include <pthread.h>

// Getter 함수
pthread_mutex_t *get_device_mutex();
pthread_mutex_t *get_fd_data_mutex();
pthread_mutex_t *get_thread_pool_mutex();

// Setter 함수
void set_device_mutex(pthread_mutex_t *__new_mutex);
void set_fd_data_mutex(pthread_mutex_t *__new_mutex);
void set_thread_pool_mutex(pthread_mutex_t *__new_mutex);

#endif