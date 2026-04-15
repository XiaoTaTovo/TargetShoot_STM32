#include "alarm.h"

void Alarm_Init(Alarm_Controller_t *alarm) {
    alarm->Beep_Active = 0;
    alarm->Blink_Active = 0;
    BEEP_OFF();
    LED_OFF();
}

void Alarm_Start_Beep(Alarm_Controller_t *alarm, uint8_t beep_times) {
    alarm->Beep_Active = 1;                   // 激活蜂鸣器大脑
    alarm->Beep_Target = beep_times * 2;      // 响1次 = 翻转2次
    alarm->Beep_Count = 0;
    alarm->Beep_LastTime = HAL_GetTick();
    BEEP_ON();                                // 第一下直接出声
}

void Alarm_Start_Blink(Alarm_Controller_t *alarm, uint8_t blink_times) {
    alarm->Blink_Active = 1;                  // 激活LED大脑 (绝对不会覆盖蜂鸣器!)
    alarm->Blink_Target = blink_times * 2;
    alarm->Blink_Count = 0;
    alarm->Blink_LastTime = HAL_GetTick();
    LED_ON();                                 // 第一下直接亮灯
}

// 👑 双核守护线程
void Alarm_Loop(Alarm_Controller_t *alarm) {
    uint32_t current_time = HAL_GetTick();

    // ==========================================
    // 独立处理：蜂鸣器 (每 100ms 翻转一次)
    // ==========================================
    if (alarm->Beep_Active == 1) {
        if ((current_time - alarm->Beep_LastTime) >= 100) {
            alarm->Beep_LastTime = current_time;
            alarm->Beep_Count++;

            // 奇数次关，偶数次开
            if (alarm->Beep_Count % 2 != 0) BEEP_OFF();
            else BEEP_ON();

            // 如果次数达标，强制关闭并休息
            if (alarm->Beep_Count >= alarm->Beep_Target) {
                BEEP_OFF();
                alarm->Beep_Active = 0;
            }
        }
    }

    // ==========================================
    // 独立处理：LED (每 200ms 翻转一次)
    // ==========================================
    if (alarm->Blink_Active == 1) {
        if ((current_time - alarm->Blink_LastTime) >= 200) {
            alarm->Blink_LastTime = current_time;
            alarm->Blink_Count++;

            if (alarm->Blink_Count % 2 != 0) LED_OFF();
            else LED_ON();

            if (alarm->Blink_Count >= alarm->Blink_Target) {
                LED_OFF();
                alarm->Blink_Active = 0;
            }
        }
    }
}