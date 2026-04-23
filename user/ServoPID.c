//
// Created by XiaoT on 2026/4/6.
//
#include "ServoPID.h"
#include "servo.h"
PID_t PID_Yaw={0,0,0,0,0.004,0,0,0,0,0,0,0,90,-90};
PID_t PID_Pitch={0,0,0,0.03,0.002,0,0,0,0,0,0,0,90,-90};
// 定义两个全局变量，保存舵机当前的“真实角度”
float Yaw_Error = 0.0f;   // 范围 -90 到 +90
float Pitch_Error = 0.0f; // 范围 -90 到 +90

void ServoPID_Execute(void) {


    PID_Yaw.Actual = -Yaw_Error;
    IncreasePID_Update(&PID_Yaw);

    PID_Pitch.Actual = Pitch_Error;
    IncreasePID_Update(&PID_Pitch);

    Servo_SetYaw(PID_Yaw.Out);
    Servo_SetPitch(PID_Pitch.Out);

}
