#include "http.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

const static char *root_ca PROGMEM = "-----BEGIN CERTIFICATE-----\n" \
"MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/\n" \
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
"DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow\n" \
"PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD\n" \
"Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n" \
"AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O\n" \
"rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq\n" \
"OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b\n" \
"xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw\n" \
"7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD\n" \
"aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\n" \
"HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG\n" \
"SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69\n" \
"ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr\n" \
"AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz\n" \
"R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5\n" \
"JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo\n" \
"Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\n" \
"-----END CERTIFICATE-----\n";

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
        }
    }
    // Disconnect
    http.end();
}

/* POST请求 */
void http_post(String url, String data) {
    // ArduinoJson文档地址: https://github.com/bblanchon/ArduinoJson.git
    // Prepare JSON document
    DynamicJsonDocument doc(1024);
/*    JsonObject object = doc.to<JsonObject>();
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

    WiFiClientSecure client;  // WiFiClient client for HTTP or WiFiClientSecure for HTTPS
    client.setCACert(root_ca); // 开启https
    HTTPClient http;
    if (WiFi.status() == WL_CONNECTED) {
        // Send request
        http.begin(client, url);
        http.addHeader("Content-Type", "application/json; charset=UTF-8");
        int httpCode = http.POST(dataJSON);
        Serial.printf("HTTP POST Code: %d\r\n", httpCode);
        Serial.println("HTTP POST Msg: " + http.getString());
        if (httpCode == HTTP_CODE_OK) {
            // Read response
            Serial.print(http.getString());
        }
    }
    // Disconnect
    http.end();
}