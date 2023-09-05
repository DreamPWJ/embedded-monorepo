#include "infrared_signals.h"
#include <Arduino.h>
#include <IRremote.hpp>

/**
* @author 潘维吉
* @date 2022/9/19 9:50
* @description 红外信号发送接收数据
*/

#define USE_MULTI_CORE 1 // 是否使用多核 根据芯片决定

/**
 * 获取红外数据
 */
void get_ir_data(void *pvParameters) {
    while (1) {  // RTOS多任务条件： 1. 不断循环 2. 无return关键字
        if (IrReceiver.decode()) {  // 注意这里取解码结果的方法，传递的是变量指针。
            Serial.print("接收红外信号数据: ");
            Serial.println(IrReceiver.decodedIRData.decodedRawData,
                           HEX); // Print raw data  以16进制换行输出接收代码
            IrReceiver.printIRResultShort(&Serial); // Print complete received data in one line
            IrReceiver.printIRSendUsage(&Serial);   // Print the statement required to send this data
            IrReceiver.resume(); // 给红外传送指令，让其继续接收下一个值。
        }
        delay(1000); // 多久执行一次 毫秒
    }
}

/**
 * 初始化
 */
void init_ir(void) {
    IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK); // Start the receiver // 初始化红外接收器
#if !USE_MULTI_CORE
    const char *params = NULL;
    xTaskCreate(
            get_ir_data,  /* Task function. */
            "get_ir_data", /* String with name of task. */
            1024 * 8,      /* Stack size in bytes. */
            (void *) params,      /* Parameter passed as input of the task */
            6,         /* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) */
            NULL);     /* Task handle. */
#else
    //最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1,或者 tskNO_AFFINITY 允许任务在两者上运行.
    xTaskCreatePinnedToCore(get_ir_data, "get_ir_data", 1024 * 2, NULL, 5, NULL, 0);
#endif
}

