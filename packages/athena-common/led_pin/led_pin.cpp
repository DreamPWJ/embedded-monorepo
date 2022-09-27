#include "led_pin.h"
#include <Arduino.h>

/**
* @author 潘维吉
* @date 2022/7/20 9:54
* @description LED灯控制
*/

// LED等引脚名称数字 开发板有标注 如果不是 Arduino 框架定义的，则设置
#ifndef LED_PIN
#define LED_PIN 5  // 注意不同的开发板配置不同
#endif

/* 将 LED 数字引脚初始化为输出 */
void set_pin_mode() {
    pinMode(LED_PIN, OUTPUT);
}

/* 开发板LED 闪动的实现 */
void set_led() {
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
    delay(1000);
}