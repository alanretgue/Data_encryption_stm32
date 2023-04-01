#pragma once

#include "main.h"
#include "stm32f4xx_hal.h"
#include <unistd.h>
#include <stdio.h>

typedef struct {
    char ied; // INIT / ENCRYPT / DECRYPT
    uint8_t length;
    char end;
    char payload[256];
} MESSAGE;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void write_flash(uint32_t new_value);
