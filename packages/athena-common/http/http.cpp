#include "http.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

/**
* @author 潘维吉
* @date 2022/7/20 14:52
* @description Http网络请求工具
* 文档地址： https://arduinojson.org/v6/how-to/use-arduinojson-with-httpclient/
*/

/* GET请求 */
void http_get(String url) {
    HTTPClient http;
    http.begin(url); //HTTP begin
    int httpCode = http.GET();
    if (httpCode > 0) {
        // httpCode will be negative on error
        Serial.printf("HTTP Get Code: %d\r\n", httpCode);
        if (httpCode == HTTP_CODE_OK) // 收到正确的内容
        {
            // Parse response
            //DynamicJsonDocument doc(2048);
            //deserializeJson(doc, http.getStream());

            // Read values
            //Serial.println(doc["time"].as<long>());

            String resBuff = http.getString();
            Serial.println(resBuff);
            digitalWrite(18, HIGH);
            delay(1000);
            digitalWrite(18, LOW);
            delay(1000);
        }
    }
    // Disconnect
    http.end();
}

/* POST请求 */
void http_post(String url, String data) {
    // ArduinoJson文档地址: https://github.com/bblanchon/ArduinoJson.git
    // Prepare JSON document
    char jsonData[] = "{\"msgtype\":\"text\",\"text\": {\"content\":\"我是ESP32嵌入式MCU单片机发送的消息!\"}}";
    DynamicJsonDocument doc(2048);
    // Serialize JSON document
    serializeJson(doc, jsonData);
    digitalWrite(18, HIGH);
    delay(1000);
    digitalWrite(18, LOW);
    delay(1000);
    WiFiClient client;  // or WiFiClientSecure for HTTPS
    HTTPClient http;

    // Send request
    http.begin(client, url);
    http.POST(jsonData);
    digitalWrite(18, HIGH);
    delay(2000);
    digitalWrite(18, LOW);
    delay(2000);
    // Read response
    Serial.print(http.getString());
    // Disconnect
    http.end();
}