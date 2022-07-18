#include <Arduino.h>

int LED_PIN = 18; // LED等引脚名称数字 开发板有标注

void setup() {
// write your initialization code here
    Serial.begin(9600);

    pinMode(LED_PIN, OUTPUT);
}

void loop() {
// write your code here
    Serial.print("Hello World Embedded ESP32! \n");
    delay(2000);

    // 开发板LED 闪动的实现
    digitalWrite(LED_PIN, HIGH);
    delay(2000);
    digitalWrite(LED_PIN, LOW);
    delay(2000);
}