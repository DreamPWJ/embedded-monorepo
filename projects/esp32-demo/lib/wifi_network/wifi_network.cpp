#include "wifi_network.h"
#include <Arduino.h>
#include <WiFi.h>

//there's an include for this but it doesn't define the function if it doesn't think it needs it, so manually declare the function
extern "C" void phy_bbpll_en_usb(bool en);

/**
* @author 潘维吉
* @date 2022/7/20 10:36
* @description WiFI无线网络模块
*/

const char *ssid = "Tech";  // WiFi用户名  注意模组只支持2.4G
const char *password = "a123456789";  // WiFi密码 最少 8 个字符

unsigned long previousMillis = 0;
unsigned long interval = 30000;

/* 设置Wifi */
// 参考文档: https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/
void init_wifi() {
    Serial.println("开始初始化WiFi模块...");
    // ESP32 WiFiMulti功能：连接到多个网络中的最强的 Wi-Fi 网络
    // 站模式：ESP32 连接到接入点连接到另一个网络, 它必须处于工作站模式
    // WiFi.useStaticBuffers(true);
    WiFi.mode(WIFI_STA);
    // 解决ESP32-C3 原生 USB CDC 在使用 WiFi 时停止工作
    phy_bbpll_en_usb(
            true); // this brings the USB serial-jtag back to life. Suggest doing this immediately after wifi startup.
    // 设置wifi账号和密码  注意文件名称不要和库的名称一样，会导致error: 'WiFi' was not declared in this scope
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    // 阻塞程序，直到连接成功
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi 连接成功！");
        // 开发板LED 闪动的实现
        digitalWrite(18, HIGH);
        delay(6000);
        digitalWrite(18, LOW);
        delay(1000);
    }

    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.print("WiFi连接强度RRSI: ");
    Serial.println(WiFi.RSSI());

    // 断开与 Wi-Fi 网络的连接
    // WiFi.disconnect();
}

void init_wifi_multi_thread(void *pvParameters) {
    init_wifi();
}

/* 定时检测重新连接WiFi */
void reconnect_wifi() {
    unsigned long currentMillis = millis();
    //如果 WiFi 已关闭，请尝试每隔 CHECK_WIFI_TIME 秒重新连接一次
    if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval)) {
        Serial.print(millis());
        Serial.println("Reconnecting to WiFi...");
        WiFi.disconnect();
        WiFi.reconnect();
    }
}