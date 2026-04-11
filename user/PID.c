//
// Created by XiaoT on 2026/2/23.
//
#include "stm32f4xx_hal.h"
#include "PID.h"
#include <math.h>
#define EPSILON 0.001

void PID_Init(PID_t *p) {
    p->Target=0;
    p->Actual=0;
    p->Out=0;
    p->Error0=0;
    p->Error1=0;
    p->ErrorInt=0;
}
void LocationPID_Update(PID_t *p) {
    p->Error1 = p->Error0;
    p->Error0 = p->Target - p->Actual;

    //误差积分
    if (fabs(p->Ki) > EPSILON) //这一行是为了防止ki为0的时候积分深度饱和，在进行积分限幅之后可以不用了
    {
        p->ErrorInt += p->Error0;
    } else { p->ErrorInt = 0; }
    if (p->ErrorInt>p->IntMax){p->ErrorInt = p->IntMax;}
    if (p->ErrorInt<p->IntMin){p->ErrorInt = p->IntMin;}

    //PID计算
    p->Out = p->Kp * p->Error0 + p->Ki * p->ErrorInt + p->Kd * (p->Error0 - p->Error1);
    //输出限幅
    if (p->Out > p->OutMax) p->Out =p->OutMax;
    if (p->Out < p->OutMin) p->Out =p->OutMin;
}

void IncreasePID_Update(PID_t *p) {
    p->Error2 = p->Error1;//上上次
    p->Error1 = p->Error0;//上次
    p->Error0 = p->Target - p->Actual;//本次


    //PID计算
    p->Out += p->Kp * (p->Error0 - p->Error1) + p->Ki * p->Error0 + p->Kd * (p->Error0 - 2 * p->Error1 + p->Error2);

    //输出限幅
    if (p->Out > p->OutMax) p->Out =p->OutMax;
    if (p->Out < p->OutMin) p->Out =p->OutMin;
}

