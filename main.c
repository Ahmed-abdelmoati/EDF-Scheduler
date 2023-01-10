/*
 * consumeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.consumeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the consumeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"
#include "semphr.h"
/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"
/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/
void vTask1(void*);
void vTask2(void*);
void vTask3(void*);
void vTask4(void*);
void vTask5(void*);
void vTask6(void*);
  BaseType_t xTaskPeriodicCreate( TaskFunction_t, const char * const , const configSTACK_DEPTH_TYPE,void * const,UBaseType_t, TaskHandle_t * const,TickType_t );

typedef struct
{	
    unsigned char length;
    char message[ 15 ];
}queue_item;
	
typedef enum {
							Normal,/*USED to initialize previous variables*/
							Down,
							UP}
xButtonStatus ;

xButtonStatus B1Pre=Normal,B1Cur=Normal,B2Pre=Normal,B2Cur=Normal;
TaskHandle_t HANDEL_TASK1,HANDEL_TASK2,HANDEL_TASK3,HANDEL_TASK4,HANDEL_TASK5,HANDEL_TASK6;
QueueHandle_t queue;
int main( void )
{	
	prvSetupHardware();
	queue = xQueueCreate( 10, sizeof(  queue_item * ) );
	vSerialPutString((signed char*)"welcome\n",8);
  xTaskPeriodicCreate(vTask1,"B1",120,NULL,0,&HANDEL_TASK1,50);//50
	vTaskSetApplicationTaskTag( HANDEL_TASK1, (TaskHookFunction_t)PIN1 );
	xTaskPeriodicCreate(vTask2,"B2",120,NULL,0,&HANDEL_TASK2,50);//50
  vTaskSetApplicationTaskTag( HANDEL_TASK2, (TaskHookFunction_t)PIN2 );
	xTaskPeriodicCreate(vTask3,"PT",120,NULL,0,&HANDEL_TASK3,100);//100
	vTaskSetApplicationTaskTag( HANDEL_TASK3, (TaskHookFunction_t)PIN3 );
	xTaskPeriodicCreate(vTask4,"UR",120,NULL,0,&HANDEL_TASK4,20);//20
	vTaskSetApplicationTaskTag( HANDEL_TASK4, (TaskHookFunction_t)PIN4);
  xTaskPeriodicCreate(vTask5,"L1",120,NULL,0,&HANDEL_TASK5,10);/*E=5ms*/
	xTaskPeriodicCreate(vTask6,"L2",120,NULL,0,&HANDEL_TASK6,100);/*E=12ms*/
	vTaskSetApplicationTaskTag( HANDEL_TASK5, (TaskHookFunction_t)PIN5 );
	vTaskSetApplicationTaskTag( HANDEL_TASK6, (TaskHookFunction_t)PIN6 );	

	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the consumeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/
void vTask1(void*p){
TickType_t xLastWakeTime;	
const TickType_t xFrequency = 50;
queue_item Task1={3,"T1\n"};	//Rising edge	
queue_item RE1={3,"R1\n"};	//Rising edge
queue_item FE1={3,"F1\n"}; //Falling edge
xLastWakeTime=xTaskGetTickCount();
	while(1){
		B1Pre=B1Cur;
		xQueueSend(queue,&Task1,0);
		if(GPIO_read(PORT_0,PIN11)==PIN_IS_LOW)
				B1Cur=Down;
	  if(GPIO_read(PORT_0,PIN11)==PIN_IS_HIGH)
				B1Cur=UP;
		if(B1Pre==Down&&B1Cur==UP)
		{	
			xQueueSend(queue,&RE1,0);
		}
		else if(B1Pre==UP&&B1Cur==Down)
		{
			xQueueSend(queue,&FE1,0);
		}
	/*GPIO_write(0,16,1);*/	/*used with traceTASK_SWITCHED_IN() andtraceTASK_SWITCHED_OUT() macros to determine exec time*/
	vTaskDelayUntil( &xLastWakeTime, xFrequency );	
			
	}
}
void vTask2(void*p){
TickType_t xLastWakeTime;	
const TickType_t xFrequency = 50;	
queue_item Task2={3,"T2\n"};	//Rising edge	
queue_item RE2={3,"R2\n"};	//Rising edge 
queue_item FE2={3,"F2\n"}; //Falling edge	
xLastWakeTime=xTaskGetTickCount();
	while(1){
			B2Pre=B2Cur;
		xQueueSend(queue,&Task2,0);
		if(GPIO_read(PORT_0,PIN13)==PIN_IS_LOW)
				B2Cur=Down;
		if(GPIO_read(PORT_0,PIN13)==PIN_IS_HIGH)
				B2Cur=UP;
		if(B2Pre==Down&&B2Cur==UP)
		{	
			xQueueSend(queue,&RE2,0);
		}
		else if(B2Pre==UP&&B2Cur==Down)
		{
			xQueueSend(queue,&FE2,0);
		}
	/*GPIO_write(0,16,1);*/	/*used with traceTASK_SWITCHED_IN() andtraceTASK_SWITCHED_OUT() macros to determine exec time*/
	vTaskDelayUntil( &xLastWakeTime, xFrequency );	
	}
}
void vTask3(void*p){/*This task will send preiodic string every 100ms to the consumer task*/
	TickType_t xLastWakeTime;	
	const TickType_t xFrequency = 100;	
	queue_item P={3,"T3\n"}; //Perodic	
	xLastWakeTime = xTaskGetTickCount();	
	while(1)
	{
		xQueueSend(queue,&P,0);
		/*GPIO_write(0,16,1);*/	/*used with traceTASK_SWITCHED_IN() andtraceTASK_SWITCHED_OUT() macros to determine exec time*/
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}
void vTask4(void*p){/*This is the consumer task which will produce on UART any received string from other tasks*/
		TickType_t xLastWakeTime;
		queue_item rec;	
		const TickType_t xFrequency = 20;
		queue_item P={3,"T4\n"}; //Perodic	
		xLastWakeTime = xTaskGetTickCount();	
	while(1)
	{		
	if (xQueueReceive( queue, &rec ,( TickType_t ) 0 )==pdTRUE )
				vSerialPutString((signed char*)rec.message,rec.length);
	/*GPIO_write(0,16,1);*/	/*used with traceTASK_SWITCHED_IN() andtraceTASK_SWITCHED_OUT() macros to determine exec time*/
	vTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}
void vTask5(void*p){
TickType_t xLastWakeTime;		int i;
const TickType_t xFrequency = 10;
xLastWakeTime = xTaskGetTickCount();
while(1){

	for(i=0;i<33015;i++)
			i=i;
	vTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}
void vTask6(void*p){
TickType_t xLastWakeTime;		int i;
const TickType_t xFrequency = 100;	
xLastWakeTime = xTaskGetTickCount();
while(1){

	for(i=750;i<80000;i++)
			i=i;
	vTaskDelayUntil( &xLastWakeTime, xFrequency );	
	}
}


void vApplicationTickHook(){	
/*
GPIO_write(PORT_0, PIN8,PIN_IS_HIGH);
GPIO_write(PORT_0, PIN8,PIN_IS_LOW);
*/
}
void vApplicationIdleHook()	
{	
/*
GPIO_write(PORT_0, PIN7,PIN_IS_HIGH);
GPIO_write(PORT_0, PIN7,PIN_IS_LOW);
*/
}

	