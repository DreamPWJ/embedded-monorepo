#include "mqtt.h"
#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <pwm.h>
#include <chip_info.h>
#include <exception>
#include <iostream>
#include <string>

using namespace std;

/**
* @author 潘维吉
* @date 2022/7/29 15:51
* @description MQTT消息队列遥测传输协议
* 参考文档： https://www.emqx.com/zh/blog/esp32-connects-to-the-free-public-mqtt-broker
*/

// MQTT Broker  EMQX服务器
const char *mqtt_broker = "192.168.1.200"; // 设置MQTT的IP或域名
const char *topics = "esp32/test"; // 设置MQTT的订阅主题
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
    /*   Serial.print("MQTT消息到达主题: ");
       Serial.println(topic);*/
    Serial.print("MQTT订阅接受的消息: ");
    String payloadData = "";
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
        payloadData += (char) payload[i];

    }
    Serial.println();
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payloadData);
    String command = doc["command"].as<String>();
    // Serial.println(command);
    Serial.println("-----------------------");

    // MQTT订阅消息处理 控制电机马达逻辑 可能重复下发指令  MQTT判断设备唯一码后处理 并设置心跳检测
    uint32_t chipId;
    try {
        chipId = get_chip_id();
    } catch (exception &e) {
        cout << &e << endl;
    }

    if (command == "raise") {
        set_motor_up();
    }
    if (command == "putdown") {
        set_motor_down();
    }
    if (command == "query") {
        String deviceCode = doc["deviceCode"].as<String>();
        Serial.println(deviceCode);
        int status = get_pwm_status();
        /*    DynamicJsonDocument doc(1024);
              JsonObject object = doc.to<JsonObject>();
              object["command"] = "query";
              object["deviceCode"] = chipId;
              object["deviceStatus"] = status; */
        std:
        string jsonData = "{\"command\":\"query\",\"deviceCode\":\"" + to_string(chipId) + "\",\"deviceStatus\":\"" +
                          to_string(status) + "\"}";
        client.publish(topics, jsonData.c_str());
    }

}

/**
 * 初始化MQTT客户端
 */
void init_mqtt(String name) {
    Serial.println("初始化MQTT客户端");
    // connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setKeepAlive(90); // 保持连接多少秒
    client.setCallback(mqtt_callback);
    while (!client.connected()) {
        String client_id = name + "-";
        client_id += get_chip_id();   //  String(random(0xffff),HEX); // String(WiFi.macAddress());
        Serial.printf("客户端 %s 已连接到 MQTT 服务器 \n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("MQTT broker 已连接成功");
        } else {
            Serial.print("MQTT状态失败 ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    // publish and subscribe
    // client.publish(topics, "Hi EMQX I'm ESP32 ^^");
    client.subscribe(topics);
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
void mqtt_reconnect(String name) {
    while (!client.connected()) {
        init_mqtt(name);
    }
}