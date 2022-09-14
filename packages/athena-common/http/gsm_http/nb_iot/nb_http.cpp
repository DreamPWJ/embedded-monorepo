#include "nb_http.h"

#include "defines.h"
#include <Arduino.h>

/**
* @author 潘维吉
* @date 2022/9/7 16:29
* @description NB-IoT网络类型的Http请求
* 参考文档： https://github.com/khoih-prog/NB_Generic
*/
// Please enter your sensitive data in the Secret tab or arduino_secrets.h
// PIN Number
const char PINNUMBER[]     = SECRET_PINNUMBER;
// initialize the library instance
NBClient client;
GPRS gprs;
NB nbAccess;

// BaudRate to communicate to NB-IoT modem. If be limit to max 115200 inside modem
unsigned long baudRateSerialNB = 9600;

char server[] = "arduino.cc";         //"example.org";
char path[] = "/asciilogo.txt";     //"/";
int port = 80; // port 80 is the default for HTTP

/**
 * NB网络Http的Get请求
 */
void nb_http_get() {
    Serial.println("NB网络Http的Get请求");
    Serial.print(F("\nStarting NBWebClient on "));
    Serial.println(BOARD_NAME);
    Serial.println(NB_GENERIC_VERSION);
#if (defined(DEBUG_NB_GENERIC_PORT) && (_NB_GENERIC_LOGLEVEL_ > 4))
    MODEM.debug(DEBUG_NB_GENERIC_PORT);
#endif
    // connection state
    boolean connected = false;
    // After starting the modem with NB.begin() using PINNUMBER
    // attach to the GPRS network
    while (!connected) {
        if ((nbAccess.begin(baudRateSerialNB, PINNUMBER) == NB_READY) && (gprs.attachGPRS() == GPRS_READY)) {
            connected = true;
        } else {
            Serial.println("Not connected");
            delay(1000);
        }
    }

    Serial.println("connecting...");

// if you get a connection, report back via serial:
    if (client.connect(server, port)) {
        Serial.println("connected");
        // Make a HTTP request:
        client.print("GET ");
        client.print(path);
        client.println(" HTTP/1.1");
        client.print("Host: ");
        client.println(server);
        client.println("Connection: close");
        client.println();
    } else {
        // if you didn't get a connection to the server:
        Serial.println("connection failed");
    }
}