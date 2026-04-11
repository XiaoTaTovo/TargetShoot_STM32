#ifndef __SERVO_H
#define __SERVO_H

#include "main.h" // 包含万能钥匙

// 给外部调用的函数声明
void Servo_Init(void);
void Servo_SetYaw(float angle_offset);
void Servo_SetPitch(float angle_offset);

#endif //__SERVO_H