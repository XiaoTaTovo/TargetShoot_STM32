#include "main.h"


void Serial_Printf(UART_HandleTypeDef *huart, char *format, ...);

void Vision_Data_Proceed(void);


extern uint8_t Vision_RxFlag;
extern uint8_t Vision_RxBuffer[50];
extern int Target_State;
extern int Vision_ErrX;
extern int Vision_ErrY;
