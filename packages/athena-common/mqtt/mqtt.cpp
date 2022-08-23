#include "mqtt.h"
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WiFi.h>

/**
* @author 潘维吉
* @date 2022/7/29 15:51
* @description MQTT消息队列遥测传输协议
* 参考文档： https://www.emqx.com/zh/blog/esp32-connects-to-the-free-public-mqtt-broker
*/

// MQTT Broker  EMQX服务器
const char *mqtt_broker = "192.168.1.200";
const char *topic = "esp32/test"; // 需要在EMQX服务器设置
const char *mqtt_username = "admin";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);


void callback(char *topic, byte *payload, unsigned int length) {
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
    client.setCallback(callback);
    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
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