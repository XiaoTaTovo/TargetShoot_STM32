#ifndef __ALARM_H
#define __ALARM_H

#include "stm32f4xx_hal.h"

// 🌟 终极进化：蜂鸣器和LED拥有独立的“大脑”
typedef struct {
    // 蜂鸣器专区
    uint8_t  Beep_Active;      // 蜂鸣器是否在工作 (1=是，0=否)
    uint8_t  Beep_Target;      // 目标翻转次数
    uint8_t  Beep_Count;       // 已经翻转的次数
    uint32_t Beep_LastTime;    // 蜂鸣器专属时间戳

    // LED专区
    uint8_t  Blink_Active;     // LED是否在工作
    uint8_t  Blink_Target;     // 目标翻转次数
    uint8_t  Blink_Count;      // 已经翻转的次数
    uint32_t Blink_LastTime;   // LED专属时间戳
} Alarm_Controller_t;

// 硬件引脚宏定义 (跟你的一模一样)
#define BEEP_ON()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)
#define BEEP_OFF()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)
#define LED_ON()     HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET)
#define LED_OFF()    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET)

void Alarm_Init(Alarm_Controller_t *alarm);
void Alarm_Start_Beep(Alarm_Controller_t *alarm, uint8_t beep_times);
void Alarm_Start_Blink(Alarm_Controller_t *alarm, uint8_t blink_times);
void Alarm_Loop(Alarm_Controller_t *alarm);

#endif