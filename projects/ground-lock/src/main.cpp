#include <Arduino.h>
#include <led_pin.h>
#include <nb_iot.h>
#include <wifi_network.h>
#include <mqtt.h>
#include <pwm.h>
#include <ground_feeling.h>
#include <ota.h>

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

    // 初始化MQTT消息协议
    init_mqtt();

    // 初始化电机马达
    init_motor();

    // 初始化地感线圈
    init_ground_feeling();

    // 执行OTA空中升级
    exec_ota();
}

void loop() {
    // 循环执行代码

    // 开发板LED 闪动的实现
    set_led();

#if WIFI_EN
    // 定时检测重新连接WiFi
    reconnect_wifi();
#endif

    // 驱动电机马达工作
    set_pwm();

    // MQTT消息服务
    mqtt_loop();

    // 地感状态检测  判断是否有车
    ground_feeling_status();

}