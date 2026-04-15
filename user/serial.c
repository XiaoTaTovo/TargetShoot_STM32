#include "serial.h"
#include "ServoPID.h"
#include <stdio.h>
#include <string.h>
#include <usart.h>
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

        // 🌟 核心修复：派侦察兵去找包头 '<'
        // 把 uint8_t* 强转成 char*，防止编译器报警告
        char *start_ptr = strchr((char *)Vision_RxBuffer, '<');

        // 如果找到了 '<'，就从它所在的位置开始抠数据
        if (start_ptr != NULL) {
            int match_count = sscanf(start_ptr, "<X:%d,Y:%d,S:%d>", &Vision_ErrX, &Vision_ErrY, &Target_State);

            if (match_count == 3) {
                // 提取成功！
                Yaw_Error = (float) Vision_ErrX;
                Pitch_Error = (float) Vision_ErrY;
            }
        }
    }
}

// 发送 2 个通道的数据到 VOFA+ 看波形 (比如 Error 和 PID_Out)
void VOFA_JustFloat_Send(float ch1, float ch2) {
    // 必须用 static，保证 DMA 发送时数组在内存中不被销毁
    static float send_data[3];

    if (huart2.gState == HAL_UART_STATE_READY) {

        send_data[0] = ch1; // 通道 1：比如 Yaw_Error
        send_data[1] = ch2; // 通道 2：比如 PID_Yaw.Out

        // JustFloat 协议的固定包尾 (0x7F800000，即浮点数的正无穷 NaN)
        ((uint8_t*)&send_data[2])[0] = 0x00;
        ((uint8_t*)&send_data[2])[1] = 0x00;
        ((uint8_t*)&send_data[2])[2] = 0x80;
        ((uint8_t*)&send_data[2])[3] = 0x7F;

        // 召唤 DMA 发送这 3 个浮点数 (总共 3 * 4 = 12 字节)
        HAL_UART_Transmit_DMA(&huart2, (uint8_t*)send_data, sizeof(send_data));
    }
}
