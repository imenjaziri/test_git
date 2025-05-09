/*
 * ihm.h
 *
 *  Created on: May 7, 2025
 *      Author: ThinkPad
 */

#ifndef INC_IHM_H_
#define INC_IHM_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "usart.h"
#include "stm32f4xx_hal_uart.h"
extern uint8_t currentStep;
extern char txBuffer[128];
void IHM_Init(void);
void Menu_function(void);
void AfficherMenuPrincipal(void);
void EnvoyerEtapeMenu(void) ;
void processMessage(uint8_t *rxBuffer);


#endif /* INC_IHM_H_ */
