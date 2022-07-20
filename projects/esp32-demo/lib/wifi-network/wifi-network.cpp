#include "wifi-network.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiType.h>

/**
* @author 潘维吉
* @date 2022/7/20 10:36
* @description WiFI无线网络模块
  */


/* 设置Wifi */
void init_wifi() {
    // 设置wifi账号和密码  注意文件名称不要和库的名称一样，会导致error: 'WiFi' was not declared in this scope
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