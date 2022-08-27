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
void init_nb_iot() {
    // 给NB模组发送AT指令  NB模组出厂自带AT固件 接入天线
    Serial.println("AT"); // 测试AT指令
    Serial.println("AT+ECICCID\r\n"); // 查看SIM ID号
    Serial.println("AT+CGATT=1\r\n"); // 附着网络
    Serial.println("AT+CGDCONT=1,'IP','CMNET'\r\n"); // 注册APNID接入网络
    Serial.println("AT+CGACT=1\r\n"); // 激活网络
    Serial.println("AT+ECPING='www.baidu.com'\r\n"); // 测试网络
}