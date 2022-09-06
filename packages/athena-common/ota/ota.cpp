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
#include <common_utils.h>
#include <version_utils.h>

using namespace std;

/**
* @author 潘维吉
* @date 2022/7/29 15:56
* @description 固件OTA空中升级
* 参考文档：https://blog.51cto.com/u_15284384/3054914
* https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_https_ota.html
* https://github.com/espressif/arduino-esp32/tree/master/libraries/Update/examples/HTTPS_OTA_Update
* https://github.com/lucadentella/esp32-tutorial/blob/master/30_https_ota/main/main.c
*/

// WebServer server(80);

// 固件文件地址 可存储到公有云OSS或者公共Git代码管理中用于访问  如果https证书有问题 可以使用http协议
/*#define FIRMWARE_VERSION         "CI_OTA_FIRMWARE_VERSION"  // 版本号用于OTA升级和远程升级文件对比 判断是否有新版本 每次需要OTA的时候更改设置
#define FIRMWARE_UPDATE_JSON_URL "http://archive-artifacts-pipeline.oss-cn-shanghai.aliyuncs.com/iot/ground-lock/prod/ground-lockota.json" // 如果https证书有问题 可以使用http协议*/
#define USE_MULTI_CORE 0 // 是否使用多核 根据芯片决定

// 提供 OTA 服务器证书以通过 HTTPS 进行身份验证server certificates  在platformio.ini内定义board_build.embed_txtfiles属性制定pem证书位置
// 生成pem证书文档: https://github.com/espressif/esp-idf/blob/master/examples/system/ota/README.md
// 证书生成命令(Windows系统在Git Bash执行): openssl req -x509 -newkey rsa:2048 -keyout ca_key.pem -out ca_cert.pem -days 3650 -nodes
extern const uint8_t server_cert_pem_start[] asm("_binary_lib_server_certs_ca_cert_pem_start"); // key值为前后固定和pem全路径组合

/**
 * 执行固件升级 1. 定时检测HTTP方式 1.主动触发MQTT方式
 */
void do_firmware_upgrade(String version, String jsonUrl) {
    DynamicJsonDocument json = http_get(jsonUrl);
    // 读取JSON数据
    // Serial.println("OTA响应数据:");
    String new_version = json["version"].as<String>();
    String file_url = json["file"].as<String>();
    //char *file_url = reinterpret_cast<char *>(json["file"].as<char>());
    //Serial.println(new_version);
    //Serial.println(file_url);

    if (version_compare(new_version, version) == 1) {
        // 做固件MD5签名算法 保证固件本身是安全的
        printf("当前固件版本v%s, 有新v%s版本OTA固件, 正在下载... \n", version.c_str(), new_version.c_str());
        esp_http_client_config_t config = {
                .url = file_url.c_str(),
                .cert_pem = (char *) server_cert_pem_start,
                .timeout_ms = 600000,
                //.crt_bundle_attach =  esp_crt_bundle_attach,
                .keep_alive_enable = true,
        };
        esp_err_t ret = esp_https_ota(&config);
        if (ret == ESP_OK) {
            // 检测固件是否正常  设计失败恢复方案 如果固件启动失败回滚
            Serial.println("执行OTA空中升级成功了, 重启单片机...");
            // 升级成功LED 闪动的方便在硬件方式查看
            /*    digitalWrite(18, HIGH);
                  delay(5000); */
            esp_restart();
        } else {
            Serial.println("执行OTA空中升级失败");
            // return ESP_FAIL;
        }
    } else {
        printf("当前固件版本v%s, 没有新版本OTA固件, 跳过升级 \n", version.c_str());
    }
    // return ESP_OK; // esp_err_t 类型
}

/**
 * 执行OTA空中升级
 */
static String otaVersion;
static String otaJsonUrl;

void exec_ota(String version, String jsonUrl) {
    Serial.println("开始检测OTA空中升级...");
    // do_firmware_upgrade(version, jsonUrl);
    otaVersion = version;
    otaJsonUrl = jsonUrl;
#if !USE_MULTI_CORE
/*    String paramsStr = version + "," + jsonUrl;
    Serial.println(paramsStr.c_str());
    const char *params = paramsStr.c_str(); // 逗号分割多参数*/
    const char *params = NULL;
    xTaskCreate(
            x_task_ota,  /* Task function. */
            "x_task_ota", /* String with name of task. */
            8192,      /* Stack size in bytes. */
            (void *) params,      /* Parameter passed as input of the task */
            5,         /* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) */
            NULL);     /* Task handle. */
#else
    //最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1,或者 tskNO_AFFINITY 允许任务在两者上运行.
    xTaskCreatePinnedToCore(x_task_ota, "TaskOTA", 8192, NULL, 5, NULL, 0);
#endif

    /* HttpsOTA.onHttpEvent(HttpEvent);
       HttpsOTA.begin(url, server_cert_pem_start);
       Serial.println("Please Wait it takes some time ..."); */
}

/**
 * 多线程OTA任务
 */
char *pcTaskName;

void x_task_ota(void *pvParameters) {
    while (1) {
        // Serial.println("多线程OTA任务, 检测OTA空中升级...");
        /*    pcTaskName = (char *) pvParameters;
               std::vector<string> res = split(pcTaskName, ",");
               String version = res[0].c_str();
               String jsonUrl = res[1].c_str();
               Serial.println(version);
               Serial.println(jsonUrl); */
        do_firmware_upgrade(otaVersion, otaJsonUrl);
        delay(60000); // 多久执行一次 毫秒
    }
}
