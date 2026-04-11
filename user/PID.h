//
// Created by XiaoT on 2026/2/23.
//

#ifndef __PID_H
#define __PID_H

typedef struct {
    float Target;
    float Actual;
    float Out;
    float Kp;
    float Ki;
    float Kd ;
    float Error0;
    float Error1;
    float Error2;
    float ErrorInt;
    float IntMax;
    float IntMin;
    float OutMax;
    float OutMin;
}PID_t;

void LocationPID_Update(PID_t *p);
void IncreasePID_Update(PID_t *p);
void PID_Init(PID_t *p) ;

#endif //__PID_H