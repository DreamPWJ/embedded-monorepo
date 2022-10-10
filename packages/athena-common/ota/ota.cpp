#include "ota.h"
#include <Arduino.h>
#include <ArduinoJson.h>
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
#include <bits/stdc++.h>
#include <common_utils.h>
#include <version_utils.h>
#include <at_http/at_http.h>
#include <wifi_network.h>
#include <at_mqtt/at_mqtt.h>
#include <chip_info.h>
#include <iostream>
#include <string>

using namespace std;

/**
* @author 潘维吉
* @date 2022/7/29 15:56
* @description 固件OTA空中升级
* 参考文档： https://github.com/espressif/esp-idf/tree/master/examples/system/ota
* https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_https_ota.html
* https://github.com/espressif/arduino-esp32/tree/master/libraries/Update/examples/
* https://github.com/lucadentella/esp32-tutorial/blob/master/30_https_ota/main/main.c
*/

// WebServer server(80);

// 固件文件地址 可存储到公有云OSS或者公共Git代码管理中用于访问  如果https证书有问题(URL 的服务器部分必须与生成证书和密钥时使用的CN字段匹配), 可以使用http协议
/*#define FIRMWARE_VERSION         "CI_OTA_FIRMWARE_VERSION"  // 版本号用于OTA升级和远程升级文件对比 判断是否有新版本 每次需要OTA的时候更改设置
#define FIRMWARE_UPDATE_JSON_URL "http://archive-artifacts-pipeline.oss-cn-shanghai.aliyuncs.com/iot/ground-lock/prod/ground-lockota.json" // 如果https证书有问题 可以使用http协议*/
#define USE_MULTI_CORE 0 // 是否使用多核 根据芯片决定
#define WIFI_ONLY_OTA 1  // 是否WIFI网络功能仅用于OTA升级  0 否  1 是

// 提供 OTA 服务器证书以通过 HTTPS 进行身份验证server certificates  在platformio.ini内定义board_build.embed_txtfiles属性制定pem证书位置
// 生成pem证书文档: https://github.com/espressif/esp-idf/tree/master/examples/system/ota
// 证书生成命令(Windows系统在Git Bash执行): openssl req -x509 -newkey rsa:2048 -keyout ca_key.pem -out ca_cert.pem -days 3650 -nodes
extern const uint8_t server_cert_pem_start[] asm("_binary_lib_server_certs_ca_cert_pem_start"); // key值为前后固定和pem全路径组合

/**
 * 执行固件升级
 * 1. 定时检测HTTP方式 2. 主动触发MQTT方式
 * 1. 整包升级 2. 差分包升级
 * 对于弱网络如NB-IoT(无法下载完整大固件包、差分升级复杂度高)或不使用Wifi作为主网络的OTA升级 可采用不完美的降级方案 检测到有新固件版本时扫描并建立开放WIFI连接(公网AP、4G路由器、手机热点等)进行OTA下载升级 升级成功后关闭WIFI连接来减少功耗和不稳定网络
 */
