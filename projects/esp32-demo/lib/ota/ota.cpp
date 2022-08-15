#include "ota.h"
#include <Arduino.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cJSON.h>
#include "esp_system.h"
#include "esp_http_client.h"
#include <esp_https_ota.h>
#include <esp_crt_bundle.h>
#include <WiFiType.h>
#include <WiFi.h>
#include <http.h>
#include "HttpsOTAUpdate.h"

/**
* @author 潘维吉
* @date 2022/7/29 15:56
* @description 固件OTA空中升级
* 参考文档：https://blog.51cto.com/u_15284384/3054914
* https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_https_ota.html
* https://github.com/espressif/arduino-esp32/tree/master/libraries/Update/examples/HTTPS_OTA_Update
* https://github.com/espressif/esp-idf/tree/master/examples/system/ota
*/

// WebServer server(80);
// 固件文件地址 可存储到公有云OSS或者公共Git代码管理中用于访问  如果https证书有问题 可以使用http协议
static const char *CONFIG_FIRMWARE_UPGRADE_URL = "http://lanneng-epark-test.oss-cn-qingdao.aliyuncs.com/firmware.bin"; // state url of your firmware image
#define FIRMWARE_VERSION       0.1
#define UPDATE_JSON_URL        "http://lanneng-epark-test.oss-cn-qingdao.aliyuncs.com/ota.json" // 如果https证书有问题 可以使用http协议

// 提供 OTA 服务器证书以通过 HTTPS 进行身份验证server certificates  在platformio.ini内定义board_build.embed_txtfiles属性制定pem证书位置
// 生成pem证书文档: https://github.com/espressif/esp-idf/blob/master/examples/system/ota/README.md
// 证书生成命令(Windows系统在Git Bash执行): openssl req -x509 -newkey rsa:2048 -keyout ca_key.pem -out ca_cert.pem -days 3650 -nodes
extern const uint8_t server_cert_pem_start[] asm("_binary_server_certs_ca_cert_pem_start"); // key值为前后固定和pem全路径组合
extern const uint8_t server_cert_pem_end[] asm("_binary_server_certs_ca_cert_pem_end");

static HttpsOTAStatus_t otaStatus;

