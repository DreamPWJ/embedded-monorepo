#include <Arduino.h>
#include <common.h>
#include <at_mqtt/at_mqtt.h>
#include <nb_iot.h>
#include <mcu_nvs.h>
#include <ota.h>
#include <pwm.h>
#include <ground_feeling.h>

/**
* @author 潘维吉
* @date 2023/12/15 9:41
* @description 程序运行入口
*/

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

    // 初始化非易失性存储
    int_nvs();
    // key关键字与系统默认内置关键字冲突 会导致存储失败
    set_nvs("version", FIRMWARE_VERSION);

#if WIFI_EN
    // 初始化WiFi无线网络
    init_wifi();
#else
    // 初始化NB-IoT网络协议
    init_nb_iot();
#endif

#if PWM_EN
    // 初始化电机马达
    init_motor();
#endif

#if MQTT_EN
    // 初始化MQTT消息协议
    init_at_mqtt();
    // WiFi网络版本初始化MQTT消息协议
    // init_mqtt();
#endif


    // OTA升级配置文件  如果https证书有问题 可以使用http协议
    std::string const &ota_temp_json = std::string("http://") + std::string(STR(FIRMWARE_UPDATE_JSON_URL));
    const char *firmware_update_json_url = ota_temp_json.c_str();

    // WiFi网络版本执行OTA空中升级
    // exec_ota(FIRMWARE_VERSION, firmware_update_json_url);
    // WIFI要供电稳定 保证电压足够 才能正常工作
    do_firmware_upgrade(FIRMWARE_VERSION, firmware_update_json_url, "");

}

void loop() {
// write your code here
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
