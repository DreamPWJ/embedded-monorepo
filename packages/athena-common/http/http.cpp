#define TINY_GSM_MODEM_SIM800

#include "http.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
// #include <TinyGsmClient.h>
#include <esp_http_client.h>

/**
* @author 潘维吉
* @date 2022/7/20 14:52
* @description Http网络请求工具
* 文档地址： https://arduinojson.org/v6/how-to/use-arduinojson-with-httpclient/
* https://github.com/vshymanskyy/TinyGSM/blob/master/examples/HttpClient/HttpClient.ino
*/


#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

/*#define SerialAT Serial1
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);*/


/**
 * GET请求
 */
DynamicJsonDocument http_get(String url) {
    HTTPClient http;
    DynamicJsonDocument doc(2048);
    http.begin(url); // HTTP begin
    int httpCode = http.GET();
    if (httpCode > 0) {
        // httpCode will be negative on error
        Serial.printf("HTTP Get Code: %d\r\n", httpCode);
        if (httpCode == HTTP_CODE_OK) // 收到正确的内容
        {
            // Parse response
            deserializeJson(doc, http.getStream());

            // Read values
            /*     Serial.println(doc["version"].as<double>());
                 Serial.println(doc["file"].as<String>()); */

            /*   String resBuff = http.getString();
                 Serial.println(resBuff); */
        }
    }
    // Disconnect
    http.end();
    return doc;
}

/**
 * POST请求
 */
void http_post(String url, String data) {
    // ArduinoJson文档地址: https://github.com/bblanchon/ArduinoJson.git
    // Prepare JSON document
    DynamicJsonDocument doc(1024);
/*  JsonObject object = doc.to<JsonObject>();
    JsonObject objectItem = doc.to<JsonObject>();
    object["msgtype"] = "text";
    objectItem["content"] = "蓝能科技：我是ESP32嵌入式MCU单片机发送的消息!";
    object["text"] = objectItem;*/
    // 钉钉通知注意要添加设置关键字
    char jsonData[] = "{\"msgtype\":\"text\",\"text\": {\"content\":\"蓝能科技：我是ESP32嵌入式MCU单片机发送的消息!\"}}";
    deserializeJson(doc, jsonData);
    String dataJSON;
    // Serialize JSON document
    serializeJsonPretty(doc, dataJSON);

    HTTPClient http;
    // Send request
    http.begin(url);
    http.addHeader("Content-Type", "application/json; charset=UTF-8");
    // if (WiFi.status() == WL_CONNECTED) {
    int httpCode = http.POST(dataJSON);
    Serial.printf("HTTP POST Code: %d\r\n", httpCode);
    Serial.println("HTTP POST Msg: " + http.getString());
    if (httpCode == HTTP_CODE_OK) {
        // Read response
        Serial.print(http.getString());
    }
    // }
    // Disconnect
    http.end();
}
