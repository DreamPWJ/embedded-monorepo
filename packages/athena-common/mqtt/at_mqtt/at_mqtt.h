#ifndef EMBEDDED_MONOREPO_AT_MQTT_H
#define EMBEDDED_MONOREPO_AT_MQTT_H

#include <Arduino.h>
#include <ArduinoJson.h>
/**
* @author 潘维吉
* @date 2022/9/13 15:31
* @description AT指令编写MQTT消息队列遥测传输协议
*/

void init_at_mqtt();

String send_mqtt_at_command(String command, const int timeout, boolean isDebug, String successResult = "OK");

void at_interrupt_mqtt_callback();

void at_mqtt_publish(String topic, String msg);

void at_mqtt_subscribe(String topic);

void at_mqtt_unsubscribe(String topic);

void at_mqtt_disconnect();

void at_mqtt_callback(void *pvParameters); // void *pvParameters

void at_mqtt_reconnect(String incomingByte);

void at_mqtt_heart_beat();

void do_at_mqtt_heart_beat();

void do_at_mqtt_subscribe(DynamicJsonDocument json, String topic);

#endif
