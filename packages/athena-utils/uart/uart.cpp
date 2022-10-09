#include "uart.h"
#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/uart.h>
#include <driver/gpio.h>
#include <cstring>
#include "freertos/queue.h"
#include "esp_log.h"

/**
* @author 潘维吉
* @date 2022/9/5 10:54
* @description UART通用异步收发送器 实现不同设备之间的全双工或半双工数据交换
*/

#define UART_ESP_TXD (GPIO_NUM_18)
#define UART_ESP_RXD (GPIO_NUM_19)
#define UART_ESP_RTS (UART_PIN_NO_CHANGE)
#define UART_ESP_CTS (UART_PIN_NO_CHANGE)

#define BUF_SIZE (1024*2)
#define UART_MAX_NUM_RX_BYTES (1024*2)

/**
 * LOCAL VARIABLES
 */
static QueueHandle_t s_uart0Queue;

static const char *TAG = "board_uart";

const uart_port_t uart_num = UART_NUM_1;

/**
 * ESP32芯片有三个UART控制器(UART 0、UART 1和UART 2)，它们具有一组相同的寄存器，以便于编程和灵活性
 * 每个UART控制器都是独立配置的，参数包括波特率、数据比特长度、位序、停止位数、奇偶校验位等
 */
static void set_uart(void *pvParameters) {

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    while (1) {
        // 读取UART的数据
        int len = uart_read_bytes(uart_num, data, BUF_SIZE, 20 / portTICK_RATE_MS);
        // 写入UART数据
        uart_write_bytes(uart_num, (const char *) data, len);
    }
}

/**
 * 串口队列接收
 */
static void set_uart_rx(void *pvParameters) {

    uart_event_t event;
    uint8_t *pTempBuf = (uint8_t *) malloc(UART_MAX_NUM_RX_BYTES);

    for (;;) {
        // Waiting for UART event.
        if (xQueueReceive(s_uart0Queue, (void *) &event, (portTickType) portMAX_DELAY)) {
            bzero(pTempBuf, UART_MAX_NUM_RX_BYTES);

            switch (event.type) {
                // Event of UART receving data
                // We'd better handler data event fast, there would be much more data events than
                // other types of events. If we take too much time on data event, the queue might be full.
                case UART_DATA:
                    // ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                    uart_read_bytes(uart_num, pTempBuf, event.size, portMAX_DELAY);
                    uart_write_bytes(uart_num, (const char *) pTempBuf, event.size);
                    break;

                    // Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(uart_num);
                    xQueueReset(s_uart0Queue);
                    break;

                    // Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(uart_num);
                    xQueueReset(s_uart0Queue);
                    break;

                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error");
                    break;

                    // Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error");
                    break;

                    // Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }

    free(pTempBuf);
    pTempBuf = NULL;
    vTaskDelete(NULL);

}

void init_uart(void) {
    Serial.println("UART通用异步收发送器初始化");
    uart_config_t uart_config = {
            .baud_rate = 9600,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    uart_set_pin(uart_num, UART_ESP_TXD, UART_ESP_RXD, UART_ESP_RTS, UART_ESP_CTS);
    // 必须要先uart_driver_install安装驱动
    uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0);
/*    // 在把串口中断服务给释放掉
    uart_isr_free(uart_num);
    // 重新注册串口中断服务函数
    uart_isr_handle_t handle;
    // 注册中断处理函数
    uart_isr_register(uart_num, uart2_irq_handler, NULL, ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM, &handle);
    //使能串口接收中断
    uart_enable_rx_intr(uart_num); */

/*    // 串口回环输出
    xTaskCreate(
            set_uart,  *//* Task function. *//*
            "set_uart", *//* String with name of task. *//*
            1024 * 2,      *//* Stack size in bytes. *//*
            NULL,      *//* Parameter passed as input of the task *//*
            10,         *//* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) *//*
            NULL);     *//* Task handle. *//*

    // 串口队列接收
    xTaskCreate(
            set_uart_rx,  *//* Task function. *//*
            "set_uart_rx", *//* String with name of task. *//*
            1024 * 2,      *//* Stack size in bytes. *//*
            NULL,      *//* Parameter passed as input of the task *//*
            12,         *//* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) *//*
            NULL);     *//* Task handle. */
}
