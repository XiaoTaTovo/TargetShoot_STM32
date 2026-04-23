#include "task.h"
#include "math.h"
#include "serial.h"
#include "function.h"
#include "servo.h"
#include "PID.h"
#include "ServoPID.h"
#include "usart.h"
#include "OLED.h"
#include "alarm.h" // 引入你的双核报警器

// 全局模式变量：1=单目标, 2=多目标停留, 3=描边
uint8_t System_Mode = 1;
//系统的启停离合器 (0=归中待机锁死, 1=允许自动追踪)
uint8_t System_Run = 0;

// 任务2专用的状态机变量
static uint32_t Lock_Start_Time = 0;
static uint8_t Is_Locked = 0;

extern Alarm_Controller_t MyAlarm; // 引入主函数里的报警器实体

void Task_Init(void) {
    System_Mode = 1;
    Is_Locked = 0;
}

void Task_Scheduler(void) {
    // 🌟 守护线程：一刻不停地跑声光报警器
    Alarm_Loop(&MyAlarm);
    VOFA_Command_Proceed();
    // ========================================================
    // 任务 A：20ms 频率 —— 核心追踪与控制逻辑 (完美契合舵机 50Hz)
    // ========================================================
    if (Task_Counter(0, 20)) {
        if (System_Run == 1) {
            // 追踪模式已经启动了，才执行追踪逻辑

        if (Target_State == 1) {
            // 视觉发来了有效目标
            if (fabs(Vision_ErrX) < 5.0) {
                Vision_ErrX = 0;
            }
            if (fabs(Vision_ErrY) < 5.0) {
                Vision_ErrY = 0;
            }
            // 1. 无论什么模式，先把 PID 跑起来，让它去追！
            ServoPID_Execute();

            // 2. 只有在模式 2 (停留打靶) 时，才需要介入计时逻辑
            if (System_Mode == 2) {
                // 假设误差在 X和Y 都小于 15 像素以内，算作“咬住靶心”
                if (fabs(Yaw_Error) < 15.0f && fabs(Pitch_Error) < 15.0f) {
                    if (Is_Locked == 0) {
                        // 刚刚咬住的瞬间，记下时间戳！
                        Lock_Start_Time = HAL_GetTick();
                        Is_Locked = 1;
                    } else {
                        // 已经咬住了，看看够不够 2 秒？(2000ms)
                        if (HAL_GetTick() - Lock_Start_Time >= 2000) {
                            // 🎯 完美停留 2 秒！
                            Alarm_Start_Beep(&MyAlarm, 1); // 滴 1 声
                            Alarm_Start_Blink(&MyAlarm, 2); // 闪 2 下

                            // 通知 Python：“我已经打完这个了，赶紧给我下一个目标的坐标！”
                            Serial_Printf(&huart1, "NEXT_TARGET\n");

                            // 重置状态，准备追下一个
                            Is_Locked = 0;
                            Lock_Start_Time = 0;
                        }
                    }
                }
            }else {
                    // 如果风吹草动导致激光点偏出去了，必须重新计时！
                    Is_Locked = 0;
                    Lock_Start_Time = 0;
                }
            }
        } else {
            // 视觉丢失目标了，重置锁定状态
            Is_Locked = 0;
        }

        // 🌟 重点监控：通道0发 Error(病情)，通道1发 Yaw_PID.Out(药量)！
        // VOFA_JustFloat_Send((float) Vision_ErrX, PID_Yaw.Out);
        // VOFA_JustFloat_Send((float) Vision_ErrY, PID_Pitch.Out);
        VOFA_JustFloat_Send((float) PID_Yaw.Out, PID_Pitch.Out);
    }


    // ========================================================
    // 任务 B：100ms 频率 —— 刷新 OLED 屏幕
    // ========================================================
    /*if (Task_Counter(1, 100)) {
        OLED_Clear();
        OLED_Printf(0, 48, OLED_8X16, "Mode:%d", System_Mode); // 显示当前模式

        if (Target_State == 1) {
            OLED_Printf(0, 0, OLED_8X16, "X:%-5d", Vision_ErrX);
            OLED_Printf(0, 16, OLED_8X16, "Y:%-5d", Vision_ErrY);

            if (System_Mode == 2 && Is_Locked == 1) {
                OLED_ShowString(0, 32, "TIMING...", OLED_8X16); // 正在倒计时
            } else {
                OLED_ShowString(0, 32, "TRACKING...", OLED_8X16); // 正在追击
            }
        } else {
            OLED_ShowString(0, 0, "Wait Target...", OLED_8X16);
        }
        OLED_Update();
    }*/
}
