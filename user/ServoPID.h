//
// Created by XiaoT on 2026/4/6.
//
#ifndef __SERVOPID_H
#define __SERVOPID_H
#include "PID.h"

void ServoPID_Execute(void) ;
extern PID_t PID_Yaw;
extern PID_t PID_Pitch;
extern float Yaw_Error ;   // 范围 -90 到 +90
extern float Pitch_Error ; // 范围 -90 到 +90
#endif
