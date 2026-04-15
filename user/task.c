#include "task.h"

#include "alarm.h"
#include "math.h"
#include "serial.h"
#include "function.h" // 里面有你的 Task_Counter 神器
#include "servo.h"
#include "PID.h"
#include "ServoPID.h" // 假设这里有你的 ServoPID_Execute
#include "usart.h"
#include "OLED.h"
extern Alarm_Controller_t MyAlarm;

// 1代表任务1(单目标)，2代表任务2(多目标)
uint8_t System_Mode = 1;

// ================= 任务2专用的状态机变量 =================
static uint8_t Mode2_Step = 0; // 记录任务2进行到了哪一步
static uint32_t Lock_Start_Time = 0; // 记录激光死死咬住靶心的那一瞬间的时间戳

void Task_Init(void) {
    System_Mode = 1;
    Mode2_Step = 0;
}

// 👑 这个函数直接放到 main.c 的 while(1) 里面疯狂调用！
/*void Task_Scheduler(void) {
 Alarm_Loop(&MyAlarm);
    // 我们用 20ms 的频率来跑整个云台的逻辑 (完美契合舵机 50Hz)
    if (Task_Counter(1, 20)) {
        switch (System_Mode) {
            // ===============================================
            // 🎯 任务 1：单目标追踪 (看见就咬死不放)
            // ===============================================
            case 1:
                // 假设 Target_State 是视觉发来的状态 (1=有目标，0=丢失)
                if (Target_State == 1) {
                    ServoPID_Execute();
                } else {
                    Servo_SetYaw(0); // 丢失目标，云台回正待命
                    Servo_SetPitch(0);
                }
                break;

                // ===============================================
                // 🎯 任务 2：多目标按边数升序打靶 (要求停留2秒)
                // ===============================================
            case 2:
                if (Target_State == 1) {
                    switch (Mode2_Step) {
                        case 0: // 【步骤0：追击阶段】
                            // 1. 先执行 PID 追击
                            ServoPID_Execute();

                            // 2. 检查是不是追到了？(假设 Error_X 和 Error_Y 都在 ±5 像素以内)
                            if (fabs(Yaw_Error) < 5.0f && fabs(Pitch_Error) < 5.0f) {
                                // 目标已锁定！记录现在的时间戳，准备进入停留阶段！
                                Lock_Start_Time = HAL_GetTick();
                                Mode2_Step = 1; // 切换到步骤1
                            }
                            break;

                        case 1: // 【步骤1：锁定停留阶段 (等待2秒)】
                            // 在这 2 秒内，依然要微调 PID，防止风吹草动导致靶标歪了
                            ServoPID_Execute();

                            // 检查时间到了没有？
                            if (HAL_GetTick() - Lock_Start_Time >= 2000) {
                                // 2 秒停留完成！
                                // 告诉树莓派：“赶紧把当前目标拉黑，把下一个边数最少的靶子坐标发给我！”
                                Serial_Printf(&huart1, "NEXT_TARGET\n");

                                Mode2_Step = 0; // 回到步骤0，去追击下一个新目标！
                            }
                            break;
                    }
                }
                break;
        }
    }
}*/

void Task_Scheduler(void) {
 Alarm_Loop(&MyAlarm);

    // 任务 A：20ms 频率 —— 给 VOFA+ 喂数据 (高速、实时)
    if (Task_Counter(0, 20)) {
        if (Target_State == 1) {
            // 把解包出来的原始误差直接发给 VOFA+
            VOFA_JustFloat_Send((float)Vision_ErrX, (float)Vision_ErrY);
        }
    }

    // 任务 B：100ms 频率 —— 刷新 OLED 屏幕 (低速、肉眼观察)
    if (Task_Counter(1, 100)) {
        OLED_Clear();
        if (Target_State == 1) {
            OLED_Printf(0, 0, OLED_8X16, "X:%+4d", Vision_ErrX);
            OLED_Printf(0, 16, OLED_8X16, "Y:%+4d", Vision_ErrY);
            OLED_ShowString(0, 32, "Target: LOCKED", OLED_8X16);
        } else {
            // OLED_ShowString(0, 0, "Wait Target...", OLED_8X16);
            OLED_Printf(0, 0, OLED_8X16, "Wait Target...");

        }
        OLED_Update(); // 把显存刷入屏幕
    }
}