#include "infrared_signals.h"
#include <Arduino.h>
#include <IRremote.hpp>

/**
* @author 潘维吉
* @date 2022/9/19 9:50
* @description 红外信号发送接收数据
*/


/**
 * 初始化
 */
void init(void) {
    IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK); // Start the receiver // 初始化红外接收器
}

/**
 * 获取红外数据
 */
void get_data(void) {
    if (IrReceiver.decode()) {  // 注意这里取解码结果的方法，传递的是变量指针。
        Serial.println(IrReceiver.decodedIRData.decodedRawData,
                       HEX); // Print raw data  以16进制换行输出接收代码
        IrReceiver.printIRResultShort(&Serial); // Print complete received data in one line
        IrReceiver.printIRSendUsage(&Serial);   // Print the statement required to send this data
        IrReceiver.resume(); // 给红外传送指令，让其继续接收下一个值。
    }
}
