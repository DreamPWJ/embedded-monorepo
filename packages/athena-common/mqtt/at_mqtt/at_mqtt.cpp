#include "at_mqtt.h"
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <json_utils.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include <chip_info.h>
#include <pwm.h>
#include <ground_feeling.h>
#include <common_utils.h>
#include <device_info.h>
#include <ota.h>
#include <mcu_nvs.h>

using namespace std;

/**
* @author 潘维吉
* @date 2022/9/13 15:31
* @description AT指令编写MQTT消息队列遥测传输协议
* 参考文章： https://www.emqx.com/zh/blog/iot-protocols-mqtt-coap-lwm2m
* https://github.com/elementzonline/Arduino-Sample-Codes/tree/master/SIM7600
* https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
*/

#define USE_MULTI_CORE 0 // 是否使用多核 根据芯片决定
#define DEBUG true

#define PIN_RX 19
#define PIN_TX 18
SoftwareSerial myMqttSerial(PIN_RX, PIN_TX);

String atMqttName = "esp32-mcu-client"; // mqtt客户端名称

const char *at_mqtt_broker = "119.188.90.222"; // 设置MQTT的IP或域名
const char *at_topics = "ESP32/common"; // 设置MQTT的订阅主题
const char *at_mqtt_username = "admin";   // 设置MQTT服务器用户名和密码
const char *at_mqtt_password = "emqx@2022";
const int at_mqtt_port = 1883;

/**
 *
 * 初始化MQTT客户端
 */
void init_at_mqtt() {
    Serial.println("初始化MQTT客户端AT指令");

    myMqttSerial.begin(115200, SWSERIAL_8N1);
    if (!myMqttSerial) { // If the object did not initialize, then its configuration is invalid
        Serial.println("Invalid SoftwareSerial pin configuration, check config");
        while (1) { // Don't continue with invalid configuration
            Serial.print(".");
            delay(1000);
        }
    }

    String client_id = atMqttName + "-";
    string chip_id = to_string(get_chip_mac());
    client_id += chip_id.c_str();   //  String(random(0xffff),HEX); // String(WiFi.macAddress());
    send_mqtt_at_command("AT+CEREG?\r\n", 3000, DEBUG); // 判断附着网络 参数1或5标识附着正常
    delay(3000);

    // 设置MQTT连接所需要的的参数 不同的调制解调器模组需要适配不同的AT指令  参考文章: https://aithinker.blog.csdn.net/article/details/127100435?spm=1001.2014.3001.5502
    //send_mqtt_at_command("AT+ECMTCFG=\042keepalive\042,0,120\r\n", 6000, DEBUG); // 配置心跳时间
    //send_mqtt_at_command("AT+ECMTCFG=\042timeout\042,0,20\r\n", 6000, DEBUG); // 配置数据包的发送超时时间（单位：s，范围：1-60，默认10s）

    send_mqtt_at_command("AT+ECMTOPEN=0,\042" + String(at_mqtt_broker) + "\042," + at_mqtt_port + "\r\n", 10000,
                         DEBUG);  // GSM无法连接局域网, 因为NB本身就是低功耗广域网
/*  myMqttSerial.printf("AT+ECMTOPEN=0,\042%s\042,%d\r\n", at_mqtt_broker,
                        at_mqtt_port);  // GSM无法连接局域网, 因为NB本身就是低功耗广域网 */
    send_mqtt_at_command(
            "AT+ECMTCONN=0,\042" + client_id + "\042,\042" + at_mqtt_username + "\042,\042" + at_mqtt_password +
            "\042\r\n", 30000, DEBUG, "+ECMTCONN: 0,0,0");
    Serial.println("MQTT Broker 连接: " + client_id);

    // 发布MQTT消息
    at_mqtt_publish(at_topics, "你好, MQTT服务器, 我是" + client_id + "单片机AT指令发布的初始化消息");
    delay(1000);
    // 订阅MQTT主题消息
    // myMqttSerial.printf("AT+ECMTSUB=0,1,\"%s\",2\r\n", at_topics);
    std::string topic_device = "ESP32/" + to_string(get_chip_mac()); // .c_str 是 string 转 const char*
    at_mqtt_subscribe(topic_device.c_str()); // 设备单独的主题订阅
    delay(1000);
    at_mqtt_subscribe("ESP32/system"); // 系统相关主题订阅

#if !USE_MULTI_CORE
    // MQTT订阅消息回调
    const char *params = NULL;
    xTaskCreate(
            at_mqtt_callback,  /* Task function. */
            "at_mqtt_callback", /* String with name of task. */
            1024 * 16,      /* Stack size in bytes. */
            (void *) params,      /* Parameter passed as input of the task */
            0,         /* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) */
            NULL);     /* Task handle. */
#else
    //最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1,或者 tskNO_AFFINITY 允许任务在两者上运行.
    xTaskCreatePinnedToCore(at_mqtt_callback, "at_mqtt_callback", 8192, NULL, 2, NULL, 0);
#endif

    // 外部中断机制  MQTT订阅消息回调
    // at_attach_mqtt_callback();

}

