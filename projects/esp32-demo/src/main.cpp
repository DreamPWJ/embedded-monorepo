#include <Arduino.h>
#include "../../../packages/athena-common/bluetooth/bluetooth.h"
#include "../../../packages/athena-common/chip_info/chip_info.h"
#include "../../../packages/athena-common/led_pin/led_pin.h"
#include "../../../packages/athena-common/wifi_network/wifi_network.h"
#include "../../../packages/athena-common/http/http.h"


void setup() {
// write your initialization code here
    Serial.begin(115200);
    // 将 LED 数字引脚初始化为输出
    set_pin_mode();
    // 初始化蓝牙设置
    // init_bluetooth("ESP32-PanWeiJi");
    // 初始化Wifi无线网络
    // init_wifi();
    // FreeRTOS多线程处理  Create a connection task with 8kB stack on core 0
    // xTaskCreatePinnedToCore(init_wifi_multi_thread, "WiFiTask", 8192, NULL, 3, NULL, 0);
}

void loop() {
// write your code here
    Serial.print("Hello World Embedded ESP32! \n");
    // 获取硬件信息
    get_chip_info();
    delay(1000);
    // 开发板LED 闪动的实现
    set_led();
    // 监听蓝牙状态
    // bluetooth_state();
    // 定时检测重新连接WiFi
    // reconnect_wifi();
    // 网络请求
    //http_get("https://www.google.com/");
}