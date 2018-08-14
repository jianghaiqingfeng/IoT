#ifndef __UART_H
#define __UART_H
#include "stdio.h"	
#include "stm32f4xx.h"

//�����������ֽ��� 200
#define USART_REC_LEN  			200  	

//ʹ��:1  ��ֹ:0
#define EN_USART1_RX   1		

//�������ݽ��ջ����� 
extern u8  USART_RX_BUF[USART_REC_LEN]; 

//����״̬���
extern u16 USART_RX_STA;         			


/************************************************************************
** ��������: Uart1Init								
** ��������: ����1��ʼ������	
** ��ڲ���: u32 bound:������
** ���ڲ���: ��
** ��    ע: 
************************************************************************/
void Uart1Init(u32 bound);



























#endif

