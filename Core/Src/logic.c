#include "logic.h"

#define FREQ 4
#define TIME 5

const int ENABLE_END = FREQ * TIME;
int enable = 1;
uint32_t timer_count = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
    timer_count = 0;
    enable = 0;

    uint32_t tmp = key;
    printf(" tmp = %lu\r\n", tmp++);
    printf(" tmp + 1 = %lu\r\n", tmp);
    write_flash(tmp);

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
    if (enable) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
    } else {
        timer_count++;
        if (timer_count == ENABLE_END){
            enable = 1;
        }
    }
}

void write_flash(uint32_t new_value) {
    uint32_t Address = (uint32_t)&key;
    printf("new value = %lu\r\n", new_value);
    printf("Address = %lu\r\n", Address);

    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase  = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.Sector = FLASH_SECTOR_1;
    EraseInitStruct.NbSectors = 1;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    uint32_t PAGEError = 0;
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
        printf("ERROR\r\n");

    // HAL_Delay(50);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, new_value);
    // HAL_Delay(50);
    HAL_FLASH_Lock();
}
