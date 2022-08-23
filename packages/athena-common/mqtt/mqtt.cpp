#include "mqtt.h"
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

/**
* @author 潘维吉
* @date 2022/7/29 15:51
* @description MQTT消息队列遥测传输协议
* 参考文档： https://www.emqx.com/zh/blog/esp32-connects-to-the-free-public-mqtt-broker
*/

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "esp32/test";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int  mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

/**
 * 初始化MQTT协议
 */
void init_mqtt() {

}