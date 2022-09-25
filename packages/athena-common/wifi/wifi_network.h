#ifndef ESP32_DEMO_WIFI_NETWORK_H
#define ESP32_DEMO_WIFI_NETWORK_H

/**
* @author 潘维吉
* @date 2022/7/20 10:36
* @description WiFI无线网络模块
  */

void init_wifi();

void init_wifi_multi_thread(void *pvParameters);

bool scan_wifi();

void reconnect_wifi(void);


#endif
