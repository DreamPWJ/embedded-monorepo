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
#include <HTTPClient.h>
#include "HttpsOTAUpdate.h"
#include <ArduinoJson.h>
#include <bits/stdc++.h>

using namespace std;

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
// static const char *CONFIG_FIRMWARE_UPGRADE_URL = "http://archive-artifacts-pipeline.oss-cn-shanghai.aliyuncs.com/iot/firmware.bin"; // state url of your firmware image
#define FIRMWARE_VERSION       "0.2.14"  // 版本号用于OTA升级和远程升级文件对比 判断是否有新版本 每次需要OTA的时候手动更改设置
#define UPDATE_JSON_URL        "http://archive-artifacts-pipeline.oss-cn-shanghai.aliyuncs.com/iot/esp32-demo/sit/esp32-demoota.json" // 如果https证书有问题 可以使用http协议
#define USE_MULTI_CORE 0 // 是否使用多核 根据芯片决定

// 提供 OTA 服务器证书以通过 HTTPS 进行身份验证server certificates  在platformio.ini内定义board_build.embed_txtfiles属性制定pem证书位置
// 生成pem证书文档: https://github.com/espressif/esp-idf/blob/master/examples/system/ota/README.md
// 证书生成命令(Windows系统在Git Bash执行): openssl req -x509 -newkey rsa:2048 -keyout ca_key.pem -out ca_cert.pem -days 3650 -nodes
extern const uint8_t server_cert_pem_start[] asm("_binary_server_certs_ca_cert_pem_start"); // key值为前后固定和pem全路径组合
extern const uint8_t server_cert_pem_end[] asm("_binary_server_certs_ca_cert_pem_end");

static HttpsOTAStatus_t otaStatus;


// Method to compare two versions.
// Returns 1 if v2 is smaller, -1 if v1 is smaller, 0 if equal
int version_compare(string v1, string v2) {
    // vnum stores each numeric
    // part of version
    int vnum1 = 0, vnum2 = 0;

    // loop until both string are
    // processed
    for (int i = 0, j = 0; (i < v1.length()
                            || j < v2.length());) {
        // storing numeric part of
        // version 1 in vnum1
        while (i < v1.length() && v1[i] != '.') {
            vnum1 = vnum1 * 10 + (v1[i] - '0');
            i++;
        }

        // storing numeric part of
        // version 2 in vnum2
        while (j < v2.length() && v2[j] != '.') {
            vnum2 = vnum2 * 10 + (v2[j] - '0');
            j++;
        }

        if (vnum1 > vnum2)
            return 1;
        if (vnum2 > vnum1)
            return -1;

        // if equal, reset variables and
        // go for next numeric part
        vnum1 = vnum2 = 0;
        i++;
        j++;
    }
    return 0;
}

/**
 * 执行固件升级
 */
void do_firmware_upgrade() {
    // while (1) {  // 多线程需要不断的任务执行
    // printf(CONFIG_FIRMWARE_UPGRADE_URL);
    DynamicJsonDocument json = http_get(UPDATE_JSON_URL);
    // 读取JSON数据
    // Serial.println("OTA响应数据:");
    string new_version = json["version"].as<string>();
    String file_url = json["file"].as<String>();
    //char *file_url = reinterpret_cast<char *>(json["file"].as<char>());
    //Serial.println(new_version);
    //Serial.println(file_url);

    if (version_compare(new_version, FIRMWARE_VERSION) == 1) {
        Serial.println("有新版本OTA固件, 正在下载...");
        esp_http_client_config_t config = {
                .url = file_url.c_str(),
                .cert_pem = (char *) server_cert_pem_start,
                .timeout_ms = 600000,
                //.crt_bundle_attach =  esp_crt_bundle_attach,
                .keep_alive_enable = true,
        };
        esp_err_t ret = esp_https_ota(&config);
        if (ret == ESP_OK) {
            Serial.println("执行OTA空中升级成功了, 准备重启单片机...");
            // 升级成功LED 闪动的方便查看
            digitalWrite(18, HIGH);
            delay(5000);
            esp_restart();
        } else {
            Serial.println("执行OTA空中升级失败");
            // return ESP_FAIL;
        }
    } else {
        Serial.println("没有新版本OTA固件, 跳过升级");
    }
    //printf("OTA定时检测任务延迟中...\n");
    // 每多少时间执行一次
    delay(30000);
    //vTaskDelay(30000 / portTICK_PERIOD_MS);
    // return ESP_OK; // esp_err_t 类型
    //}
}

/**
 * 执行OTA空中升级
 */
void exec_ota() {
    Serial.println("开始检测OTA空中升级...");
    do_firmware_upgrade();

/*#if !USE_MULTI_CORE
    xTaskCreate(
            do_firmware_upgrade,  *//* Task function. *//*
            "do_firmware_upgrade", *//* String with name of task. *//*
            8192,      *//* Stack size in bytes. *//*
            NULL,      *//* Parameter passed as input of the task *//*
            5,         *//* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) *//*
            NULL);     *//* Task handle. *//*
#else
    //最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1,或者 tskNO_AFFINITY 允许任务在两者上运行.
    xTaskCreatePinnedToCore(do_firmware_upgrade, "do_firmware_upgrade", 4096, NULL, 1, NULL, 0);
#endif*/

    // 开启多线程OTA任务
    // xTaskCreate(&do_firmware_upgrade, "do_firmware_upgrade", 8192, NULL, 5, NULL);
    // xTaskCreatePinnedToCore(do_firmware_upgrade, "do_firmware_upgrade", 8192, NULL, 3, NULL, 0);

    /* HttpsOTA.onHttpEvent(HttpEvent);
       HttpsOTA.begin(url, server_cert_pem_start);
       Serial.println("Please Wait it takes some time ..."); */

    // start the check update task
    // xTaskCreate(&check_update_task, "check_update_task", 8192, NULL, 5, NULL);
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
                //printf("响应数据: %.*s", evt->data_len, (char *) evt->data);
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
/*void check_update_task(void *pvParameter) {
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
        printf("\n");
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
        cJSON_Delete(json);
    } else printf("unable to download the json file, aborting...\n");

    // cleanup
    esp_http_client_cleanup(client);

    printf("\n");
    vTaskDelay(30000 / portTICK_PERIOD_MS);
}*/
