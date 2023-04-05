#pragma once

#include <stdint.h>
#include "main.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "stm32f4xx_hal.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>


#define INIT 1
#define ENCRYPT 2
#define DECRYPT 4
#define END 8

#define ERROR_ALREADY_DEFINED 1
#define ERROR_SEED 2
#define ERROR_RANDOM 3
#define ERROR_WRITE_KEY 4
#define SUCCESS 0

typedef struct {
    char flags;
    uint8_t length;
} Header;

typedef struct {
    char generated;
    unsigned char value[32];
} Key;

extern Key key __attribute__((section(".key")));

extern uint64_t millis;

extern ADC_HandleTypeDef hadc1;

void SYSTICK_Handler(void);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
int write_flash(Key *new_value);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
int generate_key();
