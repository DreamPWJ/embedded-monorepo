#include "radio_frequency.h"
#include <Arduino.h>
#include <RCSwitch.h>
#include <pwm.h>
/*#include <RH_ASK.h>
#include <SPI.h> */// Not actually used but needed to compile

/**
* @author 潘维吉
* @date 2022/9/19 10:53
* @description 无线射频RF通信
* 参考文章: https://randomnerdtutorials.com/rf-433mhz-transmitter-receiver-module-with-arduino/
* https://randomnerdtutorials.com/esp8266-remote-controlled-sockets/
* RadioHead与rc-switch库很棒，它适用于市场上几乎所有的射频模块
*/

#define USE_MULTI_CORE 0 // 是否使用多核 根据芯片决定
#define RF_PIN 9  // RF射频接收引脚GPIO

// RH_ASK driver;
RCSwitch mySwitch = RCSwitch();

// Replace with your remote TriState values
char *triStateOn = "5933330";
char *triStateOff = "5933336";


/**
 * 初始化
 */
void rf_init(void) {
/*    if (!driver.init()) {
        Serial.println("RF init failed"); */
    mySwitch.enableReceive(RF_PIN);  // Receiver on inerrupt 0 => that is pin
#if !USE_MULTI_CORE
    const char *params = NULL;
    xTaskCreate(
            rf_accept_data,  /* Task function. */
            "rf_accept_data", /* String with name of task. */
            8192,      /* Stack size in bytes. */
            (void *) params,      /* Parameter passed as input of the task */
            6,         /* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) */
            NULL);     /* Task handle. */
#else
    //最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1,或者 tskNO_AFFINITY 允许任务在两者上运行.
xTaskCreatePinnedToCore(rf_accept_data, "rf_accept_data", 8192, NULL, 10, NULL, 0);
#endif

    // }
}

/**
 * 接收RF射频数据
 */
void rf_accept_data(void *pvParameters) {
    while (1) {  // RTOS多任务条件： 1. 不断循环 2. 无return关键字
        if (mySwitch.available()) {
            Serial.print("接收RF无线射频数据: ");
/*   output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(), mySwitch.getReceivedProtocol()); */
            unsigned long code = mySwitch.getReceivedValue(); // 固定码 可根据芯片id生成并存储到NVS中
            Serial.println(code); // 使用10进制
            // Serial.println(code, HEX); // 使用16进制
/*            if (String(code) == String(triStateOn)) {
                //Serial.println("车位锁抬起");
                set_motor_up();
            } else if (String(code) == String(triStateOff)) {
                //Serial.println("车位锁下降");
                set_motor_down();
            }*/
            mySwitch.resetAvailable();
        }
        delay(600); // 多久执行一次 毫秒
    }
/*    uint8_t buf[24];
    uint8_t buf_len = sizeof(buf);
    if (driver.recv(buf, &buf_len)) // Non-blocking
    {
        int i;
        // Message with a good checksum received, dump it.
        Serial.print("Message: ");
        Serial.println((char *) buf);
    }*/
}

/**
 * 发送RF射频数据
 */
void rf_send_data(void) {
    //  mySwitch.sendTriState(triStateOn);

/*  const char *msg = "Hello World!";
    driver.send((uint8_t *) msg, strlen(msg));
    driver.waitPacketSent();
    */
}
