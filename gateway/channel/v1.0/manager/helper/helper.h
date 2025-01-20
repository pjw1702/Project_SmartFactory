#ifndef __HELPER_H__
#define __HELPER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include "../rcp.h"

size_t serialize(rct_instance *__src_rct_pdu, uint8_t *__dst_buffer);
void deserialize(const uint8_t *__src_buffer, rct_instance *__dst_rct_pdu);
uint16_t calculate_checksum(void *data, size_t size);
void send_confirm_ready_message(int __worker_fd);
void send_command(int __robot_fd, uint8_t __detect_flag, uint8_t __color);
void error_handling(char *__msg);
void log_file(char * __msg_contents);

#endif