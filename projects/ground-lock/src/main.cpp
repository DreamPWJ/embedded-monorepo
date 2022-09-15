#include <common.h>
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
#include <mcu_nvs.h>
#include <uart.h>
#include <device_info.h>
#include <http.h>
#include <at_mqtt/at_mqtt.h>
#include <at_http/at_http.h>


using namespace std;

#define FIRMWARE_VERSION              "0.7.0"  // 版本号用于OTA升级和远程升级文件对比 判断是否有新版本 每次需要OTA的时候更改设置 CI_OTA_FIRMWARE_VERSION关键字用于CI替换版本号
#define FIRMWARE_UPDATE_JSON_URL      "http://archive-artifacts-pipeline.oss-cn-shanghai.aliyuncs.com/iot/ground-lock/prod/ground-lockota.json" // 如果https证书有问题 可以使用http协议
#define WIFI_EN 0 // 是否开启WIFI网络功能 0 关闭  1 开启
#define MQTT_EN 1 // 是否开启MQTT消息协议 0 关闭  1 开启
#define PWM_EN 1 // 是否开启PWM脉冲宽度调制功能 0 关闭  1 开启


void setup() {
    // 初始化设置代码

    // 设置串口波特率
    Serial.begin(115200);
    delay(1000);

    // 初始化其它UART串口
    // init_uart();

    // 将LED数字引脚初始化为输出
    set_pin_mode();

    // 初始化非易失性存储
    int_nvs();
    //set_nvs("name", "panweiji");

    // 初始化NB-IoT网络协议
    init_nb_iot();

    // WiFi网络版本HTTP请求
    // http_get("http://tcc.taobao.com/cc/json/mobile_tel_segment.htm?tel=18863302302");
    // NB-IoT网络版本HTTP请求
    // at_http_get("archive-artifacts-pipeline.oss-cn-shanghai.aliyuncs.com/iot/ground-lock/prod/ground-lockota.json");


#if WIFI_EN
    // 初始化WiFi无线网络
    init_wifi();
#endif

#if MQTT_EN
    // 初始化MQTT消息协议
    init_at_mqtt();

    // MQTT心跳服务
    at_mqtt_heart_beat();

    // WiFi网络版本初始化MQTT消息协议
    // init_mqtt();

    // WiFi网络版本MQTT心跳服务
    // mqtt_heart_beat();
#endif

#if PWM_EN
    // 初始化电机马达
    init_motor();
    // 初始化地感线圈
    init_ground_feeling();
    // 检测地感状态 有车无车及时上报MQTT服务器
    check_ground_feeling_status();
#endif

    // WiFi网络版本执行OTA空中升级
    //exec_ota(FIRMWARE_VERSION, FIRMWARE_UPDATE_JSON_URL);

}

void loop() {
    // 循环执行代码

    // 开发板LED 闪动的实现
    set_led();

#if WIFI_EN
    // 定时检测重新连接WiFi
    reconnect_wifi();
#endif

#if MQTT_EN
    // WiFi网络版本MQTT消息服务
    // mqtt_loop();
#endif

#if PWM_EN
    // 驱动电机马达工作
    // set_pwm();
#endif

}