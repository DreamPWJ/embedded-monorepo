#include "http.h"
#include <Arduino.h>
#include <HTTPClient.h>

/**
* @author 潘维吉
* @date 2022/7/20 14:52
* @description Http网络请求工具
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
            String resBuff = http.getString();
            Serial.println(resBuff);
            digitalWrite(18, HIGH);
            delay(1000);
            digitalWrite(18, LOW);
            delay(1000);
        }
    }
}