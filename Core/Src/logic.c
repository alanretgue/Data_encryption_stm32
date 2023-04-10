#include "logic.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_adc.h"
#include "stm32f4xx_hal_cortex.h"
#include "stm32f4xx_hal_uart.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

const int ENABLE_END = FREQ * TIME;
int enable = 0;
uint32_t timer_count = 0;

Header header = {
    .flags = 0,
    .length = 0,
};

uint64_t millis = 0;

uint8_t idx = 0;
uint8_t set_key = 0;
uint8_t state = 1;
unsigned char recieved_data[2 * (FULL_BUFFER_SIZE)] = { 'a' };
unsigned char output[FULL_BUFFER_SIZE] = { 0 };
mbedtls_aes_context ctx;


size_t michel(){

    char unsigned buff [256] = {0};
    buff[0] = 'a';
    buff[1] = 'b';
    buff[2] = 'c';
    buff[3] = 'd';
    buff[4] = 'e';
    buff[5] = 'f';
    size_t size = 6;
    char unsigned res[258] = {0};
    mbedtls_aes_setkey_enc(&ctx, key.value, 256);
    unsigned char iv[16] = { 0 };

    if (header.flags & END) {
        res[1] = 16 - (size % 16);
        for(size_t i = 0; i < res[1]; i++) {
            buff[size + i] = (unsigned char) res[1];
        }
    } else {
        res[1] = 0;
    }

    size_t final_size = size + res[1];

    mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, final_size, iv, buff, res + 2);

    mbedtls_aes_free(&ctx);
    mbedtls_aes_init(&ctx);

    mbedtls_aes_setkey_dec(&ctx, key.value, 256);
    unsigned char iv2[16] = { 0 };

    unsigned char output[256] = {0};

    size = final_size;

    mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, size, iv2, res + 2, output);

    if (header.flags & END)
        res[1] = 16 - res[2 + size - 1];
    else
        res[1] = 0;
    return size - res[1];
}



void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
    if (!enable) {
        timer_count = 0;
        enable = 1;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);

        state = 1;
        mbedtls_aes_free(&ctx);
        mbedtls_aes_init(&ctx);
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
            HAL_UART_Receive_DMA(&huart2, recieved_data, header.length + 1);

        } else {
            /**
             * res[0] => error code
             * res[1] => padding
             * res + 2 => message
             */
            unsigned char res[258] = {0};
            res[0] = '0';
            res[1] = 0;
            if ((header.flags & INIT) == 1) {
                unsigned char res = '0' + (unsigned char)generate_key();
                HAL_UART_Transmit_DMA(&huart2, &res, 1);

            } else if ((header.flags & ENCRYPT)) {
                //michel();
                size_t size = encrypt(recieved_data, header.length + 1, res);
                HAL_UART_Transmit_DMA(&huart2, (uint8_t *)res, size + 2);

            } else if ((header.flags & DECRYPT)) {
                size_t size = decrypt(recieved_data, header.length + 1, res);
                HAL_UART_Transmit_DMA(&huart2, (uint8_t *)res, size + 2);
            }

            state = 1;

            if (header.flags & END) {
                mbedtls_aes_free(&ctx);
                set_key = 0;
                enable = 0;
            }
            HAL_UART_Receive_DMA(&huart2, (uint8_t *)&header, 2);
        }
    } else {
        // error message
        HAL_UART_Transmit_DMA(&huart2, (uint8_t *)"1", 1);
        HAL_UART_Receive_DMA(&huart2, (uint8_t *)&header, 2);
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

    /* mbedtls_entropy_add_source(&entropy, custom_poll, NULL, 1, MBEDTLS_ENTROPY_SOURCE_STRONG);

    if((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                    (unsigned char *) pers, strlen(pers))) != 0) {
        return ERROR_SEED;
    }
    */
    if((ret = mbedtls_ctr_drbg_random(&ctr_drbg, tmp_key.value, 32)) != 0) {
        return ERROR_RANDOM;
    }

    return write_flash(&tmp_key);
}

// Used to create "random"
void SYSTICK_Handler(void) { millis++; }

size_t encrypt(unsigned char *buff, uint32_t size, unsigned char *res) {
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_enc(&ctx, key.value, 256);
    unsigned char iv[16] = { -1 };

    if (header.flags & END) {
        res[1] = 16 - (size % 16);
        for(size_t i = 0; i < res[1]; i++) {
            buff[size + i] = (unsigned char) res[1];
        }
    } else {
        res[1] = 0;
    }

    size_t final_size = size + res[1];

    mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, final_size, iv, buff, res + 2);
    mbedtls_aes_free(&ctx);
    return final_size;
}

size_t decrypt(unsigned char *buff, uint32_t size, unsigned char *res) {
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_dec(&ctx, key.value, 256);
    unsigned char iv[16] = { -1 };

    mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, size, iv, buff, res + 2);
    if (header.flags & END)
        res[1] = (res + 2)[size - 1];
    else
        res[1] = 0;
    mbedtls_aes_free(&ctx);
    return size - res[1];
}

