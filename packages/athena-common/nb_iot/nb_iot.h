#ifndef EMBEDDED_MONOREPO_NB_IOT_H
#define EMBEDDED_MONOREPO_NB_IOT_H
#include <Arduino.h>
/**
* @author 潘维吉
* @date 2022/8/22 14:44
* @description NB-IoT物联网网络协议
*/

void init_nb_iot();

void at_http_get();

void check_uart_data();

void x_task_check_uart_data();

String get_imei();

#endif
