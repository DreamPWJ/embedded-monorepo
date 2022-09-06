#include "nb_iot.h"
#include <Arduino.h>
//#include <MKRGSM.h>
#include <SoftwareSerial.h>

#define PIN_TX 7
#define PIN_RX 8

// Set up a new SoftwareSerial object
SoftwareSerial mySerial;

/**
* @author 潘维吉
* @date 2022/8/22 14:44
* @description NB-IoT物联网网络协议
*/

// modem verification object
// GSMModem modem;

// NB控制GPIO
const int nb_gpio_1 = 6;
const int nb_gpio_2 = 10;


/**
 * 初始化NB网络协议
 */
void init_nb_iot() {
    // NB相关引脚初始化
    pinMode(nb_gpio_1, OUTPUT);
    pinMode(nb_gpio_2, OUTPUT);
    digitalWrite(nb_gpio_1, HIGH);
    digitalWrite(nb_gpio_2, HIGH);

    pinMode(PIN_RX, INPUT);
    pinMode(PIN_TX, OUTPUT);

    // 参考文档： https://github.com/plerup/espsoftwareserial
    mySerial.begin(9600, SWSERIAL_8N1, -1, PIN_TX, false); // NB模组的波特率
    if (!mySerial) { // If the object did not initialize, then its configuration is invalid
        Serial.println("Invalid SoftwareSerial pin configuration, check config");
        while (1) { // Don't continue with invalid configuration
            Serial.print(".");
            delay(1000);
        }
    }
    // 给NB模组发送AT指令  NB模组出厂自带AT固件 接入天线  参考文章: https://aithinker.blog.csdn.net/article/details/120765734
    Serial.println("给NB模组发送AT指令");
    // mySerial.write("AT"); // 测试AT指令
    // delay(2000);
    mySerial.write("AT+ECICCID\r\n"); // 查看SIM ID号
    delay(2000);
    mySerial.write("AT+CGATT=1\r\n"); // 附着网络
    delay(2000);
/*    mySerial.write("AT+CGDCONT=1,'IP','CMNET'\r\n"); // 注册APNID接入网络
    delay(2000);
    mySerial.write("AT+CGACT=1\r\n"); // 激活网络
    delay(2000);
    mySerial.write("AT+ECPING='www.baidu.com'\r\n"); // 测试网络*/
}

/**
 * 获取国际移动设备唯一标识IMEI串号码
 */
String get_imei() {
    // return modem.getIMEI();
    return "";
}