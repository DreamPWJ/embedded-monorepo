#include <common.h>
#include <Arduino.h>
// #include <SoftwareSerial.h>
#include <BizConstants.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <led_pin.h>
#include <nb_iot.h>
#include <wifi_network.h>
#include <mqtt.h>
#include <pwm.h>
#include <ground_feeling.h>
#include <ota.h>
#include <chip_info.h>
#include <iostream>
#include <string>
#include <mcu_nvs.h>
#include <uart.h>
#include <device_info.h>
#include <http.h>
#include <at_mqtt/at_mqtt.h>
#include <at_http/at_http.h>
#include <infrared_signals.h>
#include <radio_frequency.h>
#include <json_utils.h>
#include <TimeUtil.h>
#include <common_utils.h>

using namespace std;
// 获取自定义多环境变量宏定义
#define XSTR(x) #x
#define STR(x) XSTR(x)

#define FIRMWARE_VERSION              "CI_OTA_FIRMWARE_VERSION"  // 版本号用于OTA升级和远程升级文件对比 判断是否有新版本 每次需要OTA的时候更改设置 CI_OTA_FIRMWARE_VERSION关键字用于CI替换版本号

#define WIFI_EN 0  // 是否开启WIFI网络功能 0 关闭  1 开启
#define MQTT_EN 1  // 是否开启MQTT消息协议 0 关闭  1 开启
#define PWM_EN 1   // 是否开启PWM脉冲宽度调制功能 0 关闭  1 开启
#define IS_DEBUG false  // 是否调试模式

// This sets Arduino Stack Size - comment this line to use default 8K stack size
SET_LOOP_TASK_STACK_SIZE(16 * 1024); // 16KB

/*void IRAM_ATTR isr() {
    Serial.println("进入外部中断了");
}*/

void setup() {
    // 初始化设置代码  为保证单片机运行正常  电路设计与电压必须稳定

    // 设置UART串口波特率
    Serial.begin(115200);

    // 初始化其它UART串口通信
    // init_uart();
    Serial1.begin(9600, SERIAL_8N1, PIN_RX, PIN_TX);
    if (!Serial1) { // If the object did not initialize, then its configuration is invalid
        Serial.println("Invalid Serial1 pin configuration, check config");
        while (1) { // Don't continue with invalid configuration
            Serial.print(".");
            delay(1000);
        }
    }

/*    Serial2.begin(9600, SERIAL_8N1, PIN_RX_2, PIN_TX_2);
    if (!Serial2) { // If the object did not initialize, then its configuration is invalid
        Serial.println("Invalid Serial2 pin configuration, check config");
        while (1) { // Don't continue with invalid configuration
            Serial.print(".");
            delay(1000);
        }
    }*/

#if IS_DEBUG
    // 将LED数字引脚初始化为输出
    // set_pin_mode();
#endif

    // 初始化非易失性存储
    int_nvs();
    // key关键字与系统默认内置关键字冲突 会导致存储失败
    set_nvs("version", FIRMWARE_VERSION);
    // printf(APP_VERSION);

    // 初始化温度信息
    // init_temperature();

    // 常量与工具类调用示例
    // const BizConstants bizConstants;
    // Serial.println(bizConstants.NAME.c_str());

#if WIFI_EN
    // 初始化WiFi无线网络
    init_wifi();
#else
    // 初始化NB-IoT网络协议
    init_nb_iot();
#endif

    // Serial.println(TimeUtil::getDateTime().c_str());

    // WiFi网络版本HTTP请求
    // http_get("http://tcc.taobao.com/cc/json/mobile_tel_segment.htm?tel=18863302302");
    // AT指令网络版本HTTP请求
    // at_http_get("archive-artifacts-pipeline.oss-cn-shanghai.aliyuncs.com/iot/ground-lock/prod/ground-lockota.json");

#if PWM_EN
    // 初始化电机马达
    init_motor();
    // 初始化地感线圈
    init_ground_feeling();
    // 检测地感状态 有车无车及时上报MQTT服务器
    check_ground_feeling_status();
#endif

#if MQTT_EN
    // 初始化MQTT消息协议
    init_at_mqtt();

#if IS_DEBUG
    // 上报启动信息 用于调试查看
    DynamicJsonDocument doc(200);
    string chip_id = to_string(get_chip_mac());
    doc["type"] = "initMCU";
    doc["msg"] = "你好, 我是" + chip_id + "单片机MCU嵌入式程序开始启动消息";
    String initStr;
    serializeJson(doc, initStr);
    std::string mcu_topic = "ESP32/common"; // + chip_id
    at_mqtt_publish(mcu_topic.c_str(), initStr.c_str());
#endif

    // WiFi网络版本初始化MQTT消息协议
    // init_mqtt();

    // WiFi网络版本MQTT心跳服务
    // mqtt_heart_beat();
#endif

    // 初始化无线射频RF 用于遥控器控制
    // rf_init();

    // OTA升级配置文件  如果https证书有问题 可以使用http协议
    std::string const& ota_temp_json = std::string("http://") + std::string(STR(FIRMWARE_UPDATE_JSON_URL));
    const char *firmware_update_json_url = ota_temp_json.c_str();

    // WiFi网络版本执行OTA空中升级
    // exec_ota(FIRMWARE_VERSION, firmware_update_json_url);
    // WIFI要供电稳定 保证电压足够 才能正常工作
    do_firmware_upgrade(FIRMWARE_VERSION, firmware_update_json_url, "");

/*  pinMode(19, INPUT_PULLUP);
    attachInterrupt(19, isr, FALLING); */

#if PWM_EN
    // 初始化抬起车位锁
    set_motor_up();
#endif

}

