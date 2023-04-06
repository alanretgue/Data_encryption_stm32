#include "logic.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_adc.h"
#include "stm32f4xx_hal_cortex.h"
#include "stm32f4xx_hal_uart.h"
#include <stdint.h>
#include <stdio.h>

#define FREQ 4
#define TIME 15
#define BUFFER_SIZE 256

const int ENABLE_END = FREQ * TIME;
int enable = 0;
uint32_t timer_count = 0;

Header header = {
    .flags = 0,
    .length = 0,
};

uint8_t test_buff[2] = { 0 };

uint64_t millis = 0;

uint8_t idx = 0;
uint8_t state = 1;
uint8_t size = 1;
uint8_t recv = 1;
unsigned char recieved_data[2 * BUFFER_SIZE] = { 'a' };

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
    if (!enable) {
        timer_count = 0;
        enable = 1;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);

        state = 1;
        HAL_UART_Receive_IT(&huart2, &header, 2);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
    if (!enable) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
    } else {
        timer_count++;
        if (timer_count == ENABLE_END){
            enable = 0;
        }
    }
}

int write_flash(Key *new_value) {
    uint32_t Address = (uint32_t)&key;

    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase  = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.Sector = FLASH_SECTOR_1;
    EraseInitStruct.NbSectors = 1;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    uint32_t PAGEError = 0;
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
        printf("ERROR\r\n");
        return ERROR_WRITE_KEY;
    }

    HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, Address, new_value->generated);
    for (int i = 0; i < 32; i++) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, Address + 1 + i, new_value->value[i]);
    }
    HAL_FLASH_Lock();
    return SUCCESS;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

    // HAL_UART_Receive_DMA(&huart1, &recieved_data[16 * idx], 16);
    if (enable) {
        if (state && (header.flags & INIT) == 0) {
            state = !state;
            idx = !idx;
            size = header.length;
            HAL_UART_Receive_DMA(&huart2, recieved_data /* + idx *  BUFFER_SIZE*/, size);
        } else if ((header.flags & INIT) == 1) {
            unsigned char res = '0' + (unsigned char)generate_key();
            HAL_UART_Transmit_DMA(&huart2, (uint8_t *)key.value, 32);
            enable = 0;
        } else {
            HAL_UART_Transmit_DMA(&huart2, recieved_data, size);
            state = 1;
            if ((header.flags & END) == 0) {
                // recv = 1;
                HAL_UART_Receive_IT(&huart2, &header, 2);
            }
            else {
                enable = 0;
            }
        }
    }
}

int custom_poll( void *data,
                    unsigned char *output, size_t len, size_t *olen )
{
    ((void) data);
    ((void) output);
    *olen = sizeof(len);

    int32_t val1 = HAL_ADC_GetValue(&hadc1) >> 2;
    int32_t val2 = HAL_GetTick();
    int32_t val3 = millis;
    *output = (val1 ^ val2) | (val3 << 3);
    return 0;
}

int generate_key() {
    if (key.generated) {
        return ERROR_ALREADY_DEFINED;
    }

    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
    Key tmp_key = {
        .generated = 1,
        .value = { 0 },
    };

    char *pers = "Les loutres controleront le monde et les pates c'est bon";
    int ret;

    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    mbedtls_entropy_add_source(&entropy, custom_poll, NULL, 4, MBEDTLS_ENTROPY_SOURCE_STRONG);

    if((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                    (unsigned char *) pers, strlen(pers))) != 0) {
        return ERROR_SEED;
    }

    if((ret = mbedtls_ctr_drbg_random(&ctr_drbg, tmp_key.value, 32)) != 0) {
        return ERROR_RANDOM;
    }

    return write_flash(&tmp_key);
}

void SYSTICK_Handler(void) { millis++; }
