
#include <Modbus.h>
#include <SoftwareSerial.h>

SoftwareSerial serial(2, 3); // RX, TX

Modbus modbus;

void setup() {
    Serial.begin(9600);
    serial.begin(9600);
    modbus.begin(&serial);
}

void loop() {
    if (modbus.available()) {
        byte device = modbus.readDeviceIdentifier();
        Serial.print("Device ID: ");
        Serial.println(device, DEC);
    }

    if (Serial.available()) {
        byte address = Serial.read();
        byte registerNumber = Serial.read();
        byte value = Serial.read();

        modbus.write(address, registerNumber, value);
    }
}