/**
 * 发送AT指令
 */
String send_mqtt_at_command(String command, const int timeout, boolean debug, String successResult) {
    String response = "";
    myMqttSerial.print(command);
    long int time = millis();
    while ((time + timeout) > millis()) {
        while (myMqttSerial.available()) {
            char c = myMqttSerial.read();
            response += c;
        }
        if (response.indexOf(successResult) != -1) { // 获取到成功结果 退出循环
            break;
        }
    }
    if (debug) {
        Serial.println(command + "AT指令响应数据: " + response);
    }
    return response;
}

/**
 * MQTT订阅消息回调 中断机制
 */
void at_attach_mqtt_callback() {
    // 使用中断机制 您无需不断检查引脚的当前值。使用中断，当检测到更改时，会触发事件（调用函数) 无需循环检测
    // //将中断触发引脚 设置为INPUT_PULLUP（输入上拉）模式
    //pinMode(PIN_RX, INPUT_PULLUP);
    // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
    //attachInterrupt(digitalPinToInterrupt(PIN_RX), at_mqtt_callback, FALLING);
}

/**
 * MQTT发送消息
 */
void at_mqtt_publish(String topic, String msg) {
    // QoS（服务质量）:  0 - 最多分发一次  1 - 至少分发一次  2 - 只分发一次 (保证消息到达并无重复消息) 随着QoS等级提升，消耗也会提升，需要根据场景灵活选择
    myMqttSerial.printf(
            "AT+ECMTPUB=0,1,2,0,\042%s\042,\042%s\042\r\n", topic.c_str(), msg.c_str());
    delay(10);
}

/**
 * MQTT订阅消息
 */
void at_mqtt_subscribe(String topic) {
    myMqttSerial.printf("AT+ECMTSUB=0,1,\"%s\",2\r\n", topic.c_str());
}

/**
 * 取消MQTT主题订阅
 */
void at_mqtt_unsubscribe(String topic) {
    myMqttSerial.printf("AT+ECMTUNS=0,1,\"%s\"\r\n", topic.c_str());
}

/**
 * MQTT断开连接
 */
void at_mqtt_disconnect() {
    myMqttSerial.printf("AT+ECMTDISC=0\\r\\n\r\n");
}

/**
 * 检测重连MQTT服务
 */
void at_mqtt_reconnect(String incomingByte) {
    // 报告链路层状态 当 MQTT 链路层状态发生变化时，将上报此URC
    // +ECMTSTAT: <tcpconnectID>,<err_code> 1 连接已关闭或由对等方重置
    String flag = "ECMTSTAT";
    if (incomingByte.indexOf(flag) != -1) {
        Serial.println("AT指令重连MQTT服务");
        init_at_mqtt(); // 重连MQTT服务
    }
}

/**
 * MQTT订阅消息回调
 */
void at_mqtt_callback(void *pvParameters) {
    Serial.println("AT指令MQTT订阅接收的消息: ");
    // MQTT服务订阅返回AT指令数据
    /* +ECMTRECV: 0,0,"ESP32/common",{
            "command": "upgrade"
    }*/
    String flag = "ECMTRECV"; // 并发情况下 串口可能返回多条数据
    int isHasData = myMqttSerial.available() > 0 ? 1 : 0;
    while (1) {
        Serial.println("------------------------------------");
        // Serial.println(myMqttSerial.available());
/*        if (myMqttSerial.available() > 0) { // 串口缓冲区有数据
            Serial.println("因为NB-IOT窄带宽蜂窝网络为半双工 导致MQTT消息发布和订阅不能同时 此处做延迟处理");
            delay(200);
        }*/
        String incomingByte;
        incomingByte = myMqttSerial.readString();
        // Serial.println(incomingByte);

        if (incomingByte.indexOf(flag) != -1) {
            int startIndex = incomingByte.indexOf(flag);
            String start = incomingByte.substring(startIndex);
            int endIndex = start.indexOf("}"); //  发送JSON数据的换行 会导致后缀丢失
            String end = start.substring(0, endIndex + 1);
            String data = end.substring(end.lastIndexOf("{"), end.length());
            vector<string> dataArray = split(incomingByte.c_str(), ",");
            String topic = dataArray[2].c_str();
            // String data = dataArray[3].c_str(); // JSON结构体可能有分隔符 导致分割不正确
            Serial.printf("AT指令MQTT订阅主题: %s\n", topic.c_str());
            Serial.println(data);

            if (!data.isEmpty()) {
                DynamicJsonDocument json = string_to_json(data);
                // 获取MQTT订阅消息后执行任务
                do_at_mqtt_subscribe(json, topic);
            }
        }

        // 检测MQTT服务状态 如果失效自动重连
        at_mqtt_reconnect(incomingByte);

        delay(10);
    }
}

