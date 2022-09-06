#ifndef ESP32_DEMO_MQTT_H
#define ESP32_DEMO_MQTT_H
#include <Arduino.h>
/**
* @author 潘维吉
* @date 2022/7/29 15:51
* @description MQTT消息队列遥测传输协议
*/

void init_mqtt(String name);

void mqtt_loop();

void mqtt_reconnect(String name);

void mqtt_heart_beat();

void x_task_mqtt(void *pvParameters);

#endif
