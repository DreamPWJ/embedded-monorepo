#ifndef EMBEDDED_MONOREPO_BLUETOOTH_CONNECT_H
#define EMBEDDED_MONOREPO_BLUETOOTH_CONNECT_H

#include <WString.h>

/**
* @author 潘维吉
* @date 2022/7/20 9:41
* @description 低功耗蓝牙BLE模块
  */

void init_bluetooth(String bleName);

void bluetooth_send(String data);

void check_bluetooth_state();

void bluetooth_heart_beat();

#endif
