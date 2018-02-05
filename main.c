/*
 * main.c
 *
 *  Created on: Feb 3, 2018
 *      Author: Khalid
 */

/* Include OS Header files */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
/* Include Drivers Header files */
#include <avr/io.h>
#include <util/delay.h>
#include "ADC/ADC.h"
#include "LCD/LCD.h"
#include "KeyPad/KeyPad.h"
/* Proto. */
void T_Temp (void * pvData);
void T_LCD (void * pvData);
void T_LED (void * pvData);
void T_KeyPad (void * pvData);
void T_Bazzer (void * pvData);
void System_Init(void);
/* OS Serv , Declaration */
xSemaphoreHandle countingsemaphore ;
xSemaphoreHandle bsKeyPressedEvent ;

long temp_Read;
unsigned int Key = 0;

#define set_bit(reg,pin) reg |= (1<<pin)

#define  clear_bit(reg,pin) reg &= ~(1<<pin)

int main ()
{
	/* Pr . Init .*/
	System_Init();
	/* OS Serv . Create And Init */
	countingsemaphore=xSemaphoreCreateCounting(10,0);
	vSemaphoreCreateBinary(bsKeyPressedEvent,0);
	/* Tasks Create */
	xTaskCreate(T_LCD,NULL,100,NULL,5,NULL);
	xTaskCreate(T_LED,NULL,100,NULL,1,NULL);
	xTaskCreate(T_Temp,NULL,100,NULL,2,NULL);
	xTaskCreate(T_KeyPad,NULL,100,NULL,3,NULL);
	xTaskCreate(T_Bazzer,NULL,100,NULL,4,NULL);
	/* Start OS or Sched */
	vTaskStartScheduler();
	while(1);
	return 0 ;
}

void T_LCD (void * pvData)
{
	int Pre_Temp=0;
	int Key_check =0;
	while(1)
	{
		if(xSemaphoreTake(countingsemaphore,1000))
		{
			if(temp_Read!=Pre_Temp)
			{
				LCD_GoTo(1,1);
				LCD_Write_Data((temp_Read/10)+48);
				LCD_Write_Data((temp_Read%10)+48);
				Pre_Temp=temp_Read;
				vTaskDelay(1000);
			}
			if(Key!=Key_check)
			{
				LCD_GoTo(2,1);
				LCD_Write_Data(Key+48);
				Key_check= Key;
				vTaskDelay(5);
			}
		}
	}
}
void T_Bazzer (void * pvData)
{
	while(1)
	{
		if(xSemaphoreTake(bsKeyPressedEvent,1000))
		{
			clear_bit(PORTD,5);
			vTaskDelay(50);
			set_bit(PORTD,5);
		}
	}
}


void T_KeyPad (void * pvData)
{
	while(1)
	{
		Key=KeyPad();
		xSemaphoreGive(bsKeyPressedEvent);
		vTaskDelay(5);
	}
}

void T_Temp (void * pvData)
{
	while(1)
	{
		temp_Read=ADC_READ(1);
		temp_Read *=500;
		temp_Read /=1023;
		xSemaphoreGive(countingsemaphore);
		vTaskDelay(5);
	}
}


void T_LED (void * pvData)
{
	while(1)
	{
		PORTD^=(1<<7);
		vTaskDelay(500);
	}
}


void System_Init(void)
{
	ADC_Init();
	LCD_init();
	LCD_Write_CMD(0x01);
	KeyPad_Init();
	// LED
	int i= 0;
	for(i=0 ;i<4;i++)
	{
		set_bit(DDRD,(i+4));
	}
	clear_bit(PORTD,5);
	set_bit(PORTD,4);
	set_bit(PORTD,6);
	set_bit(PORTD,7);
}
