/**
 * @file test_idea.c
 * @author linwanhua (linwanhua@injoinic.com)
 * @brief WT588F02B语音模块，用于测试TIM输出PWM + DMA方式，提高CPU效率
 * @version 0.1
 * @date 2024-12-08
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "main.h"
#include "stdint.h"
#include "tim.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"

static volatile uint8_t tx_busy = 0;                // 数据发送状态
void wt588f_dma_callback(DMA_HandleTypeDef *_hdma); // 中断回调函数

/**
 * @brief wt588f及底层外设初始化
 */
void wt588f_init()
{
    HAL_DMA_RegisterCallback(htim2.hdma[TIM_DMA_ID_UPDATE],
                             HAL_DMA_XFER_CPLT_CB_ID,
                             wt588f_dma_callback); // 注册DMA中断回调函数
    __HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_UPDATE); // 定时器Update事件触发DMA请求
    osDelay(200);                               // WT588F02B模块上电延时
}

/**
 * @brief 一线串口控制程序（单字节指令），使用定时器PWM + DMA方式
 * @param data 待发送数据
 */
void wt588f_send_byte(uint8_t data)
{
    // 根据待发送数据，配置cmp数组
    // 数组前7个元素0，用于产生5.6ms低电平
    // 数组最后一个元素，用于等待最后一个upd事件
    static uint32_t cmp_buff[16] = {0};
    cmp_buff[15]                 = 800;
    for (uint8_t i = 7; i < 15; ++i)
    {
        if (data & 0x01)
            cmp_buff[i] = 600; // 数据1对应的pwm比较值
        else
            cmp_buff[i] = 200; // 数据0对应的pwm比较值
        data = data >> 1;
    }

    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); // 开启PWM输出
    HAL_DMA_Start_IT(                         // 配置并开启DMA
        htim2.hdma[TIM_DMA_ID_UPDATE], (uint32_t)cmp_buff,
        (uint32_t)&htim2.Instance->CCR1, 16);

    tx_busy = 1; // 设置数据发送为忙状态
}

void wt588f_dma_callback(DMA_HandleTypeDef *_hdma)
{
    if (_hdma == htim2.hdma[TIM_DMA_ID_UPDATE])
    {
        tx_busy = 0;                             // 设置数据发送为空闲状态
        HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1); // 停止PWM输出
    }
}

// /**
//  * @brief 一线串口控制程序（双字节指令）
//  * @param data 待发送数据
//  */
// void Line_1A_WT588F_Couple(uint16_t data)
// {
//     uint8_t high_byte = data >> 8;    // 获取高字节
//     uint8_t low_byte = data & 0xFF;   // 获取低字节

//     SET_DATA_LOW;                     // 拉低数据线
//     PORT_DELAY_MS(5);                 // 延时5ms
//     for (uint8_t i = 0; i < 8; i++) { // 先发高字节
//         if (high_byte & 0X01) {       // 逻辑电平1
//             SET_DATA_HIGH;
//             PORT_DELAY_US(600);
//             SET_DATA_LOW;
//             PORT_DELAY_US(200);
//         }
//         else { // 逻辑电平0
//             SET_DATA_HIGH;
//             PORT_DELAY_US(200);
//             SET_DATA_LOW;
//             PORT_DELAY_US(600);
//         };

//         high_byte = high_byte >> 1;
//     }
//     SET_DATA_HIGH;
//     PORT_DELAY_MS(2); // 延时2ms

//     SET_DATA_LOW;
//     PORT_DELAY_MS(5);                 // 延时5ms
//     for (uint8_t i = 0; i < 8; i++) { // 再发低字节
//         if (low_byte & 0X01) {        // 逻辑电平1
//             SET_DATA_HIGH;
//             PORT_DELAY_US(600);
//             SET_DATA_LOW;
//             PORT_DELAY_US(200);
//         }
//         else { // 逻辑电平0
//             SET_DATA_HIGH;
//             PORT_DELAY_US(200);
//             SET_DATA_LOW;
//             PORT_DELAY_US(600);
//         };

//         low_byte = low_byte >> 1;
//     }

//     SET_DATA_HIGH;
// }

// /* --------------------------------------------------------------------------
// */
// /*                                    测试函数 */
// /* --------------------------------------------------------------------------
// */

// test code
/*--------------------------------------
;模块名称:wt588f_list_play()
;功    能:一线串口单字节连码示例(F3 + 01 + F3 + 02)
;入    参:
;出    参:
;-------------------------------------*/
void wt588f_list_play()
{
    wt588f_send_byte(0xF3);
    while (tx_busy)
    {
    }
    osDelay(2); // 延时2ms

    wt588f_send_byte(0x00);
    while (tx_busy)
    {
    }
    osDelay(5); // 延时5ms

    // wt588f_send_byte(0xFE);
    // while (tx_busy)
    // {
    // }
    // osDelay(50); // 延时5ms

    wt588f_send_byte(0xF3);
    while (tx_busy)
    {
    }
    osDelay(2); // 延时2ms

    wt588f_send_byte(0x01);
    while (tx_busy)
    {
    }
    osDelay(5); // 延时5ms

    // wt588f_send_byte(0xFE);
    // while (tx_busy)
    // {
    // }
    // osDelay(50); // 延时5ms

    wt588f_send_byte(0xF3);
    while (tx_busy)
    {
    }
    osDelay(2); // 延时2ms

    wt588f_send_byte(0x02);
    while (tx_busy)
    {
    }
    osDelay(5); // 延时5ms

    // wt588f_send_byte(0xFE);
    // while (tx_busy)
    // {
    // }
    // osDelay(50); // 延时5ms

}

// /*--------------------------------------
// ;模块名称:List_1A_Play_WT588F()
// ;功    能:一线串口双字节连码示例(FFF3 + 0001 + FFF3 + 0002)
// ;入    参:
// ;出    参:
// ;-------------------------------------*/
// void List_1A_Play_WT588F_Couple()
// {
//     Line_1A_WT588F_Couple(0xFFF3);
//     PORT_DELAY_MS(5);  // 延时5ms
//     Line_1A_WT588F_Couple(0x0001);
//     PORT_DELAY_MS(10); // 延时10ms
//     Line_1A_WT588F_Couple(0xFFF3);
//     PORT_DELAY_MS(5);  // 延时5ms
//     Line_1A_WT588F_Couple(0x0001);
//     PORT_DELAY_MS(10); // 延时10ms
// }
