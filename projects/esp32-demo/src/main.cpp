#include <Arduino.h>
#include <nb_iot.h>
#include "../../../packages/athena-common/led_pin/led_pin.h"
#include "../../../packages/athena-common/chip_info/chip_info.h"
#include "../lib/bluetooth/bluetooth_connect.h"
#include "../lib/wifi/wifi_network.h"
#include "../../../packages/athena-common/http/http.h"
#include "../lib/aliyun/aliyun_iot.h"
#include "../lib/ota/ota.h"
#include "../lib/pwm/pwm.h"

#define PWM_EN 0 // 是否开启PWM脉冲宽度调制
int interruptCounter = 0;
hw_timer_t *timer = NULL;

//	函数名称：onTimer()
//	函数功能：中断服务的功能，它必须是一个返回void（空）且没有输入参数的函数
//  为使编译器将代码分配到IRAM内，中断处理程序应该具有 IRAM_ATTR 属性
//  https://docs.espressif.com/projects/esp-idf/zh_CN/release-v4.3/esp32/api-reference/storage/spi_flash_concurrency.html
void IRAM_ATTR TimerEvent() {
    Serial.println(interruptCounter++);
    if (interruptCounter > 5) {
        interruptCounter = 1;
    }
}

void setup() {
// write your initialization code here
    Serial.begin(115200);
    // while (Serial.available()) {  // 等待串口连接成功
    Serial.println("串口连接成功");
    // 初始化日志上报
   // init_insights();
    // 将 LED 数字引脚初始化为输出
    set_pin_mode();
    // 初始化NB-IoT网络协议
    init_NB();
    // 初始化蓝牙设置
    // init_bluetooth("ESP32-PanWeiJi");
    // 初始化Wifi无线网络
    init_wifi();
    // FreeRTOS实时系统多线程处理  Create a connection task with 8kB stack on core 0
    // xTaskCreatePinnedToCore(init_wifi_multi_thread, "WiFiTask", 8192, NULL, 3, NULL, 0);
    // 网络请求
    //  http_get("https://tcc.taobao.com/cc/json/mobile_tel_segment.htm?tel=18863302302");
/*    http_post(
            "https://oapi.dingtalk.com/robot/send?access_token=383391980b120c38f0f9a4a398349739fa67a623f9cfa834df9c5374e81b2081",
            "");*/
    // init_aliyun_iot_sdk();
    // }
    // OTA空中升级
    exec_ota();

#if PWM_EN
    init_motor();
#endif

    //	函数名称：timerBegin()
    //	函数功能：Timer初始化，分别有三个参数
    //	函数输入：1. 定时器编号（0到3，对应全部4个硬件定时器）
    //			 2. 预分频器数值（ESP32计数器基频为80M，80分频单位是微秒）
    //			 3. 计数器向上（true）或向下（false）计数的标志
    //	函数返回：一个指向 hw_timer_t 结构类型的指针
    timer = timerBegin(0, 80, true);

    //	函数名称：timerAttachInterrupt()
    //	函数功能：绑定定时器的中断处理函数，分别有三个参数
    //	函数输入：1. 指向已初始化定时器的指针（本例子：timer）
    //			 2. 中断服务函数的函数指针
    //			 3. 表示中断触发类型是边沿（true）还是电平（false）的标志
    //	函数返回：无
    timerAttachInterrupt(timer, &TimerEvent, true);

    //	函数名称：timerAlarmWrite()
    //	函数功能：指定触发定时器中断的计数器值，分别有三个参数
    //	函数输入：1. 指向已初始化定时器的指针（本例子：timer）
    //			 2. 第二个参数是触发中断的计数器值（1000000 us -> 1s）
    //			 3. 定时器在产生中断时是否重新加载的标志
    //	函数返回：无
    timerAlarmWrite(timer, 1000000, true);
    timerAlarmEnable(timer); //	使能定时器
}

void loop() {
// write your code here
    // Serial.println("PlatformIO And Arduino For Embedded ESP32! \n");
    // 获取硬件信息
    // get_chip_info();
    delay(2000);
    // 开发板LED 闪动的实现
    set_led();
    // 监听蓝牙状态
    // bluetooth_state();
    // 定时检测重新连接WiFi
    reconnect_wifi();
    // 检测OTA
    // check_ota();

#if PWM_EN
    pwm_set_duty(200 * interruptCounter, 200 * interruptCounter);
#endif

}