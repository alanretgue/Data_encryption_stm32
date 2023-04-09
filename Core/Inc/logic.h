#pragma once

#include <stdint.h>
#include "main.h"
#include "mbedtls/aes.h"
#include "mbedtls/cipher.h"
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

#define FREQ 4
#define TIME 15
#define PADDING_SIZE 16
#define BUFFER_SIZE 256
#define FULL_BUFFER_SIZE BUFFER_SIZE + PADDING_SIZE

typedef struct {
    char flags;
    uint8_t length;
} Header;

typedef struct {
    uint8_t generated;
    unsigned char value[32];
} Key;

extern Key key __attribute__((section(".key")));

extern uint64_t millis;

extern ADC_HandleTypeDef hadc1;

extern Header header;

void SYSTICK_Handler(void);

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
int write_flash(Key *new_value);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
int generate_key();
size_t encrypt(unsigned char *buff, uint32_t size, unsigned char *res);
size_t decrypt(unsigned char *buff, uint32_t size, unsigned char *res);
