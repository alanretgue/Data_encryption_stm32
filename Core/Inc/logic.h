#pragma once

#include "main.h"
#include "stm32f4xx_hal.h"
#include <unistd.h>
#include <stdio.h>

#define INIT 1
#define ENCRYPT 2
#define DECRYPT 4
#define END 8

typedef struct {
    char flags;
    uint8_t length;
    // char ied; // INIT / ENCRYPT / DECRYPT
    // char end;
    char payload[256];
} Message;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void write_flash(uint32_t new_value);
