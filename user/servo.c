//
// Created by XiaoT on 2026/4/6.
//
#include "servo.h"
#include "tim.h" // 引入定时器硬件支持 (CubeMX 自动生成的)

/*
舵机要求输入的周期是20ms，对应频率为50hz
设置定时器的自动重装载值为20000_，那么每数一个数，对应1us
一个周期内500-2500us的高电平时间决定了转动的角度，及占空比，
占空比的设置就是输出比较寄存器的值，因为小于输出比较寄存器，输出高电平
*/


// ---------------- 内部独有的函数 ----------------

static uint16_t Angle_To_CCR(float angle) {
    float result = 0.0f;
    // 这里的 angle 必须是 -90度 到 +90度 之间的相对角
    result = 1500.0f + angle * (2000.0f / 180.0f);

    if (result > 2500.0f) {
        result = 2500.0f;
    }
    else if (result < 500.0f) {
        result = 500.0f;
    }
    return (uint16_t)result;
}

// ---------------- 外部可调用的接口 ----------------

// 控制水平偏航角 (Yaw)
void Servo_SetYaw(float angle_offset) {
    uint16_t ccr_val = Angle_To_CCR(angle_offset);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr_val);
}

// 控制上下俯仰角 (Pitch)
void Servo_SetPitch(float angle_offset) {
    uint16_t ccr_val = Angle_To_CCR(angle_offset);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, ccr_val);
}

// 初始化舵机 PWM
void Servo_Init(void) {
    // 开启定时器 PWM 输出
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

    // 初始化时，让云台自动回正 (偏差 0 度，也就是 1500 的位置)
    Servo_SetYaw(0.0f);
    Servo_SetPitch(0.0f);
}

