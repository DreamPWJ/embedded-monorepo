#ifndef ESP32_DEMO_MQTT_H
#define ESP32_DEMO_MQTT_H

#include <Arduino.h>
#include <ArduinoJson.h>

/**
* @author 潘维吉
* @date 2022/7/29 15:51
* @description MQTT消息队列遥测传输协议
*/

void init_mqtt();

void mqtt_publish(String topic, String msg);

void mqtt_subscribe(String topic);

void mqtt_loop();

void mqtt_reconnect();

void mqtt_heart_beat();

void x_task_mqtt(void *pvParameters);

void do_mqtt_heart_beat();

void mqtt_callback(char *topic, byte *payload, unsigned int length);

void do_mqtt_subscribe(DynamicJsonDocument json, char *topic);

#endif
