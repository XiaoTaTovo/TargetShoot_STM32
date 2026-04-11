#include "stm32f4xx_hal.h"
#include "key.h"
static uint8_t Key_Num = 0;
//PA4--1
//PA5--2
//PA6--3

uint8_t Key_GetNum(void) {
    uint8_t Temp;
    if (Key_Num) {
        Temp = Key_Num;
        Key_Num = 0;
        return Temp;
    }
    return 0;
}

uint8_t Key_GetState(void) //获取当前按键状态的子函数，非阻塞式
{
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)
        return 1;
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET)
        return 2;
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET)  // Changed from PB8 to PA6
        return 3;
    return 0;
}

void Key_Tick(void)//在1ms一次的中断中调用，获取20ms前后按键状态
{
    static uint8_t count;
    static uint8_t current, previous;

    count++;
    if(count >= 20)
    {
        count = 0;

        previous = current;
        current = Key_GetState();
        if(current != 0 && previous == 0)  // 按下触发，而不是抬起触发
            Key_Num = current;

    }

}
