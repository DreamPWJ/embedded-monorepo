#include <Arduino.h>

uint8_t LED_BUILTIN;

void setup() {
// write your initialization code here
    Serial.begin(9600);

    // pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
// write your code here
    Serial.print("Hello World Embedded!");
    delay(2000);

    // 开发板LED 闪动的实现
/*    digitalWrite(LED_BUILTIN, HIGH);
    delay(2000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(2000);*/
}