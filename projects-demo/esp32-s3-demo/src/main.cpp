#include <Arduino.h>

/**
* @author 潘维吉
* @date 2022/7/20 9:41
* @description 程序运行入口
*/

void setup() {
// write your initialization code here
    // 设置UART串口波特率
    Serial.begin(115200);
    Serial.println("你好, ESP32 S3单片机");
}

void loop() {
// write your code here
    delay(1000);
    Serial.println("测试新主控芯片循环执行");
}