#include "logic.h"

const int ENABLE_END = 4*5;
int enable = 1;
uint32_t timer_count = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
    timer_count = 0;
    enable = !enable;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
    if (enable) {
        GPIO_PinState state = HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_13);
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, !state);
    } else {
        timer_count++;
        if (timer_count == ENABLE_END){
            enable = 1;
        }
    }
}
