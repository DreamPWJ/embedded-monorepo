#include <Arduino.h>
#include "../lib/bluetooth/bluetooth.h"
#include "../lib/chip_info/chip_info.h"
#include "../lib/led_pin/led_pin.h"
#include "../lib/wifi/wifi.h"


void setup() {
// write your initialization code here
    Serial.begin(115200);
    // 将 LED 数字引脚初始化为输出
    set_pin_mode();
    // 初始化蓝牙设置
    // init_buletooth("ESP32-PanWeiJi");
    // 初始化Wifi无线网络
    init_wifi();
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
    // buletooth_state();
}