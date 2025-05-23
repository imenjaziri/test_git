/*
 * ihm.h
 *
 *  Created on: May 5, 2025
 *      Author: ThinkPad
 */

#ifndef INC_IHM_H_
#define INC_IHM_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "usart.h"
#include "stm32l4xx_hal_uart.h"
#include "FreeRTOS.h"
#include "message_buffer.h"
#include "cmsis_os.h"
extern uint8_t currentStep;
extern uint8_t txBuffer[128];
extern uint8_t rx_flag;
extern uint8_t rxBuffer[64];
extern uint8_t rxByte;
void AfficherMenuPrincipal(void);
void ParseCommand();
void SetSF_f();
void GetSF_f();
void SetGPS_f();
void GetGPS_f();
void SAVESF_f();
void tokenization(char *str);


#endif /* INC_IHM_H_ */
