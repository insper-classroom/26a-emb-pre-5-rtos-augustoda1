/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>


#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

QueueHandle_t xQueueButId;
QueueHandle_t xQueueButId2;
QueueHandle_t xQueueBtnEvent;

void btn_callback(uint pin, uint32_t events) {
    if (events == 0x4) { // fall edge
        xQueueSendFromISR(xQueueBtnEvent, &pin, 0);
    }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int estado = 0;
    int delay;
    while (true) {
        if (xQueueReceive(xQueueButId, &delay, 0)) {
            estado = !estado;
        }
        if (estado) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }

       else{
            vTaskDelay(pdMS_TO_TICKS(10));
       }
    }
}

void btn_task(void* p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    int pin;
    int dummy = 0;
    while (true) {
        if (xQueueReceive(xQueueBtnEvent, &pin, portMAX_DELAY) == pdTRUE) {
            if (pin == BTN_PIN_R)
                xQueueSend(xQueueButId, &dummy, 0);
            else if (pin == BTN_PIN_Y)
                xQueueSend(xQueueButId2, &dummy, 0);
        }
    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    int estado = 0;
    int delay;
    while (true) {
        if (xQueueReceive(xQueueButId2, &delay, 0)) {
            estado = !estado;
        }
        if (estado) {
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }

       else{
            vTaskDelay(pdMS_TO_TICKS(10));
       }
    }
}

int main() {
    stdio_init_all();

    xQueueButId = xQueueCreate(32, sizeof(int));
    xQueueButId2 = xQueueCreate(32, sizeof(int));
    xQueueBtnEvent = xQueueCreate(32, sizeof(int));

    xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED_Task 2", 256, NULL, 1, NULL);
    xTaskCreate(btn_task,   "BTN_Task",   256, NULL, 1, NULL);

    vTaskStartScheduler();

    while(true){}

}