void do_firmware_upgrade(String version, String jsonUrl, String firmwareUrl) {

    String new_version; // 新版本号
    String file_url;  // 固件下载地址

    const char *ota_topic = "ESP32/system";
    uint64_t chipId = get_chip_mac();

#if WIFI_ONLY_OTA
    // 扫描并建立开放WIFI连接
    Serial.println("自动扫描并建立开放WIFI网络连接...");
    bool is_has_wifi = scan_wifi();
    if (!is_has_wifi) {
        Serial.println("没有开放WIFI网络, 退出OTA空中升级");
        return;
    } else {
        if (String(firmwareUrl) != "") {
            Serial.println("直接根据提供的固件地址强制进行OTA空中升级");
            new_version = "NEW_VERSION";
            file_url = firmwareUrl;
        } else {
            // 读取OTA升级文件 JSON数据
            DynamicJsonDocument json = http_get(jsonUrl);
            // Serial.println("OTA响应数据:");
            new_version = json["version"].as<String>();
            file_url = json["file"].as<String>();
            // String md5 = json["md5"].as<String>();
        }

        Serial.println(new_version);
        Serial.println(file_url);
    }
#endif

    // 检测到新版本或者指定设备才进行OTA空中升级
    if ((new_version != "null" && version_compare(new_version, version) == 1) ||
        (new_version.indexOf("NEW_VERSION") != -1)) {
        // 做固件MD5签名算法 保证固件本身是安全的  升级json文件中的原始md5值和http请求头Content-MD5中的md5值保持一致
        printf("当前固件版本v%s, 有新v%s版本OTA固件, 正在下载... \n", version.c_str(), new_version.c_str());
#if WIFI_ONLY_OTA
        // 上报MQTT消息
        string jsonDataDo = "{\"msg\":\"发现新版本了, 开始执行OTA空中升级\",\"chipId\":\"" + to_string(chipId) + "\"}";
        at_mqtt_publish(ota_topic, jsonDataDo.c_str());
#endif
        esp_http_client_config_t config = {
                .url = file_url.c_str(),
                .cert_pem = (char *) server_cert_pem_start, // HTTPS 客户端将检查证书内的CN字段与HTTPS URL中给出的地址是否匹配
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
                  delay(2000); */
#if WIFI_ONLY_OTA
            // 上报MQTT消息
            string jsonDataSuccess = "{\"msg\":\"执行OTA空中升级成功了\",\"chipId\":\"" + to_string(chipId) + "\"}";
            at_mqtt_publish(ota_topic, jsonDataSuccess.c_str());
            delay(1000);
            // 升级成功后关闭WIFI连接来减少功耗和不稳定网络
            WiFi.disconnect();
#endif
            esp_restart();
        } else {
            Serial.println("执行OTA空中升级失败: " + chipId);
#if WIFI_ONLY_OTA
            // 上报MQTT消息
            string jsonDataFail = "{\"msg\":\"执行OTA空中升级失败了\",\"chipId\":\"" + to_string(chipId) + "\"}";
            at_mqtt_publish(ota_topic, jsonDataFail.c_str());
            // 升级成功后关闭WIFI连接来减少功耗和不稳定网络
            WiFi.disconnect();
#endif
            // return ESP_FAIL;
        }
    } else {
        printf("当前固件版本v%s, 没有检测到新版本OTA固件, 跳过升级 \n", version.c_str());
    }
    // return ESP_OK; // esp_err_t 类型
}

/**
 * 执行OTA空中升级
 */
static String otaVersion;
static String otaJsonUrl;

void exec_ota(String version, String jsonUrl) {
    Serial.println("开始自动检测OTA空中升级...");
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
            1024 * 16,      /* Stack size in bytes. */
            (void *) params,      /* Parameter passed as input of the task */
            0,         /* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) */
            NULL);     /* Task handle. */
#else
    //最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1,或者 tskNO_AFFINITY 允许任务在两者上运行.
    xTaskCreatePinnedToCore(x_task_ota, "x_task_ota", 8192, NULL, 10, NULL, 0);
#endif

    /* HttpsOTA.onHttpEvent(HttpEvent);
       HttpsOTA.begin(url, server_cert_pem_start);
       */
}

/**
 * 多线程OTA任务
 */
char *pcTaskName;

void x_task_ota(void *pvParameters) {
    while (1) {  // RTOS多任务条件： 1. 不断循环 2. 无return关键字
        // Serial.println("多线程OTA任务, 检测OTA空中升级...");
        /*    pcTaskName = (char *) pvParameters;
               std::vector<string> res = split(pcTaskName, ",");
               String version = res[0].c_str();
               String jsonUrl = res[1].c_str();
               Serial.println(version);
               Serial.println(jsonUrl); */
        do_firmware_upgrade(otaVersion, otaJsonUrl);
        delay(3600 * 1000); // 多久执行一次 毫秒
    }
}
