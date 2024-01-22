#ifndef EMBEDDED_MONOREPO_CHIP_INFO_H
#define EMBEDDED_MONOREPO_CHIP_INFO_H
#include <Arduino.h>
/**
* @author 潘维吉
* @date 2022/7/20 9:41
* @description 芯片信息模块
*/

uint32_t get_chip_id();

uint64_t get_chip_mac();

void get_chip_info(void);

#endif
