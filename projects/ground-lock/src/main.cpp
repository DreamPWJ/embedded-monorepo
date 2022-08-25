#include <Arduino.h>
#include <led_pin.h>
#include <nb_iot.h>
#include <wifi_network.h>

#define WIFI_EN 1 // 是否开启WIFI网络功能 0 关闭  1 开启

void setup() {
    // 初始化设置代码

    // 设置串口波特率
    Serial.begin(115200);

    // 将LED数字引脚初始化为输出
    set_pin_mode();

    // 初始化NB-IoT网络协议
    init_nb_iot();

#if WIFI_EN
    // 初始化Wifi无线网络
    init_wifi();
#endif

}

void loop() {
    // 循环执行代码

    // 开发板LED 闪动的实现
    set_led();

#if WIFI_EN
    // 定时检测重新连接WiFi
    reconnect_wifi();
#endif

}