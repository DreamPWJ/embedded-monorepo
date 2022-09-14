#include "at_mqtt.h"
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <SoftwareSerial.h>
#include <json_utils.h>

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
    // myMqttSerial.write("AT+CEREG?\r\n"); // 判断附着网络 参数1或5标识附着正常
    delay(1000);
    // 设置MQTT连接所需要的的参数
    // myMqttSerial.write("AT+ECMTCFG=\042keepalive\042,120\r\n");
    delay(1000);
    myMqttSerial.write("AT+ECMTOPEN=0,\042119.188.90.222\042,1883\r\n");  // GSM无法连接局域网, 因为NB、4G等本身就是广域网
    delay(1000);
    myMqttSerial.write("AT+ECMTCONN=0,\042at-esp32-mcu-client-test\042,\042admin\042,\042public\042\r\n");
    delay(1000);

    // 发布MQTT消息
    myMqttSerial.write(
            "AT+ECMTPUB=0,0,0,0,\042ESP32/common\042,\042你好, MQTT服务器 , 我是ESP32单片机AT指令发布的消息\042\r\n");
    delay(1000);

    // 订阅MQTT消息
    myMqttSerial.write("AT+ECMTSUB=0,1,\"ESP32/common\",1\r\n");

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
 * MQTT订阅消息回调
 */
void at_mqtt_callback(void *pvParameters) {
    Serial.print("MQTT订阅接受的消息: ");
    String flag = "ECMTRECV"; /* +ECMTRECV: 0,0,"ESP32/common",{
            "command": "putdown"
    }*/
    while (1) {
        String incomingByte;
        incomingByte = myMqttSerial.readString();
        Serial.println(incomingByte);
        if (incomingByte.indexOf(flag) != -1) {
            int startIndex = incomingByte.indexOf(flag);
            String start = incomingByte.substring(startIndex);
            int endIndex = start.indexOf("}");
            String end = start.substring(0, endIndex + 1);
            // String topic = end.substring(end.indexOf(",\""), end.lastIndexOf(",\""));
            // Serial.println("MQTT订阅主题: " + topic);
            String data = end.substring(end.lastIndexOf("{"), end.length());
            Serial.println(data);

            DynamicJsonDocument json = string_to_json(data);
            String command = json["command"].as<String>();

            Serial.println(command);
        }
        delay(10);
    }
}