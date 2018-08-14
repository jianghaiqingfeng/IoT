#ifndef __UART_H
#define __UART_H
#include "stdio.h"	
#include "stm32f4xx.h"

//定义最大接收字节数 200
#define USART_REC_LEN  			200  	

//使能:1  禁止:0
#define EN_USART1_RX   1		

//串口数据接收缓冲区 
extern u8  USART_RX_BUF[USART_REC_LEN]; 

//接收状态标记
extern u16 USART_RX_STA;         			


/************************************************************************
** 函数名称: Uart1Init								
** 函数功能: 串口1初始化函数	
** 入口参数: u32 bound:波特率
** 出口参数: 无
** 备    注: 
************************************************************************/
void Uart1Init(u32 bound);



























#endif

