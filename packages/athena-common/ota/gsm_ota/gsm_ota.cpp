#include "gsm_ota.h"
#include <Arduino.h>
#include <Update.h>
#include <ArduinoJson.h>
#include "freertos/task.h"
#include <at_http/at_http.h>
#include <version_utils.h>
#include "FS.h"
#include "SPIFFS.h"
#include <SoftwareSerial.h>

/**
* @author 潘维吉
* @date 2022/9/16 13:59
* @description GSM网络OTA空中升级
* 参考地址: https://github.com/Xinyuan-LilyGO/LilyGo-T-Call-SIM800/issues/132
*/

#define PIN_RX 19
#define PIN_TX 18
SoftwareSerial myOTASerial(PIN_RX, PIN_TX);

uint32_t knownFileSize = 1024;

void appendFile(fs::FS &fs, const char *path, const char *message) {
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for appending");
        return;
    }
    if (file.print(message)) {
        Serial.println("APOK");
    } else {
        Serial.println("APX");
    }
}

void readFile(fs::FS &fs, const char *path) {
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if (!file || file.isDirectory()) {
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while (file.available()) {
        Serial.write(file.read());
        delayMicroseconds(100);
    }
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.print(message)) {
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
}


void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels) {
                listDir(fs, file.name(), levels - 1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void deleteFile(fs::FS &fs, const char *path) {
    Serial.printf("Deleting file: %s\n", path);
    if (fs.remove(path)) {
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void performUpdate(Stream &updateSource, size_t updateSize) {
    if (Update.begin(updateSize)) {
        size_t written = Update.writeStream(updateSource);
        if (written == updateSize) {
            Serial.println("Writes : " + String(written) + " successfully");
        } else {
            Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
        }
        if (Update.end()) {
            Serial.println("OTA finished!");
            if (Update.isFinished()) {
                Serial.println("Restart ESP device!");
                ESP.restart();
            } else {
                Serial.println("OTA not fiished");
            }
        } else {
            Serial.println("Error occured #: " + String(Update.getError()));
        }
    } else {
        Serial.println("Cannot beggin update");
    }
}


void printPercent(uint32_t readLength, uint32_t contentLength) {
    // If we know the total length
    if (contentLength != -1) {
        Serial.print("\r ");
        Serial.print((100.0 * readLength) / contentLength);
        Serial.print('%');
    } else {
        Serial.println(readLength);
    }
}


void updateFromFS() {
    File updateBin = SPIFFS.open("/update.bin");
    if (updateBin) {
        if (updateBin.isDirectory()) {
            Serial.println("Directory error");
            updateBin.close();
            return;
        }

        size_t updateSize = updateBin.size();

        if (updateSize > 0) {
            Serial.println("Starting update");
            performUpdate(updateBin, updateSize);
        } else {
            Serial.println("Error, archivo vacío");
        }

        updateBin.close();

        // whe finished remove the binary from sd card to indicate end of the process
        //fs.remove("/update.bin");
    } else {
        Serial.println("no such binary");
    }
}

/**
 * 执行固件升级 1. 定时检测HTTP方式 2. 主动触发MQTT方式
 */
void do_gsm_firmware_upgrade(String version, String jsonUrl) {
    Serial.println("GSM网络OTA空中升级...");

    delay(3000);
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    SPIFFS.format();
    listDir(SPIFFS, "/", 0);

    Serial.println("Reading header");
    uint32_t contentLength = knownFileSize;

    File file = SPIFFS.open("/update.bin", FILE_APPEND);

    // 读取OTA升级文件 JSON数据
    DynamicJsonDocument json = at_http_get(jsonUrl);
    // Serial.println("OTA响应数据:");
    String new_version = json["version"].as<String>();
    String file_url = json["file"].as<String>();

    long timeout = millis();
/*    while (1) {
        String line = myOTASerial.readStringUntil('\n');
        line.trim();
        //Serial.println(line);    // Uncomment this to show response header
        line.toLowerCase();
        if (line.startsWith("content-length:")) {
            contentLength = line.substring(line.lastIndexOf(':') + 1).toInt();
        } else if (line.length() == 0) {
            break;
        }
    }*/

    timeout = millis();
    uint32_t readLength = 0;

    unsigned long timeElapsed = millis();
    printPercent(readLength, contentLength);

/*    while (readLength < contentLength && millis() - timeout < 10000L) {
        int i = 0;
        while (myOTASerial.available()) {
            // read file data to spiffs
            if (!file.print(char(myOTASerial.read()))) {
                Serial.println("Appending file");
            }
            //Serial.print((char)c);       // Uncomment this to show data
            readLength++;
            if (readLength % (contentLength / 13) == 0) {
                printPercent(readLength, contentLength);
            }
            timeout = millis();
        }
    }*/


    file.close();

    printPercent(readLength, contentLength);
    timeElapsed = millis() - timeElapsed;

   // float duration = float(timeElapsed) / 1000;

/*    Serial.println("Se genera una espera de 3 segundos");
    for (int i = 0; i < 3; i++) {
        Serial.print(String(i) + "...");
        delay(1000);
    }*/

    //readFile(SPIFFS, "/update.bin");

    updateFromFS();

    // 检测到新版本或者指定设备才进行OTA空中升级
/*    if (new_version != "null" && version_compare(new_version, version) == 1) {
        // 做固件MD5签名算法 保证固件本身是安全的
        printf("当前固件版本v%s, 有新v%s版本OTA固件, 正在下载... \n", version.c_str(), new_version.c_str());
        if (1 == 1) {
            // 检测固件是否正常  设计失败恢复方案 如果固件启动失败回滚
            Serial.println("执行OTA空中升级成功了, 重启单片机...");
        } else {
            Serial.println("执行OTA空中升级失败");
        }
    } else {
        printf("当前固件版本v%s, 没有检测到新版本OTA固件, 跳过升级 \n", version.c_str());
    }*/
}