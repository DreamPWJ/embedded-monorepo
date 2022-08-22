#include "nb_iot.h"
#include <Arduino.h>

/**
* @author 潘维吉
* @date 2022/8/22 14:44
* @description NB-IoT物联网网络协议
*/


/**
 * 初始化NB网络协议
 */
void init_NB() {
    Serial.println("AT+GMR");
}