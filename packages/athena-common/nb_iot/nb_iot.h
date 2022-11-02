#ifndef EMBEDDED_MONOREPO_NB_IOT_H
#define EMBEDDED_MONOREPO_NB_IOT_H

#include <Arduino.h>

/**
* @author 潘维吉
* @date 2022/8/22 14:44
* @description NB-IoT物联网网络协议
*/

void init_nb_iot();

String send_at_command(String command, const int timeout, boolean isDebug, String successResult = "OK");

void at_command_response();

void nb_iot_heart_beat(void *pvParameters); // void *pvParameters

void restart_nb_iot();

void  hardware_restart_nb_iot();

String get_nb_ntp_time();

String get_imei();

#endif
