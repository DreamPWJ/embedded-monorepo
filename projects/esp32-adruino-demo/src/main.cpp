#include <Arduino.h>

void setup() {
// write your initialization code here
    Serial.begin(9600);
}

void loop() {
// write your code here
    Serial.print("Hello World!");
    delay(1000);
}