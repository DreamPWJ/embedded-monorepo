#include <Arduino.h>
#include "../lib/led_pin/led_pin.h"

void setup() {
// write your initialization code here
    // 将 LED 数字引脚初始化为输出
    set_pin_mode();
}

void loop() {
// write your code here
    // 开发板LED 闪动的实现
    set_led();
}