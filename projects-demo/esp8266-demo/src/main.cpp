#include <Arduino.h>
//#include "../../../packages/athena-common/led_pin/led_pin.h"

void setup() {
// write your initialization code here
    Serial.begin(115200);
    // 将 LED 数字引脚初始化为输出
    //set_pin_mode();
}

void loop() {
// write your code here
    Serial.println("PlatformIO And Arduino For Embedded ESP32! \n");
    delay(1000);
    // 开发板LED 闪动的实现
    // set_led();
}