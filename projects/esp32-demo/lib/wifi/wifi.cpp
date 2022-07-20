#include "wifi.h"
#include <Arduino.h>
#include <WiFiType.h>
#include <WiFi.h>

/**
* @author 潘维吉
* @date 2022/7/20 10:36
* @description WiFI无线网络模块
  */


/* 设置Wifi */
void init_wifi() {
    WiFi.begin("TP-LINK_A6B2_5G", "rzgj0633");
    // 阻塞程序，直到连接成功
    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(".");
    }
    Serial.println("WiFi 连接成功！");

    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}