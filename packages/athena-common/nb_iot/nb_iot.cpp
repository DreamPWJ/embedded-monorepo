#include "nb_iot.h"
#include <Arduino.h>
#include <string.h>
//#include <MKRGSM.h>
#include <SoftwareSerial.h>

using namespace std;

#define PIN_RX 19
#define PIN_TX 7

// Set up a new SoftwareSerial object
SoftwareSerial mySerial(PIN_RX, PIN_TX);
//SoftwareSerial mySerial(PIN_RX, PIN_TX);

/**
* @author 潘维吉
* @date 2022/8/22 14:44
* @description NB-IoT物联网网络协议
*/

// modem verification object
// GSMModem modem;

// NB控制GPIO
#define MODEM_RST            6
#define MODEM_PWKEY          10


/**
 * 初始化NB网络协议
 */
void init_nb_iot() {
    // NB相关引脚初始化
    pinMode(MODEM_RST, OUTPUT);
    pinMode(MODEM_PWKEY, OUTPUT);
    digitalWrite(MODEM_RST, HIGH);
    digitalWrite(MODEM_PWKEY, HIGH);

    pinMode(PIN_RX, INPUT);
    pinMode(PIN_TX, OUTPUT);
    // 参考文档： https://github.com/plerup/espsoftwareserial
    //mySerial.begin(9600, SWSERIAL_8N1, PIN_RX, PIN_TX, false); // NB模组的波特率
    mySerial.begin(9600);
    if (!mySerial) { // If the object did not initialize, then its configuration is invalid
        Serial.println("Invalid SoftwareSerial pin configuration, check config");
        while (1) { // Don't continue with invalid configuration
            Serial.print(".");
            delay(1000);
        }
    }
    // 给NB模组发送AT指令  NB模组出厂自带AT固件 接入天线  参考文章: https://aithinker.blog.csdn.net/article/details/120765734
    Serial.println("给NB模组发送AT指令");
/*    // mySerial.write("AT\r\n"); // 测试AT指令
    delay(3000);
    mySerial.write("AT+ECICCID\r\n"); // 查看SIM ID号
    delay(1000);
    mySerial.write("AT+CGATT=1\r\n"); // 附着网络
    delay(1000);
    mySerial.write("AT+CGDCONT=1,\042IP\042,\042CMNET\042\r\n"); // 注册APNID接入网络
    delay(1000);
    mySerial.write("AT+CGACT=1\r\n"); // 激活网络
    delay(1000);
    //mySerial.write("AT+ECPING=\042www.baidu.com\042\r\n"); // 测试网络*/

    // 安信可NB-IoT的AT指令文档: https://docs.ai-thinker.com/_media/nb-iot/nb-iot%E7%B3%BB%E5%88%97%E6%A8%A1%E7%BB%84at%E6%8C%87%E4%BB%A4%E9%9B%86v1.0.pdf
    delay(1000);
    mySerial.write("AT+ECDNS=\042archive-artifacts-pipeline.oss-cn-shanghai.aliyuncs.com\042\r\n"); // DNS解析测试
    delay(1000);
    mySerial.write(
            "AT+HTTPCREATE=0,\042http://archive-artifacts-pipeline.oss-cn-shanghai.aliyuncs.com:80\042\r\n"); // 创建实例
    delay(1000);
    mySerial.write("AT+HTTPCON=0\r\n"); // 连接服务器
    delay(1000);
    string path = "/iot/ground-lock/prod/ground-lockota.json";
    mySerial.write("AT+HTTPSEND=0,0,41,\042/iot/ground-lock/prod/ground-lockota.json\042\r\n"); // Http请求
    String buffer;
    char incomingByte;
    // Now we simply display any text that the GSM shield sends out on the serial monitor
    if (mySerial.available() > 0) {
        Serial.println("=========================");
        //for (int i = 0; i <= 10; i++) {
        incomingByte = mySerial.read();
        Serial.println(incomingByte);
        buffer.concat(incomingByte);
        delay(10);
        // }
    }
    Serial.println(buffer);
}

/**
 * 获取国际移动设备唯一标识IMEI串号码
 */
String get_imei() {
    // return modem.getIMEI();
    return "";
}