#include <Arduino.h>
#include <led_pin.h>

void setup() {
    // 初始化代码
    Serial.begin(115200);

    // 将LED数字引脚初始化为输出
    set_pin_mode();
}

void loop() {
   // 循环执行代码
   // 开发板LED 闪动的实现
    set_led();
}