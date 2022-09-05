#include "nb_iot.h"
#include <Arduino.h>
//#include <MKRGSM.h>

/**
* @author 潘维吉
* @date 2022/8/22 14:44
* @description NB-IoT物联网网络协议
*/

// modem verification object
//GSMModem modem;

/**
 * 初始化NB网络协议
 */
void init_nb_iot() {
    // NB相关引脚初始化
    pinMode(6, OUTPUT);
    pinMode(10, OUTPUT);
    digitalWrite(6, HIGH);
    digitalWrite(10, HIGH);

    // 给NB模组发送AT指令  NB模组出厂自带AT固件 接入天线
    //Serial.println("AT"); // 测试AT指令
    //Serial.println("AT+ECICCID\r\n"); // 查看SIM ID号
    Serial.println("AT+CGATT=1\r\n"); // 附着网络
    Serial.println("AT+CGDCONT=1,'IP','CMNET'\r\n"); // 注册APNID接入网络
    Serial.println("AT+CGACT=1\r\n"); // 激活网络
    //Serial.println("AT+ECPING='www.baidu.com'\r\n"); // 测试网络
}


/**
 * 获取国际移动设备唯一标识IMEI串号码
 */
String get_imei() {
    // return modem.getIMEI();
    return "";
}