#include "uart.h"
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/uart.h>
#include <driver/gpio.h>
#include <nb_iot.h>

/**
* @author 潘维吉
* @date 2022/9/5 10:54
* @description UART通用异步收发送器 实现不同设备之间的全双工或半双工数据交换
*/

#define UART_ESP_TXD (GPIO_NUM_7)
#define UART_ESP_RXD (GPIO_NUM_19)
#define UART_ESP_RTS (UART_PIN_NO_CHANGE)
#define UART_ESP_CTS (UART_PIN_NO_CHANGE)

#define BUF_SIZE (1024)


/**
 * ESP32芯片有三个UART控制器(UART 0、UART 1和UART 2)，它们具有一组相同的寄存器，以便于编程和灵活性
 * 每个UART控制器都是独立配置的，参数包括波特率、数据比特长度、位序、停止位数、奇偶校验位等
 */
static void set_uart(void *pvParameters) {
    const uart_port_t uart_num = UART_NUM_1;
    uart_config_t uart_config = {
            .baud_rate = 9600,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_CTS,
            .source_clk = UART_SCLK_APB,
    };
    uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0);
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    uart_set_pin(uart_num, UART_ESP_TXD, UART_ESP_RXD, UART_ESP_RTS, UART_ESP_CTS);

    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    while (1) {
        // 读取UART的数据
        int len = uart_read_bytes(uart_num, data, BUF_SIZE, 20 / portTICK_RATE_MS);
        // 写入UART数据
        uart_write_bytes(uart_num, (const char *) data, len);
       // delay(1000);
    }

}

void init_uart(void) {
    Serial.println("UART通用异步收发送器初始化");
    xTaskCreate(
            set_uart,  /* Task function. */
            "set_uart", /* String with name of task. */
            1024,      /* Stack size in bytes. */
            NULL,      /* Parameter passed as input of the task */
            10,         /* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) */
            NULL);     /* Task handle. */

    // 初始化NB-IoT网络协议
    //init_nb_iot();
}
