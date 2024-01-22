#include "at_http.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <hex_utils.h>
#include <json_utils.h>

/**
* @author 潘维吉
* @date 2022/9/14 9:03
* @description 基于AT指令的HTTP工具
*/

#define IS_DEBUG false // 是否调试模式

/**
 * Http请求GET方法
 */
DynamicJsonDocument at_http_get(String url, bool isResponseData) {
    Serial.println("HTTP请求GET方法AT指令");

    // NB-IoT的AT指令文档: https://docs.ai-thinker.com/_media/nb-iot/nb-iot%E7%B3%BB%E5%88%97%E6%A8%A1%E7%BB%84at%E6%8C%87%E4%BB%A4%E9%9B%86v1.0.pdf
    url.replace("http://", "");
    url.replace("https://", "");
    unsigned int endIndex = url.indexOf("/");
    String domain = url.substring(0, endIndex);
    String path = url.substring(url.indexOf("/"), url.length());
    int pathLength = path.length();
    delay(1000);
    /* Serial1.printf("AT+ECDNS=\042%s\042\r\n", domain.c_str()); // DNS解析测试
     delay(1000);*/
    Serial1.printf(
            "AT+HTTPCREATE=0,\042http://%s:80\042\r\n", domain.c_str()); // 创建实例 测试地址 如 http://httpbin.org/get
    delay(1000);
    Serial1.printf("AT+HTTPCON=0\r\n"); // 连接服务器
    delay(1000);
    Serial1.printf("AT+HTTPSEND=0,0,%d,\042%s\042\r\n", pathLength, path.c_str()); // Http请求

    // 获取缓冲区串口返回的数据
    if (isResponseData) {
        return get_http_uart_data();
    } else {
        return (DynamicJsonDocument &&) (const JsonDocument &) "";
    }
}

/**
 * 发送AT指令
 */
String send_http_at_command(String command, const int timeout, boolean isDebug, String successResult) {
    String response = "";
    Serial1.print(command);
    long int time = millis();
    while ((time + timeout) > millis()) {
        while (Serial1.available()) {
            char c = Serial1.read();
            response += c;
        }
        if (response.indexOf(successResult) != -1) { // 获取到成功结果 退出循环
            break;
        }
        delay(10);
    }
    if (isDebug) {
        Serial.println(command + "AT指令响应数据: " + response);
    }
    return response;
}

/**
 * 获取缓冲区串口返回的数据
 */
DynamicJsonDocument get_http_uart_data() {
    // Receiving MODEM Response
    while (Serial1.available() > 0) {
        // while (1) {
        // 等待数据返回结果
        Serial.println("HTTP获取串口缓冲区返回的数据");
        unsigned long tm = millis();
        DynamicJsonDocument json(2048);
        String flag = "HTTPRESPC"; // http请求数据前缀
        while (millis() - tm <= 6000) { // 多少秒超时 退出循环 Serial1.available() &&
            // Serial.println(Serial1.available());
            String incomingByte;
            incomingByte = Serial1.readString();
            Serial.println(incomingByte);
            // Serial.println(Serial1.available());
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
                Serial1.printf("AT+HTTPDESTROY=0\r\n"); // 关闭连接
                return json;

            }
            delay(10);
            if (!json.isNull()) {
                Serial.println("已获取到数据, 退出HTTP请求数据监听");
                break;
            }
        }
        Serial1.printf("AT+HTTPDESTROY=0\r\n"); // 关闭连接
        return json;
        // }
    }
    return (DynamicJsonDocument &&) (const JsonDocument &) "";
}

/**
 * 等待数据返回结果
 */
int wait_for_result(char *s, unsigned short timeout) {
    unsigned long tm = millis();
    //print Return from ESP
    while (millis() - tm <= timeout) {
        if (Serial1.find(s)) { // match result
            Serial.println(s);
            return 1;
        }
    }
    // send timeout msg to console.
    Serial.print(F("TimedOutWaitingFor: "));
    Serial.println(s);
    return 0;
}