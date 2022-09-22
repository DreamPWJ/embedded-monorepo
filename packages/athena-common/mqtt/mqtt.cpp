#include "mqtt.h"

#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
//#include <TinyGsmClient.h>
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
* 参考文档： https://www.emqx.com/zh/blog/iot-protocols-mqtt-coap-lwm2m
* https://www.emqx.com/zh/blog/esp32-connects-to-the-free-public-mqtt-broker
* https://github.com/vshymanskyy/TinyGSM
*/

#define USE_MULTI_CORE 0 // 是否使用多核 根据芯片决定

String mqttName = "esp32-mcu-client"; // mqtt客户端名称
// MQTT Broker  EMQX服务器
const char *mqtt_broker = "192.168.1.200"; // 设置MQTT的IP或域名
const char *topics = "ESP32/common"; // 设置MQTT的订阅主题
const char *mqtt_username = "admin";   // 设置MQTT服务器用户名和密码
const char *mqtt_password = "public";
const int mqtt_port = 1883;

// NB-IoT参考：https://github.com/radhyahmad/NB-IoT-SIM700-MQTT/blob/main/NB-IOT/src/main.cpp
/*#define SerialAT Serial1
TinyGsm modem();
TinyGsmClient espClient(modem); // NB-IoT网络类型*/
WiFiClient espClient; // WiFi网络类型
PubSubClient client(espClient);


/**
 * 初始化MQTT客户端
 */
void init_mqtt() {
    Serial.println("初始化MQTT客户端");
    // connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setKeepAlive(90); // 保持连接多少秒
    client.setCallback(mqtt_callback);
    String client_id = mqttName + "-";
    string chip_id = to_string(get_chip_mac());
    while (!client.connected()) {
        client_id += chip_id.c_str();   //  String(random(0xffff),HEX); // String(WiFi.macAddress());
        Serial.printf("客户端 %s 已连接到 MQTT 服务器 \n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("MQTT Broker 已连接成功");
        } else {
            Serial.print("MQTT状态失败 ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    // 订阅与发布 publish and subscribe
    // QoS（服务质量）:  0 - 最多分发一次  1 - 至少分发一次  2 - 只分发一次 (保证消息到达并无重复消息) 随着QoS等级提升，消耗也会提升，需要根据场景灵活选择
    std::string topic_device = "ESP32/" + to_string(get_chip_mac()); // .c_str 是 string 转 const char*
    client.publish(topic_device.c_str(), " 你好, MQTT服务器 , 我是ESP32单片机发布的初始化消息 ");

    client.subscribe(topic_device.c_str());
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
        init_mqtt();
    }
}

/**
 * MQTT心跳服务
 */
void mqtt_heart_beat() {
#if !USE_MULTI_CORE

    const char *params = NULL;
    xTaskCreate(
            x_task_mqtt,  /* Task function. */
            "x_task_mqtt", /* String with name of task. */
            8192,      /* Stack size in bytes. */
            (void *) params,      /* Parameter passed as input of the task */
            3,         /* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) */
            NULL);     /* Task handle. */
#else
    //最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1,或者 tskNO_AFFINITY 允许任务在两者上运行.
    xTaskCreatePinnedToCore(x_task_mqtt, "x_task_mqtt", 8192, NULL, 5, NULL, 0);
#endif
}

/**
 * 多线程MQTT任务
 */
void x_task_mqtt(void *pvParameters) {
    while (1) {
        // Serial.println("多线程MQTT任务, 心跳检测...");
        mqtt_reconnect();
        // 发送心跳消息
        client.publish(topics, " 我是MQTT心跳发的消息 ");
        delay(60000); // 多久执行一次 毫秒
    }
}

/**
 * MQTT接收的消息回调
 */
void mqtt_callback(char *topic, byte *payload, unsigned int length) {
    /*   Serial.print("MQTT订阅主题: ");
       Serial.println(topic); */
    Serial.println("MQTT订阅接收的消息: ");
    String payloadData = "";
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
        payloadData += (char) payload[i];

    }
    Serial.println();
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payloadData);

    // 获取MQTT订阅消息后执行任务
    do_mqtt_subscribe(doc, topic);
}


/**
 * 获取MQTT订阅消息后执行任务
 */
void do_mqtt_subscribe(DynamicJsonDocument json, char *topic) {
    Serial.printf("MQTT订阅主题: %s\n", topic);
    if (String(topic) == "") { // 针对主题做逻辑处理

    }
    String command = json["command"].as<String>();
    // Serial.println("指令类型: " + command);
    Serial.println("-----------------------");

    // MQTT订阅消息处理 控制电机马达逻辑 可能重复下发指令  MQTT判断设备唯一码后处理 并设置心跳检测
    uint32_t chipId;
    try {
        chipId = get_chip_mac();
    } catch (exception &e) {
        cout << &e << endl;
    }

    if (command == "raise") {
        set_motor_up();
    } else if (command == "putdown") {
        set_motor_down();
    } else if (command == "query") {
        int status = get_pwm_status();
        std:
        string jsonData = "{\"command\":\"query\",\"deviceCode\":\"" + to_string(chipId) + "\",\"deviceStatus\":\"" +
                          to_string(status) + "\"}";
        client.publish(topics, jsonData.c_str());
    }

}