/**
 * 多线程MQTT任务
 */
void x_at_task_mqtt(void *pvParameters) {
    while (1) {
        // Serial.println("多线程MQTT任务, 心跳检测...");
        int deviceStatus = get_pwm_status(); // 设备电机状态
        int parkingStatus = ground_feeling_status(); // 是否有车
        // String networkRSSI = get_nvs("network_rssi"); // 是否有车
        // float electricityValue = get_electricity(); // 电量值

        // 发送心跳消息
        string jsonData =
                "{\"command\":\"heartbeat\",\"deviceCode\":\"" + to_string(get_chip_mac()) + "\",\"deviceStatus\":\"" +
                to_string(deviceStatus) + "\",\"parkingStatus\":\"" + to_string(parkingStatus) + "\"}";
        at_mqtt_publish(at_topics, jsonData.c_str()); // 我是AT指令 MQTT心跳发的消息
        delay(1000 * 10); // 多久执行一次 毫秒
    }
}

/**
 * MQTT心跳服务
 */
void at_mqtt_heart_beat() {
#if !USE_MULTI_CORE
    const char *params = NULL;
    xTaskCreate(
            x_at_task_mqtt,  /* Task function. */
            "x_at_task_mqtt", /* String with name of task. */
            8192,      /* Stack size in bytes. */
            (void *) params,      /* Parameter passed as input of the task */
            8,         /* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) */
            NULL);     /* Task handle. */
#else
    //最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1,或者 tskNO_AFFINITY 允许任务在两者上运行.
    xTaskCreatePinnedToCore(x_at_task_mqtt, "x_at_task_mqtt", 8192, NULL, 8, NULL, 0);
#endif
}

/**
 * 获取MQTT订阅消息后执行任务
 */
void do_at_mqtt_subscribe(DynamicJsonDocument json, String topic) {
    // MQTT订阅消息处理 控制电机马达逻辑 可能重复下发指令使用QoS控制  并设置心跳检测
    String command = json["command"].as<String>();
/*    int pin = 4;
    pinMode(pin, OUTPUT);
    *//* 开发板LED 闪动的实现 方便观测程序运行 *//*
    digitalWrite(pin, HIGH);
    delay(1000);
    digitalWrite(pin, LOW);
    delay(1000);*/

    Serial.println("指令类型: " + command);

    uint64_t chipId;
    try {
        chipId = get_chip_mac();
    } catch (exception &e) {
        cout << &e << endl;
    }

    // MQTT订阅消息处理
    if (topic.indexOf("ESP32/system") != -1) { // 针对主题做逻辑处理
        // MQTT通讯立刻执行OTA升级方法
        if (command == "upgrade") {
            /*    {
                    "command": "upgrade",
                    "firmwareUrl" : "http://archive-artifacts-pipeline.oss-cn-shanghai.aliyuncs.com/iot/ground-lock/prod/firmware.bin",
                     "chipIds" : ""
                }*/
            Serial.println("MQTT通讯立刻执行OTA升级方法");
            String firmwareUrl = json["firmwareUrl"].as<String>();
            String chipIds = json["chipIds"].as<String>();  // 根据设备标识进行指定设备升级 为空全部升级 逗号分割
            vector<string> array = split(chipIds.c_str(), ",");
            bool isUpdateByDevice = false;
            if (std::find(array.begin(), array.end(), to_string(chipId)) != array.end()) {
                Serial.print("根据设备标识进行指定设备OTA升级: ");
                Serial.println(chipId);
                isUpdateByDevice = true;
            }

            if (chipIds == "null" || chipIds.isEmpty() || isUpdateByDevice) {
                do_firmware_upgrade("", "", firmwareUrl); // 主动触发升级
            }
        } else if (command == "restart") {
            esp_restart();
        }
    }

    if (command == "raise") { // 电机升起指令
        set_motor_up();
    } else if (command == "putdown") { // 电机下降指令
        set_motor_down();
    } else if (command == "query") { // MQTT主动查询指令
        int deviceStatus = get_pwm_status(); // 设备电机状态
        int parkingStatus = ground_feeling_status(); // 是否有车
        std:
        string jsonData = "{\"command\":\"query\",\"deviceCode\":\"" + to_string(chipId) + "\",\"deviceStatus\":\"" +
                          to_string(deviceStatus) + "\",\"parkingStatus\":\"" + to_string(parkingStatus) + "\"}";
        at_mqtt_publish(at_topics, jsonData.c_str());
    }

}