void HttpEvent(HttpEvent_t *event) {
    switch (event->event_id) {
        case HTTP_EVENT_ERROR:
            Serial.println("Http Event Error");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            Serial.println("Http Event On Connected");
            break;
        case HTTP_EVENT_HEADER_SENT:
            Serial.println("Http Event Header Sent");
            break;
        case HTTP_EVENT_ON_HEADER:
            Serial.printf("Http Event On Header, key=%s, value=%s\n", event->header_key, event->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            break;
        case HTTP_EVENT_ON_FINISH:
            Serial.println("Http Event On Finish");
            break;
        case HTTP_EVENT_DISCONNECTED:
            Serial.println("Http Event Disconnected");
            break;
    }
}

// receive buffer
char rcv_buffer[600];

// esp_http_client event handler
esp_err_t _http_event_handler(esp_http_client_event_t *evt) {

    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            break;
        case HTTP_EVENT_ON_CONNECTED:
            break;
        case HTTP_EVENT_HEADER_SENT:
            break;
        case HTTP_EVENT_ON_HEADER:
            break;
        case HTTP_EVENT_ON_DATA:
            if (!esp_http_client_is_chunked_response(evt->client)) {
                strncpy(rcv_buffer, (char *) evt->data, evt->data_len);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            break;
        case HTTP_EVENT_DISCONNECTED:
            break;
    }
    return ESP_OK;
}

// 参考: https://github.com/lucadentella/esp32-tutorial/blob/master/30_https_ota/main/main.c
// downloads every 30sec the json file with the latest firmware
void check_update_task(void *pvParameter) {
    printf("Looking for a new firmware...\n");
    //printf(server_cert_pem_start);

    // configure the esp_http_client
    esp_http_client_config_t config = {
            .url = UPDATE_JSON_URL,
            .cert_pem =  (char *) server_cert_pem_start,
            .timeout_ms = 8000,
            .event_handler = _http_event_handler,
            .keep_alive_enable = true,
            //.crt_bundle_attach =  esp_crt_bundle_attach,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    // downloading the json file
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        printf(rcv_buffer);
        // parse the json file
        cJSON *json = cJSON_Parse(rcv_buffer);
        if (json == NULL) printf("downloaded file is not a valid json, aborting...\n");
        else {
            cJSON *version = cJSON_GetObjectItemCaseSensitive(json, "version");
            cJSON *file = cJSON_GetObjectItemCaseSensitive(json, "file");

            // check the version
            if (!cJSON_IsNumber(version)) printf("unable to read new version, aborting...\n");
            else {
                double new_version = version->valuedouble;
                if (new_version > FIRMWARE_VERSION) {
                    printf("current firmware version (%.1f) is lower than the available one (%.1f), upgrading...\n",
                           FIRMWARE_VERSION, new_version);
                    if (cJSON_IsString(file) && (file->valuestring != NULL)) {
                        printf("downloading and installing new firmware (%s)...\n", file->valuestring);

                        esp_http_client_config_t ota_client_config = {
                                .url = file->valuestring,
                                //.crt_bundle_attach =  esp_crt_bundle_attach,
                                .cert_pem =  (char *) server_cert_pem_start,
                                .timeout_ms = 8000,
                                .keep_alive_enable = true,
                        };
                        esp_err_t ret = esp_https_ota(&ota_client_config);
                        if (ret == ESP_OK) {
                            Serial.println("执行OTA空中升级成功了, 准备重启单片机....");
                            esp_restart();
                        } else {
                            printf("OTA failed...\n");
                        }
                    } else printf("unable to read the new file name, aborting...\n");
                } else
                    printf("current firmware version (%.1f) is greater or equal to the available one (%.1f), nothing to do...\n",
                           FIRMWARE_VERSION, new_version);
            }
        }
    } else printf("unable to download the json file, aborting...\n");

    // cleanup
    esp_http_client_cleanup(client);

    printf("\n");
    vTaskDelay(30000 / portTICK_PERIOD_MS);
}

/**
 * 执行固件升级
 */
esp_err_t do_firmware_upgrade() {
    printf(CONFIG_FIRMWARE_UPGRADE_URL);
    http_get(UPDATE_JSON_URL);

    esp_http_client_config_t config = {
            .url = CONFIG_FIRMWARE_UPGRADE_URL,
            .cert_pem = (char *) server_cert_pem_start,
            .timeout_ms = 600000,
            //.crt_bundle_attach =  esp_crt_bundle_attach,
            .keep_alive_enable = true,
    };
/*    esp_https_ota_config_t ota_config = {
            .http_config = &config,
            .partial_http_download=true
    }; */
    esp_err_t ret = esp_https_ota(&config);
    if (ret == ESP_OK) {
        Serial.println("执行OTA空中升级成功了");
        esp_restart();
    } else {
        return ESP_FAIL;
    }
    return ESP_OK;
}

/**
 * 执行OTA空中升级
 */
void exec_ota() {
    Serial.println("开始执行OTA空中升级...");
    //if (WiFi.status() == WL_CONNECTED) {
    // do_firmware_upgrade();
    //}
    /* HttpsOTA.onHttpEvent(HttpEvent);
       HttpsOTA.begin(url, server_cert_pem_start);
       Serial.println("Please Wait it takes some time ..."); */

    // start the check update task
    xTaskCreate(&check_update_task, "check_update_task", 8192, NULL, 5, NULL);
}

/**
 * 检查OTA空中升级
 */
void check_ota() {
    otaStatus = HttpsOTA.status();
    if (otaStatus == HTTPS_OTA_SUCCESS) {
        Serial.println(
                "Firmware written successfully. To reboot device, call API ESP.restart() or PUSH restart button on device");
        // fflush(stdout);
        esp_restart();
    } else if (otaStatus == HTTPS_OTA_FAIL) {
        Serial.println("Firmware Upgrade Fail");
    }
    delay(1000);
}