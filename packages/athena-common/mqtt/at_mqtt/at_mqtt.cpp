#include "at_mqtt.h"
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <json_utils.h>
#include <iostream>
#include <string>
#include <chip_info.h>
#include <pwm.h>

using namespace std;


/**
* @author 潘维吉
* @date 2022/9/13 15:31
* @description AT指令编写MQTT消息队列遥测传输协议
* 参考文章： https://github.com/elementzonline/Arduino-Sample-Codes/tree/master/SIM7600
*/

#define USE_MULTI_CORE 0 // 是否使用多核 根据芯片决定

#define PIN_RX 19
#define PIN_TX 7
SoftwareSerial myMqttSerial(PIN_RX, PIN_TX);

String mqttName = "esp32-mcu-client"; // mqtt客户端名称

const char *mqtt_broker = "119.188.90.222"; // 设置MQTT的IP或域名
const char *topics = "ESP32/common"; // 设置MQTT的订阅主题
const char *mqtt_username = "admin";   // 设置MQTT服务器用户名和密码
const char *mqtt_password = "public";
const int mqtt_port = 1883;

/**
 *
 * 初始化MQTT客户端
 */
void init_at_mqtt() {
    myMqttSerial.begin(9600);
    if (!myMqttSerial) { // If the object did not initialize, then its configuration is invalid
        Serial.println("Invalid SoftwareSerial pin configuration, check config");
        while (1) { // Don't continue with invalid configuration
            Serial.print(".");
            delay(1000);
        }
    }
    Serial.println("初始化MQTT客户端AT指令");
    String client_id = mqttName + "-";
    client_id += get_chip_id();   //  String(random(0xffff),HEX); // String(WiFi.macAddress());
    // myMqttSerial.printf("AT+CEREG?\r\n"); // 判断附着网络 参数1或5标识附着正常
    // delay(1000);
    // 设置MQTT连接所需要的的参数
    // myMqttSerial.printf("AT+ECMTCFG=\042keepalive\042,120\r\n");
    delay(2000);
    myMqttSerial.printf("AT+ECMTOPEN=0,\042%s\042,%d\r\n", mqtt_broker, mqtt_port);  // GSM无法连接局域网, 因为NB、4G等本身就是广域网
    delay(1000);
    myMqttSerial.printf("AT+ECMTCONN=0,\042%s\042,\042%s\042,\042%s\042\r\n", client_id.c_str(), mqtt_username,
                        mqtt_password);
    delay(1000);
    Serial.println("MQTT Broker 已连接成功: " + client_id);
    // 发布MQTT消息
    myMqttSerial.printf(
            "AT+ECMTPUB=0,0,0,0,\042%s\042,\042你好, MQTT服务器 , 我是ESP32单片机AT指令发布的消息\042\r\n", topics);
    delay(1000);

    // 订阅MQTT消息
    // myMqttSerial.printf("AT+ECMTSUB=0,1,\"%s\",1\r\n", topics);
    std::string topic_device = "ESP32/" + to_string(get_chip_id()); // .c_str 是 string 转 const char*
    myMqttSerial.printf("AT+ECMTSUB=0,1,\"%s\",1\r\n", topic_device.c_str());
    delay(1000);

    // MQTT订阅消息回调
    const char *params = NULL;
    xTaskCreate(
            at_mqtt_callback,  /* Task function. */
            "at_mqtt_callback", /* String with name of task. */
            8192,      /* Stack size in bytes. */
            (void *) params,      /* Parameter passed as input of the task */
            2,         /* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) */
            NULL);     /* Task handle. */
}

/**
 * MQTT发送消息
 */
void at_mqtt_publish(String topic, String msg) {
    myMqttSerial.printf(
            "AT+ECMTPUB=0,0,0,0,\042%s\042,\042%s\042\r\n", topic.c_str(), msg.c_str());
}

/**
 * MQTT订阅消息
 */
void at_mqtt_subscribe(String topic) {
    myMqttSerial.printf("AT+ECMTSUB=0,1,\"%s\",1\r\n", topic.c_str());
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
    Serial.println("AT指令MQTT订阅接受的消息: ");
    // MQTT服务是否打开成功返回AT指令数据： +ECMTOPEN: 0,0
    // MQTT服务是否连接成功返回AT指令数据： +ECMTCONN: 0,0,0
    // MQTT服务是否发送成功返回AT指令数据： +ECMTPUB: 0,0,0
    // MQTT服务是否订阅成功返回AT指令数据： +ECMTSUB: 0,1,0,1

    // MQTT服务订阅返回AT指令数据
    String flag = "ECMTRECV"; /* +ECMTRECV: 0,0,"ESP32/common",{
            "command": "putdown"
    }*/
    while (1) {
        String incomingByte;
        incomingByte = myMqttSerial.readString();
        // Serial.println(incomingByte);

        // 检测MQTT服务状态 如果失效自动重连
        at_mqtt_reconnect(incomingByte);

        if (incomingByte.indexOf(flag) != -1) {
            int startIndex = incomingByte.indexOf(flag);
            String start = incomingByte.substring(startIndex);
            int endIndex = start.indexOf("}");
            String end = start.substring(0, endIndex + 1);
            // String topic = end.substring(end.indexOf(",\""), end.lastIndexOf(",\""));
            // Serial.println("MQTT订阅主题: " + topic);
            String data = end.substring(end.lastIndexOf("{"), end.length());
            // Serial.println(data);

            DynamicJsonDocument json = string_to_json(data);

            // 获取MQTT订阅消息后执行任务
            do_at_mqtt_subscribe(json);
        }
        delay(10);
    }
}

/**
 * 多线程MQTT任务
 */
void x_at_task_mqtt(void *pvParameters) {
    while (1) {
        // Serial.println("多线程MQTT任务, 心跳检测...");
        // 发送心跳消息
        at_mqtt_publish(topics, " 我是AT指令 MQTT心跳发的消息 ");
        delay(60000); // 多久执行一次 毫秒
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
    xTaskCreatePinnedToCore(x_at_task_mqtt, "x_at_task_mqtt", 8192, NULL, 3, NULL, 0);
#endif
}

/**
 * 获取MQTT订阅消息后执行任务
 */
void do_at_mqtt_subscribe(DynamicJsonDocument json) {
    // MQTT订阅消息处理 控制电机马达逻辑 可能重复下发指令使用QoS控制  并设置心跳检测
    String command = json["command"].as<String>();
    // Serial.println(command);
    uint32_t chipId;
    try {
        chipId = get_chip_id();
    } catch (exception &e) {
        cout << &e << endl;
    }

    if (command == "raise") { // 电机升起指令
        set_motor_up();
    }
    if (command == "putdown") { // 电机下降指令
        set_motor_down();
    }
    if (command == "query") { // MQTT主动查询指令
        int status = get_pwm_status();
        /*    DynamicJsonDocument doc(1024);
              JsonObject object = doc.to<JsonObject>();
              object["command"] = "query";
              object["deviceCode"] = chipId;
              object["deviceStatus"] = status; */
        std:
        string jsonData = "{\"command\":\"query\",\"deviceCode\":\"" + to_string(chipId) + "\",\"deviceStatus\":\"" +
                          to_string(status) + "\"}";
        at_mqtt_publish(topics, jsonData.c_str());
    }
}
