#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <led_pin.h>
#include <nb_iot.h>
#include <wifi_network.h>
#include <mqtt.h>
#include <pwm.h>
#include <ground_feeling.h>
#include <ota.h>
#include <chip_info.h>
#include <iostream>
#include <string>
#include <nvs.h>
#include <uart.h>
#include <device_info.h>
#include <http.h>
#include "../lib/gsm_http/gsm_http.h"
#include "../lib/gsm_mqtt/gsm_mqtt.h"

using namespace std;

#define FIRMWARE_VERSION              "0.6.3"  // 版本号用于OTA升级和远程升级文件对比 判断是否有新版本 每次需要OTA的时候更改设置 CI_OTA_FIRMWARE_VERSION关键字用于CI替换版本号
#define FIRMWARE_UPDATE_JSON_URL      "http://archive-artifacts-pipeline.oss-cn-shanghai.aliyuncs.com/iot/ground-lock/prod/ground-lockota.json" // 如果https证书有问题 可以使用http协议
#define WIFI_EN 0 // 是否开启WIFI网络功能 0 关闭  1 开启
#define MQTT_EN 0 // 是否开启MQTT消息协议 0 关闭  1 开启
#define PWM_EN 1 // 是否开启PWM脉冲宽度调制功能 0 关闭  1 开启

String mqttName = "esp32-mcu-client"; // mqtt客户端名称

void setup() {
    // 初始化设置代码

    // 设置串口波特率
    Serial.begin(115200);

    // 初始化其它UART串口
    // init_uart();

    // 将LED数字引脚初始化为输出
    set_pin_mode();

    // 初始化非易失性存储
    intNVS();
    //setNVS("name", "panweiji");

    // 初始化NB-IoT网络协议
    init_nb_iot();

    // 网络请求
    //http_get("http://tcc.taobao.com/cc/json/mobile_tel_segment.htm?tel=18863302302");
    //esp_http();
    //nb_http_get();
    // gsm_http_get();
    // init_gsm_mqtt();

#if WIFI_EN
    // 初始化WiFi无线网络
    init_wifi();
#endif

#if MQTT_EN
    // 初始化MQTT消息协议
    init_mqtt(mqttName);

    // MQTT心跳服务
    mqtt_heart_beat();
#endif

#if PWM_EN
    // 初始化电机马达
    init_motor();
#endif

    // 初始化地感线圈
    init_ground_feeling();

    // 初始化温度设置
    // init_temperature();

    // 执行OTA空中升级
    exec_ota(FIRMWARE_VERSION, FIRMWARE_UPDATE_JSON_URL);

}

void loop() {
    // 循环执行代码

    // 开发板LED 闪动的实现
    set_led();

    //Serial.println(getNVS("name"));
    //Serial.println(get_temperature());

#if WIFI_EN
    // 定时检测重新连接WiFi
    reconnect_wifi();
#endif

#if PWM_EN
    // 驱动电机马达工作
    // set_pwm();
#endif

#if MQTT_EN
    // MQTT消息服务
    mqtt_reconnect(mqttName);
    mqtt_loop();
#endif

    // 地感状态检测  判断是否有车
    // ground_feeling_status();

    // gsm_mqtt_loop();

}