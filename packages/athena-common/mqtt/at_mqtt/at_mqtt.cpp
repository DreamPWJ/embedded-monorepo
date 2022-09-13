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
    if (!myMqttSerial) { // If the object did not initialize, then its configuration is invalid
        Serial.println("Invalid SoftwareSerial pin configuration, check config");
        while (1) { // Don't continue with invalid configuration
            Serial.print(".");
            delay(1000);
        }
    }
    Serial.println("初始化MQTT客户端AT指令");
    myMqttSerial.write("AT+CEREG?\r\n"); // 判断附着网络 参数1或5标识附着正常
    delay(3000);
    // 设置MQTT连接所需要的的参数
    // myMqttSerial.write("AT+ECMTCFG=\042keepalive\042,120\r\n");
    delay(1000);
    myMqttSerial.write("AT+ECMTOPEN=0,\042119.188.90.222\042,1883\r\n");  // GSM无法连接局域网, 因为NB、4G等本身就是广域网
    delay(1000);
    myMqttSerial.write("AT+ECMTCONN=0,\042at-esp32-mcu-client-test\042,\042admin\042,\042public\042,0,0\r\n");
    delay(3000);
    // 发布MQTT消息
    myMqttSerial.write("AT+ECMTPUB=0,0,0,0,\042ESP32/common\042,\042你好, MQTT服务 , 我是ESP32 单片机\042,0\r\n");
    delay(1000);

    delay(100);
    String incomingByte;
    incomingByte = myMqttSerial.readString();
    Serial.println(incomingByte);

    // 订阅MQTT消息
    myMqttSerial.write("AT+ECMTSUB=0,0,\"ESP32/common\",0,0\r\n");
}