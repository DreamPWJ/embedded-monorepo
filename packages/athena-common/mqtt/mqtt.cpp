#include "mqtt.h"
#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <pwm.h>
#include <chip_info.h>

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
    // Serial.print("MQTT消息到达主题: ");
    // Serial.println(topic);
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

    // 控制电机马达逻辑 可能重复下发指令  MQTT判断设备唯一码后处理 并设置心跳检测
    uint32_t chipId = get_chip_id();
    if (command == "raise") {
        set_motor_up();
    }
    if (command == "putdown") {
        set_motor_down();
    }
    if (command == "query") {
        int status = get_pwm_status();
        DynamicJsonDocument doc(1024);
        JsonObject object = doc.to<JsonObject>();
        object["command"] = "query";
        object["deviceCode"] = chipId;
        object["deviceStatus"] = status;
        client.publish(topic, "主动查询MCU状态信息");
        client.subscribe(topic);
    }

}

/**
 * 初始化MQTT协议
 */
void init_mqtt(String name) {
    Serial.println("初始化MQTT协议");
    // connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
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
    while (!client.connected()) {
        init_mqtt("esp32-mcu-client");
    }
}