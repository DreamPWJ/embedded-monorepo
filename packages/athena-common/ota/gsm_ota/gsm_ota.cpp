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
* 参考地址: https://github.com/espressif/esp-idf/tree/master/examples/system/ota
* https://github.com/espressif/esp-bootloader-plus/blob/master/README_CN.md
* https://github.com/espressif/arduino-esp32/tree/master/libraries/Update/examples
* https://github.com/Xinyuan-LilyGO/LilyGo-T-Call-SIM800/issues/132
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
            Serial.println("Error, empty file");
        }

        updateBin.close();

        // whe finished remove the binary from sd card to indicate end of the process
        //fs.remove("/update.bin");
    } else {
        Serial.println("no such binary");
    }
}

/**
 * 执行固件升级
 * 1. 定时检测HTTP方式 2. 主动触发MQTT方式
 * 1. 整包升级 2. 差分包升级(自定义bootloader实现)
 */
void do_gsm_firmware_upgrade(String version, String jsonUrl) {
    Serial.println("GSM网络OTA空中升级...");

    delay(3000);
/*    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    } else {
        Serial.println("SPIFFS is Success");
    }

    SPIFFS.format();
    listDir(SPIFFS, "/", 0);

    Serial.println("Reading header");
    uint32_t contentLength = knownFileSize;

    File file = SPIFFS.open("/update.bin", FILE_APPEND);*/

    // 读取OTA升级文件 JSON数据
    DynamicJsonDocument json = at_http_get(jsonUrl);
    // Serial.println("OTA响应数据:");
    String new_version = json["version"].as<String>();
    String file_url = json["file"].as<String>();
    // String md5 = json["md5"].as<String>();  // 升级json文件中的原始md5值和http请求头中Content-MD5的md5值保持一致

    Serial.println(new_version);
    Serial.println(file_url);

    if (file_url != "null") {
        Serial.println("下载升级固件...");
        at_http_get(file_url, true);
    }
/*

    long timeout = millis();
    while (1) {
        String line = myOTASerial.readStringUntil('\n');
        line.trim();
        //Serial.println(line);    // Uncomment this to show response header
        line.toLowerCase();
        if (line.startsWith("content-length:")) {
            contentLength = line.substring(line.lastIndexOf(':') + 1).toInt();
        } else if (line.length() == 0) {
            break;
        }
    }

    timeout = millis();
    uint32_t readLength = 0;

    unsigned long timeElapsed = millis();
    printPercent(readLength, contentLength);

    while (readLength < contentLength && millis() - timeout < 10000L) {
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
    }

    file.close();
*/

    // printPercent(readLength, contentLength);
    // timeElapsed = millis() - timeElapsed;

    // float duration = float(timeElapsed) / 1000;

/*    Serial.println("Se genera una espera de 3 segundos");
    for (int i = 0; i < 3; i++) {
        Serial.print(String(i) + "...");
        delay(1000);
    }*/

/*
    readFile(SPIFFS, "/update.bin");

    updateFromFS();
*/

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

/**
 * 多线程OTA任务
 */
static String otaVersion;
static String otaJsonUrl;

void x_gsm_task_ota(void *pvParameters) {
    while (1) {  // RTOS多任务条件： 1. 不断循环 2. 无return关键字
        do_gsm_firmware_upgrade(otaVersion, otaJsonUrl);
        delay(600000); // 多久执行一次 毫秒
    }
}

/**
 * 执行OTA空中升级
 */
void gsm_exec_ota(String version, String jsonUrl) {
    Serial.println("开始检测GSM网络OTA空中升级...");
    otaVersion = version;
    otaJsonUrl = jsonUrl;
#if !USE_MULTI_CORE
    const char *params = NULL;
    xTaskCreate(
            x_gsm_task_ota,  /* Task function. */
            "x_gsm_task_ota", /* String with name of task. */
            1024 * 8,      /* Stack size in bytes. */
            (void *) params,      /* Parameter passed as input of the task */
            10,         /* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) */
            NULL);     /* Task handle. */
#else
    //最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1,或者 tskNO_AFFINITY 允许任务在两者上运行.
    xTaskCreatePinnedToCore(x_gsm_task_ota, "x_gsm_task_ota", 8192, NULL, 10, NULL, 0);
#endif
}

/**
 * 差分固件升级
 * https://github.com/espressif/esp-bootloader-plus/blob/master/README_CN.md
 */
void do_gsm_diff_firmware_upgrade() {

}