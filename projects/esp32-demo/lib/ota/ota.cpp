#include "ota.h"
#include <Arduino.h>
#include "HttpsOTAUpdate.h"

/**
* @author 潘维吉
* @date 2022/7/29 15:56
* @description 固件OTA空中升级
* 参考文档：https://blog.51cto.com/u_15284384/3054914
* https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_https_ota.html
* https://github.com/espressif/arduino-esp32/tree/master/libraries/Update/examples/HTTPS_OTA_Update
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

/**
 * 执行OTA空中升级
 */
void exec_ota() {
    HttpsOTA.onHttpEvent(HttpEvent);
    Serial.println("Starting OTA");
    HttpsOTA.begin(url, server_certificate);
    Serial.println("Please Wait it takes some time ...");
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