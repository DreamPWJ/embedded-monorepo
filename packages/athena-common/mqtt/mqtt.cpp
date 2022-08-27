#include "mqtt.h"
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

/**
* @author 潘维吉
* @date 2022/7/29 15:51
* @description MQTT消息队列遥测传输协议
* 参考文档： https://www.emqx.com/zh/blog/esp32-connects-to-the-free-public-mqtt-broker
*/

// MQTT Broker  EMQX服务器
const char *mqtt_broker = "192.168.1.200"; // 设置MQTT的IP或域名
const char *topic = "esp32/test"; // 设置MQTT的订阅主题
const char *mqtt_username = "admin";   // 设置MQTT服务器用户名和密码
const char *mqtt_password = "public";
const int mqtt_port = 1883;

// NB-IoT参考：https://github.com/radhyahmad/NB-IoT-SIM700-MQTT/blob/main/NB-IOT/src/main.cpp
WiFiClient espClient; // WiFi网络类型
PubSubClient client(espClient);

/**
 * MQTT接受的消息回调
 */
void mqtt_callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}

/**
 * 初始化MQTT协议
 */
void init_mqtt() {
    Serial.println("初始化MQTT协议");
    // connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(mqtt_callback);
    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(random(0xffff),HEX); // String(WiFi.macAddress())
        Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public emqx mqtt broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    // publish and subscribe
    client.publish(topic, "Hi EMQX I'm ESP32 ^^");
    client.subscribe(topic);
}

/**
 * MQTT协议监听
 */
void mqtt_loop() {
    client.loop();
}

/**
 * 重连MQTT服务
 */
void mqtt_reconnect() {
    while (!client.connected()){
        init_mqtt();
    }
}