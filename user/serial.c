#include "serial.h"
#include "ServoPID.h"
#include <stdio.h>
#include <string.h>
// 给 PID 准备的输入变量
int Target_State = 0;
int Vision_ErrX = 0;
int Vision_ErrY = 0;


// 真正的非阻塞 DMA 打印函数
void Serial_Printf(UART_HandleTypeDef *huart, char *format, ...) {
    // 必须用 static 或者全局变量，因为 DMA 是异步发送的，函数退出了数据还没发完
    // 如果用局部变量，内存会被覆盖，发出的全是乱码
    static char String[150];

    // 如果上一次的数据 DMA 还没发完，就直接丢弃这次的数据，绝不卡死等待！
    if (huart->gState == HAL_UART_STATE_READY) {
        va_list arg;
        va_start(arg, format);
        vsnprintf(String, sizeof(String), format, arg);
        va_end(arg);

        // 召唤 DMA 兄弟帮忙发送，单片机立刻返回！
        HAL_UART_Transmit_DMA(huart, (uint8_t *) String, strlen(String));
    }
}


void Vision_Data_Proceed(void) {
    if (Vision_RxFlag == 1) {
        Vision_RxFlag = 0; // 放下旗帜

        // 假设树莓派发来的是: <X:-120,Y:+045,S:1>
        // sscanf 会自动去字符串里寻找匹配的格式，抠出 3 个整数。
        // 如果成功抠出 3 个数字，它会返回 3。
        int match_count = sscanf(Vision_RxBuffer, "<X:%d,Y:%d,S:%d>", &Vision_ErrX, &Vision_ErrY, &Target_State);

        if (match_count == 3) {
            // 提取成功！把值赋给 PID 的 Actual
            // 注意：视觉里通常用负数表示偏左，正数表示偏右，你需要根据实际舵机方向决定要不要加负号
            Yaw_Error = (float) Vision_ErrX;
            Pitch_Error = (float) Vision_ErrY;
        }
    }
}
