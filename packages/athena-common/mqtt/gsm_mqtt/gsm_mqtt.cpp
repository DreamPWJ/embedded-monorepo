#include "gsm_mqtt.h"
#include <Arduino.h>

/**
* @author 潘维吉
* @date 2022/9/8 14:44
* @description GSM网络类型的MQTT消息队列遥测传输协议
* 参考文档： https://randomnerdtutorials.com/esp32-cloud-mqtt-broker-sim800l/
* https://github.com/vshymanskyy/TinyGSM
*/

// Select your modem:
#define TINY_GSM_MODEM_SIM800 // Modem is SIM800L

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands
#define SerialAT Serial1

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon
// Your GPRS credentials (leave empty, if not needed)
const char apn[] = "CMNET"; // APN (example: internet.vodafone.pt) use https://wiki.apnchanger.org
const char gprsUser[] = ""; // GPRS User
const char gprsPass[] = ""; // GPRS Password

// SIM card PIN (leave empty, if not defined)
const char simPIN[] = "";

// MQTT details
const char *broker = "192.168.1.200";                    // Public IP address or domain name
const char *mqttUsername = "admin";  // MQTT username
const char *mqttPassword = "public";  // MQTT password
const char *topics = "ESP32/common";

// 定义GSM网络模组管脚 pins
#define MODEM_RST            6
#define MODEM_PWKEY          10
//#define MODEM_POWER_ON       23
#define MODEM_TX             7
#define MODEM_RX             8

// Select your modem:
#define TINY_GSM_MODEM_SIM800 // Modem is SIM800L

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands
#define SerialAT Serial1

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon


// Define the serial console for debug prints, if needed
//#define DUMP_AT_COMMANDS

// #include <Wire.h>
#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modemMqtt(SerialAT);
#endif

#include "PubSubClient.h"

TinyGsmClient clientMqtt(modemMqtt);
PubSubClient mqtt(clientMqtt);


void mqttCallback(char *topic, byte *message, unsigned int len) {
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String messageTemp;

    for (int i = 0; i < len; i++) {
        Serial.print((char) message[i]);
        messageTemp += (char) message[i];
    }
    Serial.println();

    // Feel free to add more if statements to control more GPIOs with MQTT

    // If a message is received on the topic esp/output1, you check if the message is either "true" or "false".
    // Changes the output state according to the message
    if (String(topic) == "ESP32/common") {

    }
}


/**
 * 初始化 GSM网络类型的MQTT协议
 */
boolean init_gsm_mqtt() {
    Serial.println("初始化 GSM网络类型的MQTT协议");
    SerialMon.begin(115200);
    // Set GSM module baud rate and UART pins
    SerialAT.begin(9200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    delay(6000);
    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    SerialMon.println("Initializing modem...");
    modemMqtt.restart();
    // modem.init();

    String modemInfo = modemMqtt.getModemInfo();
    SerialMon.print("Modem Info: ");
    SerialMon.println(modemInfo);

    // Unlock your SIM card with a PIN if needed
/*    if ( GSM_PIN && modem.getSimStatus() != 3 ) {
        modem.simUnlock(GSM_PIN);
    }*/

    SerialMon.print("Connecting to APN: ");
    SerialMon.print(apn);
    if (!modemMqtt.gprsConnect(apn, gprsUser, gprsPass)) {
        SerialMon.println(" fail");
        ESP.restart();
    } else {
        SerialMon.println(" OK");
    }

    if (modemMqtt.isGprsConnected()) {
        SerialMon.println("GPRS connected");
    }

    // MQTT Broker setup
    mqtt.setServer(broker, 1883);
    mqtt.setCallback(mqttCallback);

    SerialMon.print("Connecting to ");
    SerialMon.print(broker);

    // Connect to MQTT Broker without username and password
    //boolean status = mqtt.connect("GsmClientN");

    // Or, if you want to authenticate MQTT:
    boolean status = mqtt.connect("GsmClientN", mqttUsername, mqttPassword);
    if (status == false) {
        SerialMon.println(" fail");
        ESP.restart();
        return false;
    }
    SerialMon.println(" success");
    mqtt.subscribe(topics);
    return mqtt.connected();
}

/**
 * MQTT协议监听
 */
void gsm_mqtt_loop() {
    mqtt.loop();
}