#include <Arduino.h>
#include "../../../packages/athena-common/led_pin/led_pin.h"

char serialData; // 串口数据读取值

void setup() {
// write your initialization code here
    Serial.begin(115200);
    // 将LED数字引脚初始化为输出
    set_pin_mode();
}

void loop() {
// write your code here
    // Serial.println("PlatformIO And Arduino For Embedded ESP32! \n");
    // 获取硬件信息
    // get_chip_info();
    delay(2000);
    // 开发板LED 闪动的实现
    set_led();

    /**
     * 读取串口数据
     */
    if (Serial.available()) {
        serialData = Serial.read();
        Serial.print("读取串口数据: ");
        Serial.println(serialData);
    }
}