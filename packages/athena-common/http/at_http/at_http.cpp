#include "at_http.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <hex_utils.h>
#include <json_utils.h>

/**
* @author 潘维吉
* @date 2022/9/14 9:03
* @description 基于AT指令的HTTP工具
*/

#define PIN_RX 19
#define PIN_TX 7
SoftwareSerial myHttpSerial(PIN_RX, PIN_TX);

/**
 * Http请求GET方法
 */
void at_http_get() {
    Serial.println("HTTP请求GET方法AT指令");
    myHttpSerial.begin(9600);
    if (!myHttpSerial) { // If the object did not initialize, then its configuration is invalid
        Serial.println("Invalid SoftwareSerial pin configuration, check config");
        while (1) { // Don't continue with invalid configuration
            Serial.print(".");
            delay(1000);
        }
    }
    // 安信可NB-IoT的AT指令文档: https://docs.ai-thinker.com/_media/nb-iot/nb-iot%E7%B3%BB%E5%88%97%E6%A8%A1%E7%BB%84at%E6%8C%87%E4%BB%A4%E9%9B%86v1.0.pdf
    delay(1000);
    myHttpSerial.write("AT+ECDNS=\042archive-artifacts-pipeline.oss-cn-shanghai.aliyuncs.com\042\r\n"); // DNS解析测试
    delay(1000);
    myHttpSerial.write(
            "AT+HTTPCREATE=0,\042http://archive-artifacts-pipeline.oss-cn-shanghai.aliyuncs.com:80\042\r\n"); // 创建实例
    delay(1000);
    myHttpSerial.write("AT+HTTPCON=0\r\n"); // 连接服务器
    delay(1000);
    String path = "/iot/ground-lock/prod/ground-lockota.json";
    myHttpSerial.write("AT+HTTPSEND=0,0,41,\042/iot/ground-lock/prod/ground-lockota.json\042\r\n"); // Http请求

    // 获取缓冲区串口返回的数据
    get_http_uart_data();
}

/**
 * 获取缓冲区串口返回的数据
 */
void get_http_uart_data() {
    // Receiving MODEM Response
    // while (myHttpSerial.available() > 0) {
    // while (1) {
    // 等待数据返回结果
    unsigned long tm = millis();
    String flag = "HTTPRESPC";
    while (millis() - tm <= 30000) { // 多少秒超时 退出循环
        String incomingByte;
        incomingByte = myHttpSerial.readString();
        Serial.println(incomingByte);
        // Serial.println(myHttpSerial.available());
        if (incomingByte.indexOf(flag) != -1) {
            int startIndex = incomingByte.indexOf(flag);
            String start = incomingByte.substring(startIndex);
            int endIndex = start.indexOf("\n");
            String end = start.substring(0, endIndex + 1);
            String data = end.substring(end.lastIndexOf(",") + 1, end.length());
            // Serial.print("AT Message is: ");
            // Serial.println(data);
            String jsonStr = hex_to_string(data.c_str()).c_str();
            Serial.println(jsonStr);
            DynamicJsonDocument json = string_to_json(jsonStr);

            String new_version = json["version"].as<String>();
            String file_url = json["file"].as<String>();
            Serial.println(new_version);
            Serial.println(file_url);
            break;
        }
    }
    // }
    // }
}

/**
 * 等待数据返回结果
 */
int wait_for_result(char *s, unsigned short timeout) {
    unsigned long tm = millis();
    //print Return from ESP
    while (millis() - tm <= timeout) {
        if (myHttpSerial.find(s)) { // match result
            Serial.println(s);
            return 1;
        }
    }
    // send timeout msg to console.
    Serial.print(F("TimedOutWaitingFor: "));
    Serial.println(s);
    return 0;
}