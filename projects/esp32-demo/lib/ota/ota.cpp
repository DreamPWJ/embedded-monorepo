#include "ota.h"
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cJSON.h>
#include "esp_system.h"
#include <esp_https_ota.h>
#include "esp_crt_bundle.h"
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

static const char *url = "https://lanneng-epark-test.oss-cn-qingdao.aliyuncs.com/firmware.bin"; // state url of your firmware image

// 提供 OTA 服务器证书以通过 HTTPS 进行身份验证  新pem证书地址：https://github.com/espressif/esp-idf/blob/master/examples/system/ota/advanced_https_ota/server_certs/ca_cert.pem
static const char *server_certificate = "-----BEGIN CERTIFICATE-----\n"
                                        "MIIDWDCCAkACCQCbF4+gVh/MLjANBgkqhkiG9w0BAQsFADBuMQswCQYDVQQGEwJJ\n"
                                        "TjELMAkGA1UECAwCTUgxDDAKBgNVBAcMA1BVTjEMMAoGA1UECgwDRVNQMQwwCgYD\n"
                                        "VQQLDANFU1AxDDAKBgNVBAMMA0VTUDEaMBgGCSqGSIb3DQEJARYLZXNwQGVzcC5j\n"
                                        "b20wHhcNMjEwNzEyMTIzNjI3WhcNNDEwNzA3MTIzNjI3WjBuMQswCQYDVQQGEwJJ\n"
                                        "TjELMAkGA1UECAwCTUgxDDAKBgNVBAcMA1BVTjEMMAoGA1UECgwDRVNQMQwwCgYD\n"
                                        "VQQLDANFU1AxDDAKBgNVBAMMA0VTUDEaMBgGCSqGSIb3DQEJARYLZXNwQGVzcC5j\n"
                                        "b20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDhxF/y7bygndxPwiWL\n"
                                        "SwS9LY3uBMaJgup0ufNKVhx+FhGQOu44SghuJAaH3KkPUnt6SOM8jC97/yQuc32W\n"
                                        "ukI7eBZoA12kargSnzdv5m5rZZpd+NznSSpoDArOAONKVlzr25A1+aZbix2mKRbQ\n"
                                        "S5w9o1N2BriQuSzd8gL0Y0zEk3VkOWXEL+0yFUT144HnErnD+xnJtHe11yPO2fEz\n"
                                        "YaGiilh0ddL26PXTugXMZN/8fRVHP50P2OG0SvFpC7vghlLp4VFM1/r3UJnvL6Oz\n"
                                        "3ALc6dhxZEKQucqlpj8l1UegszQToopemtIj0qXTHw2+uUnkUyWIPjPC+wdOAoap\n"
                                        "rFTRAgMBAAEwDQYJKoZIhvcNAQELBQADggEBAItw24y565k3C/zENZlxyzto44ud\n"
                                        "IYPQXN8Fa2pBlLe1zlSIyuaA/rWQ+i1daS8nPotkCbWZyf5N8DYaTE4B0OfvoUPk\n"
                                        "B5uGDmbuk6akvlB5BGiYLfQjWHRsK9/4xjtIqN1H58yf3QNROuKsPAeywWS3Fn32\n"
                                        "3//OpbWaClQePx6udRYMqAitKR+QxL7/BKZQsX+UyShuq8hjphvXvk0BW8ONzuw9\n"
                                        "RcoORxM0FzySYjeQvm4LhzC/P3ZBhEq0xs55aL2a76SJhq5hJy7T/Xz6NFByvlrN\n"
                                        "lFJJey33KFrAf5vnV9qcyWFIo7PYy2VsaaEjFeefr7q3sTFSMlJeadexW2Y=\n"
                                        "-----END CERTIFICATE-----";

#define FIRMWARE_VERSION       1.0
#define UPDATE_JSON_URL        "https://esp32tutorial.netsons.org/https_ota/firmware.json"

// server certificates  在platformio.ini内定义board_build.embed_txtfiles属性制定pem证书位置
extern const char server_cert_pem_start[] asm("_binary_server_certs_ca_cert_pem_start"); // key值为前后固定和pem全路径组合
extern const char server_cert_pem_end[] asm("_binary_server_certs_ca_cert_pem_end");

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
char rcv_buffer[200];

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

// Check update task
// downloads every 30sec the json file with the latest firmware
void check_update_task(void *pvParameter) {
    printf("Looking for a new firmware...\n");

    // configure the esp_http_client
    esp_http_client_config_t config = {
            .url = UPDATE_JSON_URL,
            .event_handler = _http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // downloading the json file
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {

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
                                .cert_pem = server_cert_pem_start,
                        };
                        esp_err_t ret = esp_https_ota(&ota_client_config);
                        if (ret == ESP_OK) {
                            printf("OTA OK, restarting...\n");
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
 * 执行OTA空中升级
 */
void exec_ota() {
    Serial.println("开始执行OTA空中升级");
/*  HttpsOTA.onHttpEvent(HttpEvent);
    HttpsOTA.begin(url, server_certificate);
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