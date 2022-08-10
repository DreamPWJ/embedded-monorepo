#include "ota.h"
#include <Arduino.h>
#include <WebServer.h>

/**
* @author 潘维吉
* @date 2022/7/29 15:56
* @description 固件OTA空中升级
* 参考文档：https://blog.51cto.com/u_15284384/3054914
* https://github.com/chrisjoyce911/esp32FOTA
* https://github.com/ayushsharma82/AsyncElegantOTA
*/

// WebServer server(80);


/**
 * 执行OTA空中升级
 */
void exec_ota() {

}

/**
 * Web执行OTA空中升级
 */
/*
 void exec_web_ota() {
   server.on("/", []() {
        server.send(200, "text/plain", "Hi! I am ESP32.");
    });
    AsyncElegantOTA.begin(&server);    // Start ElegantOTA
    server.begin();
    Serial.println("HTTP server started"); // 访问 http://ip:port/update进行升级
}
 */