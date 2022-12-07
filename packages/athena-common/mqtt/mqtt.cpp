#include "mqtt.h"

#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
//#include <TinyGsmClient.h>
#include <WiFi.h>
#include <pwm.h>
#include <chip_info.h>
#include <exception>
#include <vector>
#include <iostream>
#include <iostream>
#include <string>
#include <ota.h>
#include <ground_feeling.h>
#include <common_utils.h>
#include <mcu_nvs.h>
#include <device_info.h>

using namespace std;

/**
* @author 潘维吉
* @date 2022/7/29 15:51
* @description MQTT消息队列遥测传输协议
* 参考文档： https://www.emqx.com/zh/blog/iot-protocols-mqtt-coap-lwm2m
* https://www.emqx.com/zh/blog/esp32-connects-to-the-free-public-mqtt-broker
* https://github.com/vshymanskyy/TinyGSM
* https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
*/

#define USE_MULTI_CORE 0 // 是否使用多核 根据芯片决定

String mqttName = "esp32-mcu-client"; // mqtt客户端名称
// MQTT Broker  EMQX服务器
const char *mqtt_broker = "119.188.90.222"; // 设置MQTT的IP或域名
const char *topics = "ESP32/common"; // 设置MQTT的订阅主题
const char *mqtt_username = "admin";   // 设置MQTT服务器用户名
const char *mqtt_password = "emqx@2022"; // 设置MQTT服务器密码
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
    client.setKeepAlive(120); // 保持连接多少秒
    client.setCallback(mqtt_callback);
    String client_id = mqttName + "-";
    string chip_id = to_string(get_chip_mac());
    while (!client.connected()) {
        client_id += chip_id.c_str();   //  String(random(0xffff),HEX); // String(WiFi.macAddress());
        Serial.printf("客户端 %s 已连接到 MQTT 服务器 \n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("MQTT Broker 已连接成功");
        } else {
            Serial.print("MQTT连接失败 ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    // 订阅与发布 publish and subscribe
    std::string topic_device = "ESP32/" + to_string(get_chip_mac()); // .c_str 是 string 转 const char*
    DynamicJsonDocument doc(200);
    doc["type"] = "initMQTT";
    doc["msg"] = "你好, MQTT服务器, 我是" + client_id + "单片机发布的初始化消息";
    doc["version"] = get_nvs("version");
    String initStr;
    serializeJson(doc, initStr);
    client.publish(topics, initStr.c_str());
    delay(1000);
    client.subscribe(topic_device.c_str()); // 设备单独的主题订阅
    client.subscribe("ESP32/system");  // 系统相关主题订阅
}

/**
 * MQTT发送消息
 */
void mqtt_publish(String topic, String msg) {
    // 注意完善： 1. 并发队列控制 2. 发送失败重试机制
    // QoS（服务质量）:  0 - 最多分发一次  1 - 至少分发一次  2 - 只分发一次 (保证消息到达并无重复消息) 随着QoS等级提升，消耗也会提升，需要根据场景灵活选择
    client.publish(topic.c_str(), msg.c_str());
}

/**
 * MQTT订阅消息
 */
void mqtt_subscribe(String topic) {
    client.subscribe(topic.c_str());
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
            1024 * 8,      /* Stack size in bytes. */
            (void *) params,      /* Parameter passed as input of the task */
            6,         /* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) */
            NULL);     /* Task handle. */
#else
    //最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1,或者 tskNO_AFFINITY 允许任务在两者上运行.
    xTaskCreatePinnedToCore(x_task_mqtt, "x_task_mqtt", 1024 * 8, NULL, 5, NULL, 0);
#endif
}

/**
 * 多线程MQTT任务
 */
void x_task_mqtt(void *pvParameters) {
    while (1) {
        // Serial.println("多线程MQTT任务, 心跳检测...");
        do_mqtt_heart_beat();
        delay(1000 * 60); // 多久执行一次 毫秒
    }
}

/**
 * 执行MQTT心跳
 */
void do_mqtt_heart_beat() {
    mqtt_reconnect();
    int deviceStatus = get_pwm_status(); // 设备电机状态
    int parkingStatus = ground_feeling_status(); // 是否有车
    String firmwareVersion = get_nvs("version"); // 固件版本
    String networkSignal = get_nvs("network_signal"); // 信号质量
    vector<string> array = split(to_string(get_electricity()), "."); // 电量值
    String electricityValue = array[0].c_str();
    // 发送心跳消息
    string jsonData =
            "{\"command\":\"heartbeat\",\"deviceCode\":\"" + to_string(get_chip_mac()) + "\",\"deviceStatus\":\"" +
            to_string(deviceStatus) + "\",\"parkingStatus\":\"" + to_string(parkingStatus) +
            "\",\"firmwareVersion\":\"" + firmwareVersion.c_str() + "\"," +
            "\"electricity\":\"" + electricityValue.c_str() + "\"," +
            "\"networkSignal\":\"" + networkSignal.c_str() + "\"}";
    // 发送心跳消息
    client.publish(topics, jsonData.c_str());
}

/**
 * MQTT接收的消息回调
 */
void mqtt_callback(char *topic, byte *payload, unsigned int length) {
    Serial.printf("MQTT订阅主题: %s\n", topic);
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
    String command = json["command"].as<String>();
    // Serial.println("指令类型: " + command);
    // Serial.println("-----------------------");

    uint64_t chipId;
    try {
        chipId = get_chip_mac();
    } catch (exception &e) {
        cout << &e << endl;
    }

    // MQTT订阅消息处理
    if (String(topic) == "ESP32/system") { // 针对主题做逻辑处理
        String chipIds = json["chipIds"].as<String>();  // 根据设备标识进行指定设备升级 为空全部升级 逗号分割
        vector<string> array = split(chipIds.c_str(), ",");
        bool isUpdateByDevice = false;
        if (std::find(array.begin(), array.end(), to_string(chipId)) != array.end()) {
            Serial.print("根据设备标识进行指定设备OTA升级: ");
            Serial.println(chipId);
            isUpdateByDevice = true;
        }

        if (command == "upgrade") { // MQTT通讯立刻执行OTA升级
            /*    {
                    "command": "upgrade",
                    "firmwareUrl" : "http://archive-artifacts-pipeline.oss-cn-shanghai.aliyuncs.com/iot/ground-lock/prod/firmware.bin",
                     "chipIds" : ""
                }*/
            String firmwareUrl = json["firmwareUrl"].as<String>();
            if (chipIds == "null" || chipIds.isEmpty() || isUpdateByDevice) {
                Serial.println("MQTT通讯立刻执行OTA升级");
                do_firmware_upgrade("", "", firmwareUrl); // 主动触发升级
            }
        } else if (command == "restart") {  // 远程重启设备
            if (chipIds == "null" || chipIds.isEmpty() || isUpdateByDevice) {
                Serial.println("远程重启单片机设备...");
                esp_restart();
            }
        }
        return;
    }

    if (command == "heartbeat") { // 心跳指令
        do_mqtt_heart_beat();
    }
    if (command == "raise") {
        set_motor_up();
    }
    if (command == "putdown") {
        set_motor_down();
    }
    if (command == "query") {
        int status = get_pwm_status();
        std:
        string jsonData = "{\"command\":\"query\",\"deviceCode\":\"" + to_string(chipId) + "\",\"deviceStatus\":\"" +
                          to_string(status) + "\"}";
        client.publish(topics, jsonData.c_str());
    }

}
