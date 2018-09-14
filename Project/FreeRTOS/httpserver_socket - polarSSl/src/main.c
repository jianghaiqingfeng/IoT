/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4x7_eth.h"
#include "netconf.h"
#include "main.h"
#include "tcpip.h"
#include "httpserver-socket.h"
#include "serial_debug.h"
#include "bsp.h"
#include "mqttclient.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
extern __IO uint32_t  EthStatus;
/*--------------- LCD Messages ---------------*/
#if defined (STM32F40XX)
#define MESSAGE1   "    STM32F40/41x     "
#elif defined (STM32F427X)
#define MESSAGE1   "     STM32F427x      "
#endif
#define MESSAGE2   "  STM32F-4 Series   "
#define MESSAGE3   "Basic WebServer Demo"
#define MESSAGE4   "                    "

/*--------------- Tasks Priority -------------*/
#define MAIN_TASK_PRIO   ( tskIDLE_PRIORITY + 1 )
#define DHCP_TASK_PRIO   ( tskIDLE_PRIORITY + 4 )      
#define LED_TASK_PRIO    ( tskIDLE_PRIORITY + 2 )
#define MQTT_TASK_PRIO    ( tskIDLE_PRIORITY +4 )
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void LCD_LED_Init(void);
void ToggleLed4(void * pvParameters);
void Main_task(void * pvParameters);
void Beep(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured to 
       168 MHz, this is done through SystemInit() function which is called from
       startup file (startup_stm32f4xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f4xx.c file
     */  
  
  /* Configures the priority grouping: 4 bits pre-emption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	//���ô���1
	Uart1Init(115200);
	
	printf("//***********************************************/\n");
	printf("/**            2017��12��01��                 **/\n");
	printf("/***********************************************/\n");
	
	
	printf(">>>������ʼ������׼��ִ��ϵͳ��ʼ������...OK\n");
	
  /* Init task */
  xTaskCreate(Main_task, (int8_t *) "Main", configMINIMAL_STACK_SIZE *2, NULL,MAIN_TASK_PRIO, NULL);

	printf(">>��ʼ�������...OK\n");
  /* ������������������ʼִ��*/
  vTaskStartScheduler();

  /* We should never get here as control is now taken by the scheduler */
	/*���һ��������main������ִ֪�е�������ִ�е�����ܿ������ڴ治�㵼�¿��������޷�����*/
  for( ;; );
}

void Main_task(void * pvParameters)
{
#ifdef SERIAL_DEBUG
  DebugComPort_Init();
#endif

  /*Initialize LCD and Leds */ 
	printf(">>����IO��ʼ��...OK\r\n");
  LCD_LED_Init();
	
	//��ʼ����sram
	printf(">>��ʼ���ⲿ��չ�ڴ�\r\n");
	FSMC_SRAM_Init();
	
  /* configure ethernet (GPIOs, clocks, MAC, DMA) */ 
	printf(">>��ʼ��̫��ģ������......\r\n");
  ETH_BSP_Config();
	if((EthStatus&(0x01)) == 0)
	{
		printf(">>����ʧ�ܣ����������Ƿ���---ERR\n");
	}
	else
	{
		printf(">>������̫���ɹ�...OK\r\n");
	}
	
  /* Initilaize the LwIP stack */
  LwIP_Init();
	printf(">>��ʼ��LWIPЭ��ջ�ɹ�...OK\r\n");
	
	
  /* Initialize webserver demo */
	printf(">>HTTP server ��ʼ��\r\n");
  //http_server_socket_init();

	
  #ifdef USE_DHCP
  /* Start DHCPClient */
	printf(">>����ΪDHCPģʽ������DHCP����...OK\n");
  xTaskCreate(LwIP_DHCP_task, (int8_t *) "DHCP", configMINIMAL_STACK_SIZE * 2, NULL,DHCP_TASK_PRIO, NULL);
  #endif

  /* Start toogleLed4 task : Toggle LED4  every 250ms */
  xTaskCreate(ToggleLed4, (int8_t *) "LED1", configMINIMAL_STACK_SIZE*2, NULL, LED_TASK_PRIO, NULL);
	
	//�����������һ��
	Beep();
	printf(">>ϵͳ��ʼ����ϣ�������������״̬...OK\r\n");

	
	//����MQTT����
	xTaskCreate(mqtt_thread, (int8_t *) "MQTT", configMINIMAL_STACK_SIZE*8, NULL, MQTT_TASK_PRIO, NULL);


  for( ;; )
  {
      vTaskDelete(NULL);
  }
} 


/**
  * @brief  Toggle Led4 task
  * @param  pvParameters not used
  * @retval None
  */
void ToggleLed4(void * pvParameters)
{
  for( ;; )
  {
    /* toggle LED4 each 1000ms */
    vTaskDelay(1000/portTICK_RATE_MS);
  }
}

/**
  * @brief  Initializes the LCD and LEDs resources.
  * @param  None
  * @retval None
  */
void LCD_LED_Init(void)
{
#ifdef USE_LCD
  /* Initialize the STM324xG-EVAL's LCD */
  STM324xG_LCD_Init();
#endif

  /* Initialize STM324xG-EVAL's LEDs */
  STM_EVAL_LEDInit(LED1);
  STM_EVAL_LEDInit(LED2);
	//Ϩ��LED2
	STM_EVAL_LEDOff(LED2);
	
  STM_EVAL_LEDInit(LED3);
	//ֹͣ����
	STM_EVAL_LEDOn(LED3);
  //STM_EVAL_LEDInit(LED4);
	
	
  
#ifdef USE_LCD
  /* Clear the LCD */
  LCD_Clear(Black);

  /* Set the LCD Back Color */
  LCD_SetBackColor(Black);

  /* Set the LCD Text Color */
  LCD_SetTextColor(White);

  /* Display message on the LCD*/
  LCD_DisplayStringLine(Line0, (uint8_t*)MESSAGE1);
  LCD_DisplayStringLine(Line1, (uint8_t*)MESSAGE2);
  LCD_DisplayStringLine(Line2, (uint8_t*)MESSAGE3);
  LCD_DisplayStringLine(Line3, (uint8_t*)MESSAGE4);  
#endif
}

/**
  * @brief  Inserts a delay time.
  * @param  nCount: number of Ticks to delay.
  * @retval None
  */
void Delay(uint32_t nCount)
{
  vTaskDelay(nCount);
}

/**
  * @brief  beep.
  * @param  nCount: number of Ticks to delay.
  * @retval None
  */
void Beep(void)
{
	STM_EVAL_LEDOff(LED3);
  vTaskDelay(500);
	STM_EVAL_LEDOn(LED3);
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
