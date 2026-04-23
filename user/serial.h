#ifndef __SERIAL_H
#define __SERIAL_H

#include "main.h"

void Serial_Printf(UART_HandleTypeDef *huart, char *format, ...);
void Vision_Data_Proceed(void);
void VOFA_JustFloat_Send(float ch1, float ch2); // 声明 VOFA 发送函数
void VOFA_Command_Proceed(void); // 新增：解析 VOFA 发来的调参指令

// 视觉专属 (USART1)
extern uint8_t Vision_RxFlag;
extern uint8_t Vision_RxBuffer[50];
extern int Target_State;
extern int Vision_ErrX;
extern int Vision_ErrY;

// 🌟 新增：蓝牙专属 (USART2)
extern uint8_t BT_RxFlag;
extern uint8_t BT_RxBuffer[50];

#endif