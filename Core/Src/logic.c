#include "logic.h"

#define FREQ 4
#define TIME 5

const int ENABLE_END = FREQ * TIME;
int enable = 1;
uint32_t timer_count = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
    timer_count = 0;
    enable = 0;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
    if (enable) {
        GPIO_PinState state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, !state);
    } else {
        timer_count++;
        if (timer_count == ENABLE_END){
            enable = 1;
        }
    }
}
