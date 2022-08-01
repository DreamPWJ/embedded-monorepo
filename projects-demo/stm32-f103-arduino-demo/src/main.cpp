#include <Arduino.h>

void setup() {
// write your initialization code here
    Serial.begin(115200);
}

void loop() {
// write your code here
    Serial.println("PlatformIO And Arduino For Embedded STM32! \n");
    delay(1000);
}