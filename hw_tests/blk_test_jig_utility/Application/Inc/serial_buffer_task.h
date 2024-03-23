/****************************************************************************/
/**
** Copyright 2022 Kirintec Ltd. All rights reserved.
**
** @file serial_buffer_task.h
**
** Include file for serial_buffer_task.c
**
** Project   : Blackstar
**
** Build instructions   : None, include file only
**
****************************************************************************/

/* Define to prevent recursive inclusion */
#ifndef __SERIAL_BUFFER_TASK_H
#define __SERIAL_BUFFER_TASK_H

/*****************************************************************************
*
*  Include
*
*****************************************************************************/
#include <stdbool.h>
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"

/*****************************************************************************
*
*  Global Definitions
*
*****************************************************************************/
#define SBT_TX_BUF_SIZE		16
#define SBT_MAX_NO_UARTS	2

/*****************************************************************************
*
*  Global Macros
*
*****************************************************************************/


/*****************************************************************************
*
*  Global Datatypes
*
*****************************************************************************/
typedef struct sbt_Uart
{
	UART_HandleTypeDef* huart;
	osMessageQId		uart_tx_data_queue;
	osMessageQId 		uart_rx_data_queue;
	uint8_t 			uart_rx_buf;
	uint8_t 			uart_tx_buf[SBT_TX_BUF_SIZE];
} sbt_Uart_t;

typedef struct sbt_Init
{
	osMessageQId		rx_event_queue;
	uint16_t			no_uarts;
	sbt_Uart_t			uarts[SBT_MAX_NO_UARTS];
} sbt_Init_t;

typedef struct sbt_Event
{
	uint8_t 	uart_idx;
	uint8_t 	data;
	uint16_t	spare;
} sbt_Event_t;

/*****************************************************************************
*
*  Global Functions
*
*****************************************************************************/
void sbt_InitTask(sbt_Init_t init_data);
void sbt_SerialBufferTask(void const *argument);

/*****************************************************************************
*
*  External Variables
*
*****************************************************************************/


#endif /* __SERIAL_BUFFER_TASK_H */