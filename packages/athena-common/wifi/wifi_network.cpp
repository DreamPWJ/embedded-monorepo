#include "wifi_network.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>

/**
* @author 潘维吉
* @date 2022/7/20 10:36
* @description WiFI无线网络模块
*/

WiFiMulti wifiMulti;

const char *ssid = "TP-LINK_A6B2_4G";  // WiFi用户名  注意模组只支持2.4G
const char *password = "rzgj0633";  // WiFi密码 最少 8 个字符

unsigned long previousMillis = 0;
unsigned long interval = 60000; // 检测wifi状态间隔 毫秒

/**
 * 初始化WiFi
 * 参考文档: https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/
 */
void init_wifi() {
    Serial.println("开始初始化WiFi模块");
    // ESP32 WiFiMulti功能：连接到多个网络中的最强的 Wi-Fi 网络, 也可以扫描公开网络实现自动联网无需配网
    // 站模式：ESP32 连接到接入点连接到另一个网络, 它必须处于工作站模式
    WiFi.mode(WIFI_STA);
    // 设置wifi账号和密码  注意文件名称不要和库的名称一样，会导致error: 'WiFi' was not declared in this scope
    WiFi.begin(ssid, password);
    // 阻塞程序，直到连接成功
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("");
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi 连接成功！");
        // 开发板LED 闪动的实现
/*      digitalWrite(18, HIGH);
        delay(2000);
        digitalWrite(18, LOW);
        delay(1000); */
        Serial.println("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("WiFi连接强度RRSI: ");
        Serial.println(WiFi.RSSI());
    }

    // 断开与 Wi-Fi 网络的连接
    // WiFi.disconnect();
}

/**
 * 多线程处理
 */
void init_wifi_multi_thread(void *pvParameters) {
    init_wifi();
}

/**
 * 扫码WiFi 选择开放Wifi直接连接
 */
void scan_wifi() {
    WiFi.mode(WIFI_STA);
    // Add list of wifi networks
    wifiMulti.addAP("ssid_from_AP_1", "your_password_for_AP_1");
    wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
    wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
            delay(10);
        }
    }

    // Connect to Wi-Fi using wifiMulti (connects to the SSID with strongest connection)
    Serial.println("Connecting Wifi...");
    if (wifiMulti.run() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }
}

/**
 * 定时检测重新连接WiFi
 */
void reconnect_wifi() {
    unsigned long currentMillis = millis();
    // 如果 WiFi 已关闭，请尝试每隔 CHECK_WIFI_TIME 秒重新连接一次
    if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval)) {
        // Serial.print(millis());
        Serial.println("WiFi已关闭, 重新连接WiFi...");
        WiFi.disconnect();
        WiFi.reconnect();
    }
}