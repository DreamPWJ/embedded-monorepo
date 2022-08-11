#include <Arduino.h>
#include "../../../packages/athena-common/led_pin/led_pin.h"
#include "../../../packages/athena-common/chip_info/chip_info.h"
#include "../lib/bluetooth/bluetooth_connect.h"
#include "../lib/wifi/wifi_network.h"
#include "../../../packages/athena-common/http/http.h"
#include "../lib/aliyun/aliyun_iot.h"
#include "../lib/ota/ota.h"


void setup() {
// write your initialization code here
    Serial.begin(115200);
    // while (Serial.available()) {  // 等待串口连接成功
    Serial.println("串口连接成功");
    // 将 LED 数字引脚初始化为输出
    set_pin_mode();
    // 初始化蓝牙设置
    init_bluetooth("ESP32-PanWeiJi");
    // 初始化Wifi无线网络
    init_wifi();
    // FreeRTOS实时系统多线程处理  Create a connection task with 8kB stack on core 0
    // xTaskCreatePinnedToCore(init_wifi_multi_thread, "WiFiTask", 8192, NULL, 3, NULL, 0);
    // 网络请求
    http_get("https://tcc.taobao.com/cc/json/mobile_tel_segment.htm?tel=18863302302");
    /* http_post(
             "https://oapi.dingtalk.com/robot/send?access_token=383391980b120c38f0f9a4a398349739fa67a623f9cfa834df9c5374e81b2081",
             "");*/
    // init_aliyun_iot_sdk();
    // }
    // OTA空中升级
    exec_ota();
}

void loop() {
// write your code here
    Serial.println("PlatformIO And Arduino For Embedded ESP32! \n");
    // 获取硬件信息
    get_chip_info();
    delay(2000);
    // 开发板LED 闪动的实现
    set_led();
    // 监听蓝牙状态
    // bluetooth_state();
    // 定时检测重新连接WiFi
    reconnect_wifi();
    // 检测OTA
    check_ota();
}