#include "gsm_http.h"
#include <Arduino.h>

/**
* @author 潘维吉
* @date 2022/9/8 8:38
* @description GSM网络类型的Http请求
* 参考文档： https://randomnerdtutorials.com/esp32-sim800l-publish-data-to-cloud/
* https://github.com/vshymanskyy/TinyGSM
*/

// Your GPRS credentials (leave empty, if not needed)
const char apn[] = "CMNET"; // APN (example: internet.vodafone.pt) use https://wiki.apnchanger.org
const char gprsUser[] = ""; // GPRS User
const char gprsPass[] = ""; // GPRS Password

// SIM card PIN (leave empty, if not defined)
const char simPIN[] = "";

// Server details
// The server variable can be just a domain name or it can have a subdomain. It depends on the service you are using
const char server[] = "archive-artifacts-pipeline.oss-cn-shanghai.aliyuncs.com"; // domain name: example.com, maker.ifttt.com, etc
const char resource[] = "/iot/ground-lock/prod/ground-lockota.json";         // resource path, for example: /post-data.php
const int port = 80;

// 定义GSM网络模组管脚 pins
#define MODEM_RST            6
#define MODEM_PWKEY          10
//#define MODEM_POWER_ON       23
#define MODEM_TX             7
#define MODEM_RX             3

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to SIM800 module)
#define SerialAT Serial1

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800     // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb

// Define the serial console for debug prints, if needed
#define DUMP_AT_COMMANDS

// #include <Wire.h>
#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

// TinyGSM Client for Internet connection
TinyGsmClient client(modem);

/**
 * GSM网络Http的Get请求
 */
void gsm_http_get() {
    Serial.println("GSM网络Http的Get请求");
    SerialMon.begin(115200);
    /* // Set modem reset, enable, power pins
     pinMode(MODEM_PWKEY, OUTPUT);
     pinMode(MODEM_RST, OUTPUT);
    // pinMode(MODEM_POWER_ON, OUTPUT);
     digitalWrite(MODEM_PWKEY, LOW);
     digitalWrite(MODEM_RST, HIGH);*/
    // digitalWrite(MODEM_POWER_ON, HIGH);

    // Set GSM module baud rate and UART pins
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    delay(3000);
    // Restart SIM800 module, it takes quite some time
    // To skip it, call init() instead of restart()
    SerialMon.println("Initializing modem...");
    // use modem.init() if you don't need the complete restart
    modem.restart();
    String modemInfo = modem.getModemInfo();
    SerialMon.print("Modem Info: ");
    SerialMon.println(modemInfo);
    // Unlock your SIM card with a PIN if needed
/*    if (strlen(simPIN) && modem.getSimStatus() != 3 ) {
        modem.simUnlock(simPIN);
    }*/
    SerialMon.print("Connecting to APN: ");
    SerialMon.print(apn);
    yield(); // 处理和看门狗任务冲突

    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        SerialMon.println("GSM gprsConnect Fail");
    } else {
        SerialMon.println("GSM gprsConnect OK");
        SerialMon.print(" GSM Connecting to ");
        SerialMon.print(server);
        if (!client.connect(server, port)) {
            SerialMon.println(" connect Fail");
        } else {
            SerialMon.println(" connect OK");

            // Making an HTTP GET request
            SerialMon.println("Performing HTTP GET request...");
            // Prepare your HTTP GET request data
            String httpRequestData = "";
            //String httpRequestData = "api_key=tPmAT5Ab3j7F9&value1=24.75&value2=49.54&value3=1005.14";

            client.print(String("GET ") + resource + " HTTP/1.1\r\n");
            client.print(String("Host: ") + server + "\r\n");
            client.println("Connection: close");
            client.println("Content-Type: application/json");
            client.print("Content-Length: ");
            client.println(httpRequestData.length());
            client.println();
            client.println(httpRequestData);

            unsigned long timeout = millis();
            while (client.connected() && millis() - timeout < 10000L) {
                // Print available data (HTTP response from server)
                while (client.available()) {
                    char c = client.read();
                    SerialMon.print(c);
                    timeout = millis();
                }
            }
            SerialMon.println();

            // Close client and disconnect
            client.stop();
            SerialMon.println(F("Server disconnected"));
            modem.gprsDisconnect();
            SerialMon.println(F("GPRS disconnected"));
        }
    }
}