#include "radio_frequency.h"
#include <Arduino.h>
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

/**
* @author 潘维吉
* @date 2022/9/19 10:53
* @description 无线射频RF通信
* 参考文章: https://randomnerdtutorials.com/rf-433mhz-transmitter-receiver-module-with-arduino/
* RadioHead 库很棒，它适用于市场上几乎所有的射频模块
*/

#define USE_MULTI_CORE 0 // 是否使用多核 根据芯片决定
RH_ASK driver;

/**
 * 初始化
 */
void rf_init(void) {
    if (!driver.init()) {
        Serial.println("RF init failed");
#if !USE_MULTI_CORE
        const char *params = NULL;
        xTaskCreate(
                rf_accept_data,  /* Task function. */
                "rf_accept_data", /* String with name of task. */
                8192,      /* Stack size in bytes. */
                (void *) params,      /* Parameter passed as input of the task */
                3,         /* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) */
                NULL);     /* Task handle. */
#else
        //最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1,或者 tskNO_AFFINITY 允许任务在两者上运行.
    xTaskCreatePinnedToCore(rf_accept_data, "rf_accept_data", 8192, NULL, 10, NULL, 0);
#endif
    }
}

/**
 * 接受RF射频数据
 */
void rf_accept_data(void *pvParameters) {
    Serial.print("接受RF射频数据: ");
    uint8_t buf[24];
    uint8_t buf_len = sizeof(buf);
    if (driver.recv(buf, &buf_len)) // Non-blocking
    {
        int i;
        // Message with a good checksum received, dump it.
        Serial.print("Message: ");
        Serial.println((char *) buf);
    }
}

/**
 * 发送RF射频数据
 */
void rf_send_data(void) {
    const char *msg = "Hello World!";
    driver.send((uint8_t *) msg, strlen(msg));
    driver.waitPacketSent();
}
