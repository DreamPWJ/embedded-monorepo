#ifndef EMBEDDED_MONOREPO_RADIO_FREQUENCY_H
#define EMBEDDED_MONOREPO_RADIO_FREQUENCY_H

/**
* @author 潘维吉
* @date 2022/9/19 10:53
* @description 无线射频RF通信
*/



void rf_init(void);

void rf_accept_data(void *pvParameters);

void rf_send_data(void);


#endif
