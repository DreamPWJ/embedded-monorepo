#include <Arduino.h>

// 获取自定义多环境变量宏定义
#define XSTR(x) #x
#define STR(x) XSTR(x)

void setup() {
// write your initialization code here
    // 初始化设置代码  为保证单片机运行正常  电路设计与电压必须稳定

    // 设置UART串口波特率
    Serial.begin(115200);
    delay(1000);
    Serial.println("ESP32 C3 MCU");
    const char* mqtt_version = STR(MQTT_VERSION);
    Serial.println(mqtt_version);
    const char* app_version = STR(APP_VERSION);
    Serial.println(app_version);
}

void loop() {
// write your code here
}