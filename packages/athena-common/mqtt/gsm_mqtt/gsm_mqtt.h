#ifndef GROUND_LOCK_GSM_MQTT_H
#define GROUND_LOCK_GSM_MQTT_H
#include <Arduino.h>

/**
* @author 潘维吉
* @date 2022/9/8 14:44
* @description GSM网络类型的MQTT消息队列遥测传输协议
*/


boolean  init_gsm_mqtt();

void gsm_mqtt_loop();

#endif
