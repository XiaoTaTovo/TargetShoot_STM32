#include "serial.h"
#include "ServoPID.h"
#include "PID.h"
#include <stdio.h>
#include <string.h>
#include <usart.h>
#include <stdarg.h>
#include <stdlib.h>


#include "alarm.h"
extern Alarm_Controller_t MyAlarm;
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
        char *start_ptr = strchr((char *) Vision_RxBuffer, '<');

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
    // 使用静态数组，防止函数退出后局部变量被销毁导致乱码
    static uint8_t tx_buf[12];
    uint8_t tail[4] = {0x00, 0x00, 0x80, 0x7F}; // VOFA+ 结尾暗号

    // 把 float 的 4个字节直接强行搬运进数组
    memcpy(&tx_buf[0], &ch1, 4);
    memcpy(&tx_buf[4], &ch2, 4);
    memcpy(&tx_buf[8], tail, 4);

    // 🚀 重点：这里换成 huart2 了！发送给蓝牙！
    HAL_UART_Transmit_DMA(&huart2, tx_buf, 12);

}


extern uint8_t BT_RxFlag;
extern uint8_t BT_RxBuffer[50];



// 🌟 新增：解析 VOFA 发来的调参字符串
void VOFA_Command_Proceed(void) {
    if (BT_RxFlag == 1) {
        BT_RxFlag = 0; // 清除标志位

        float temp_val = 0.0f;

        // 1. 拦截 Kp 指令 (格式: "P:1.23\n")
        if (sscanf((char *) BT_RxBuffer, "P:%f", &temp_val) == 1) {
            // 🌟 名字统一改成 PID_Yaw 和 PID_Pitch ！
            PID_Yaw.Kp = temp_val;
            // PID_Pitch.Kp = temp_val;
            Alarm_Start_Beep(&MyAlarm, 1);
        }
        // 2. 拦截 Ki 指令 (格式: "I:0.12\n")
        else if (sscanf((char *) BT_RxBuffer, "I:%f", &temp_val) == 1) {
            PID_Yaw.Ki = temp_val;
            // PID_Pitch.Ki = temp_val;
            Alarm_Start_Beep(&MyAlarm, 2);
        }
        // 3. 拦截 Kd 指令 (格式: "D:0.50\n")
        else if (sscanf((char *) BT_RxBuffer, "D:%f", &temp_val) == 1) {
            PID_Yaw.Kd = temp_val;
            // PID_Pitch.Kd = temp_val;
            Alarm_Start_Beep(&MyAlarm, 3);
        }

        // 清空缓冲区，防止旧数据干扰下一帧
        memset(BT_RxBuffer, 0, sizeof(BT_RxBuffer));

        // 重新开启 USART2 的 DMA 接收
        HAL_UART_Receive_DMA(&huart2, (uint8_t *) BT_RxBuffer, 50);
    }
}
