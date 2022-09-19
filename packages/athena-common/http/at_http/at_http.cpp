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
#define PIN_TX 18
SoftwareSerial myHttpSerial(PIN_RX, PIN_TX);

/**
 * Http请求GET方法
 */
DynamicJsonDocument at_http_get(String url, bool isResponseData) {
    Serial.println("HTTP请求GET方法AT指令");
    myHttpSerial.begin(9600);
    if (!myHttpSerial) { // If the object did not initialize, then its configuration is invalid
        Serial.println("Invalid SoftwareSerial pin configuration, check config");
        while (1) { // Don't continue with invalid configuration
            Serial.print(".");
            delay(1000);
        }
    }
    // NB-IoT的AT指令文档: https://docs.ai-thinker.com/_media/nb-iot/nb-iot%E7%B3%BB%E5%88%97%E6%A8%A1%E7%BB%84at%E6%8C%87%E4%BB%A4%E9%9B%86v1.0.pdf
    url.replace("http://", "");
    url.replace("https://", "");
    unsigned int endIndex = url.indexOf("/");
    String domain = url.substring(0, endIndex);
    String path = url.substring(url.indexOf("/"), url.length());
    int pathLength = path.length();
    delay(1000);
    /* myHttpSerial.printf("AT+ECDNS=\042%s\042\r\n", domain.c_str()); // DNS解析测试
     delay(1000);*/
    myHttpSerial.printf(
            "AT+HTTPCREATE=0,\042http://%s:80\042\r\n", domain.c_str()); // 创建实例 测试地址 如 http://httpbin.org/get
    delay(1000);
    myHttpSerial.printf("AT+HTTPCON=0\r\n"); // 连接服务器
    delay(1000);
    myHttpSerial.printf("AT+HTTPSEND=0,0,%d,\042%s\042\r\n", pathLength, path.c_str()); // Http请求

    // 获取缓冲区串口返回的数据
    if (isResponseData) {
        return get_http_uart_data();
    } else {
        return (const JsonDocument &) "";
    }
}

/**
 * 获取缓冲区串口返回的数据
 */
DynamicJsonDocument get_http_uart_data() {
    // Receiving MODEM Response
    // while (myHttpSerial.available() > 0) {
    // while (1) {
    // 等待数据返回结果
    Serial.println("HTTP获取串口缓冲区返回的数据");
    unsigned long tm = millis();
    DynamicJsonDocument json(2048);
    String flag = "HTTPRESPC"; // http请求数据前缀
    while (millis() - tm <= 600000) { // 多少秒超时 退出循环 myHttpSerial.available() &&
        // Serial.println(myHttpSerial.available());
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
            Serial.print("AT Message is: ");
            Serial.println(data);
            String jsonStr = hex_to_string(data.c_str()).c_str();
            // Serial.println(jsonStr);
            json = string_to_json(jsonStr);

/*          String new_version = json["version"].as<String>();
            String file_url = json["file"].as<String>();
            Serial.println(new_version);
            Serial.println(file_url);*/
            myHttpSerial.printf("AT+HTTPDESTROY=0\r\n"); // 关闭连接
            return json;

        }
        delay(10);
        if (!json.isNull()) {
            Serial.println("已获取到数据, 退出HTTP请求数据监听");
            break;
        }
    }
    myHttpSerial.printf("AT+HTTPDESTROY=0\r\n"); // 关闭连接
    return json;
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