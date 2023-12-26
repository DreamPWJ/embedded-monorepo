#include <Arduino.h>
#include <wifi_network.h>
#include <mqtt.h>
#include <bluetooth_connect.h>
#include <LibDemo.h>
#include <ota.h>
// #include <log_insight.h>

/**
* @author 潘维吉
* @date 2022/7/20 9:41
* @description 程序运行入口
*/
using namespace std;

// 获取自定义多环境变量宏定义
#define XSTR(x) #x
#define STR(x) XSTR(x)

#define FIRMWARE_VERSION              "CI_OTA_FIRMWARE_VERSION"  // 版本号用于OTA升级和远程升级文件对比 判断是否有新版本 每次需要OTA的时候更改设置 CI_OTA_FIRMWARE_VERSION关键字用于CI替换版本号

static const char *TAG = "esp32_demo";

void setup() {
// write your initialization code here
    // 初始化设置代码  为保证单片机运行正常  电路设计与电压必须稳定

    // 设置UART串口波特率
    Serial.begin(115200);

    Serial.println("ESP32 C3 MCU");

    // PlatformIO自定义库 用于项目应用示例
    init_demo();

    String project_name = STR(PROJECT_NAME);
    Serial.println(project_name);
    String projectName = "esp32-c3-demo";
    if (project_name == projectName) {
        Serial.println("单片机工程匹配成功");
    }

    const char *env_name = STR(ENV_NAME);
    Serial.println(env_name);

    if (env_name == "app1") {
        Serial.println("App1环境工程");
    } else if (env_name == "app2") {
        Serial.println("App2环境工程");
    }
    const char *env_app_version = STR(ENV_APP_VERSION);
    Serial.println(env_app_version);
    const char *app_version = STR(APP_VERSION);
    Serial.println(app_version);
    const char *mqtt_broker = STR(MQTT_BROKER);
    Serial.println(mqtt_broker);

    Serial.println(STR(CORE_DEBUG_LEVEL));

    // 初始化设置蓝牙
    // init_bluetooth("panweiji");
    // 初始化WiFi无线网络
    init_wifi();
    // WiFi网络版本初始化MQTT消息协议
    init_mqtt();

    std::string const &ota_temp_json = std::string("http://") + std::string(STR(FIRMWARE_UPDATE_JSON_URL));
    const char *firmware_update_json_url = ota_temp_json.c_str();
    Serial.println(firmware_update_json_url);
    // WIFI要供电稳定 保证电压足够 才能正常工作
    // do_firmware_upgrade(FIRMWARE_VERSION, firmware_update_json_url, "");

    delay(1000);
    // 初始化日志云上报
//    init_insights();
//    ESP_LOGI(TAG, "初始化insights日志云上报");
//    ESP_LOGE(TAG, "初始化insights日志云上报, 错误日志");

}

void loop() {
// write your code here

// 定时检测重新连接WiFi
    reconnect_wifi();

// MQTT重连
    mqtt_reconnect();
// MQTT监听
    mqtt_loop();

    delay(1000);

}