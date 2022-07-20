#include <Arduino.h>
#include "../lib/bluetooth_low_energy/bluetooth_low_energy.h"
#include "../lib/chip_info/chip_info.h"
#include "../lib/led_pin/led_pin.h"


void setup() {
// write your initialization code here
    Serial.begin(115200);
    // 将 LED 数字引脚初始化为输出
    set_pin_mode();
    // 初始化蓝牙设置
    init_BLE("ESP32-PanWeiJi");
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
    monitor_state();
}