void loop() {
    // 循环执行代码
#if IS_DEBUG
    // 开发板LED 闪动的实现 影响serialEvent运行
    // set_led();
    // Print unused stack for the task that is running loop() - the same as for setup()
    // Serial.printf("\nLoop() - Free Stack Space: %d", uxTaskGetStackHighWaterMark(NULL));
    // Serial.printf("电池电量值: %f\n", get_electricity());
#endif

#if WIFI_EN
    // 定时检测重新连接WiFi
    reconnect_wifi();
#endif

#if MQTT_EN
    // WiFi网络版本MQTT消息服务
    // mqtt_loop();
#endif

#if PWM_EN
    // 驱动电机马达工作
    // set_pwm();
#endif

}

/**
 * UART1串口中断入口
 */
void serialEvent1() {
    // serialEvent()作为串口中断回调函数，需要注意的是，这里的中断与硬件中断有所不同，这个回调函数只会在loop()执行完后才会执行，所以在loop()里的程序不能写成阻塞式的，只能写成轮询式的
    // 使用串口中断机制 外设发出的中断请求 您无需不断检查引脚的当前值。使用中断，当检测到更改时，会触发事件（调用函数) 无需循环检测)。 持续监控某种事件、时效性和资源使用情况更好
    // RTOS多线程内while不断获取串口数据和系统看门狗冲突 导致随机性UART接收数据部分乱码和丢失 建议使用串口中断方式获取串口数据
    String rxData1 = "";
    while (Serial1.available()) {
        // Serial.println("serialEvent()作为串口中断回调函数");
        char inChar = char(Serial1.read());
        rxData1 += inChar;
        delay(2); // 这里不能去掉，要给串口处理数据的时间
        /* if (inChar == '\n') { // 换行符 表示一个完整数据结束
             stringComplete = true;
        } */
    }
#if IS_DEBUG
    Serial.println("------------------------------------");
    Serial.println(rxData);
    Serial.println("************************************");
    /*    vector<string> dataArray = split(rxData.c_str(), "\\n");
    for (int i = 0; i < dataArray.size(); i++) {
        Serial.println("----------------DataArray--------------------");
        Serial.println(dataArray[i].c_str());
    }*/
#endif

    // MQTT订阅消息
    at_mqtt_callback(rxData1);
}

/**
 * UART2串口中断入口
 */
/*void serialEvent2() {
    String rxData2 = "";
    while (Serial2.available()) {
        // Serial.println("serialEvent()作为串口中断回调函数");
        rxData2 += char(Serial2.read());
        delay(2); // 这里不能去掉，要给串口处理数据的时间
    }
#if true
    Serial.println("------------------Serial2------------------");
    Serial.println(rxData2);
    Serial.println("******************Serial2******************");
#endif
    // UART2串口响应数据处理
    // uart_check_car(rxData2);
}*/

/**
 * UART0串口中断入口
 */
/*void serialEvent() {
    String rxData = "";
    while (Serial.available()) {
        // Serial.println("serialEvent()作为串口中断回调函数");
        rxData += char(Serial.read());
        delay(2); // 这里不能去掉，要给串口处理数据的时间
    }
#if IS_DEBUG
    Serial.println("------------------------------------");
    Serial.println(rxData);
    Serial.println("************************************");
#endif
}*/

