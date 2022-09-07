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


/*esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                printf("%.*s", evt->data_len, (char*)evt->data);
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}*/

void esp_http() {
    esp_http_client_config_t config = {
            .url = "http://httpbin.org/redirect/2",
            //.event_handler = _http_event_handle,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        Serial.println(esp_http_client_get_status_code(client));
        Serial.println(esp_http_client_get_content_length(client));
    }
    esp_http_client_cleanup(client);
}
