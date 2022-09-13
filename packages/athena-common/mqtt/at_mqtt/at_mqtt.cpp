#include "at_mqtt.h"
#include <Arduino.h>
#include <SoftwareSerial.h>

/**
* @author 潘维吉
* @date 2022/9/13 15:31
* @description AT指令编写MQTT消息队列遥测传输协议
* 参考文章： https://blog.csdn.net/Boantong_/article/details/116376011
*/

#define PIN_RX 19
#define PIN_TX 7
SoftwareSerial myMqttSerial(PIN_RX, PIN_TX);

/**
 * 初始化MQTT客户端
 */
void init_at_mqtt(String name) {
    myMqttSerial.begin(9600);
    Serial.println("初始化MQTT客户端AT指令");
    // 设置MQTT连接所需要的的参数
    myMqttSerial.println("AT+ECMTCFG=0,1,120,0");
    delay(1000);
    myMqttSerial.println("AT+ECMTOPEN=0,\"192.168.1.200\",1883");
    delay(1000);
    myMqttSerial.println("AT+ECMTCONN=0,\"at-esp32-mcu-client-test\",\"admin\",\"public\"");
    delay(3000);
    // 发布MQTT消息
    myMqttSerial.println("AT+ECMTPUB=0,0,0,0,\"ESP32/common\",\"你好, MQTT服务 , 我是ESP32 单片机\",0");
    delay(1000);

    delay(100);
    String incomingByte;
    incomingByte = myMqttSerial.readString();
    Serial.println(incomingByte);

    // 订阅MQTT消息
    myMqttSerial.println("AT+ECMTSUB=0,0,\"ESP32/common\",0,0");
}