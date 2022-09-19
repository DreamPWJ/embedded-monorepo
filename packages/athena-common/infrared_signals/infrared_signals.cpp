#include "infrared_signals.h"
#include <Arduino.h>
#include <IRremote.h>

/**
* @author 潘维吉
* @date 2022/9/19 9:50
* @description 红外信号发送接受数据
*/

IRrecv irrecv(RECV_PIN);

decode_results results; // 注意这个数据类型

/**
 * 初始化
 */
void init(void) {
    irrecv.enableIRIn(); // 初始化红外接收器
}

/**
 * 获取红外数据
 */
void get_data(void) {
    if (irrecv.decode(&results)) {  // 注意这里取解码结果的方法，传递的是变量指针。
        Serial.println(results.value, HEX); // 以16进制换行输出接收代码。注意取值方法【results.value】。
        irrecv.resume(); // 给红外传送指令，让其继续接收下一个值。
    }
}
