//
// Created by XiaoT on 2026/4/5.
//
#include "function.h"
#include "stm32f4xx_hal.h"


uint8_t Task_Counter(uint8_t id, uint32_t ms) {
#define MAX_TASK 10
    static uint32_t Last_Ticks[MAX_TASK] = {0};
    uint32_t Current_Tick = HAL_GetTick();
    if (id >= MAX_TASK) return 0;
    if (Current_Tick - Last_Ticks[id] >= ms) {
        Last_Ticks[id] = Current_Tick;
        return 1;
    }
    return 0;
}