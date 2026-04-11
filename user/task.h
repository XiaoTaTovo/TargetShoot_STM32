#ifndef TARGETSHOOT_TASK_H
#define TARGETSHOOT_TASK_H

#include "main.h"

// 暴露给外部的全局变量，方便按键中断修改
extern uint8_t System_Mode;

// 函数声明
void Task_Init(void);
void Task_Scheduler(void);

#endif //TARGETSHOOT_TASK_H