#include "ihm.h"
#define RX_BUFFER_SIZE 64

uint8_t rxBuffer[RX_BUFFER_SIZE];
uint8_t new_buff[RX_BUFFER_SIZE];
uint8_t rxByte;
uint8_t rxIndex = 0;
uint8_t rx_flag;
uint8_t txBuffer[128];
uint8_t MessageBufferFlag=0;
uint8_t xBytesSent ;
uint8_t received_data[64];
MessageBufferHandle_t  MessageBufferHandle=NULL;
const size_t xMessageBufferSizeBytes = 100;
BaseType_t xHigherPriorityTaskWoken = pdFALSE;
uint8_t processing =0;

void Start_IHM_Task(void const * argument)
{
	/* USER CODE BEGIN Start_IHM_Task */
	char *msg = "IHM Task Running\r\n";
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 1000);
	AfficherMenuPrincipal();
	MessageBufferHandle = xMessageBufferCreate(xMessageBufferSizeBytes);
	if( MessageBufferHandle != NULL )
	{
	}
	else
	{
		HAL_UART_Transmit(&huart2,(uint8_t*)"Error in MessageBuffer Creation\r\n", 34, 100);
	}

	/* Infinite loop */
	for(;;)
	{
		xMessageBufferReceive( MessageBufferHandle, received_data, sizeof(received_data), portMAX_DELAY);
		processMessage(received_data);
		osDelay(1);
	}
	/* USER CODE END Start_IHM_Task */
}
// Fonction pour afficher le menu principal complet
void AfficherMenuPrincipal(void) {
	// Afficher tout le menu une seule fois
	sprintf((char*)txBuffer,"*************** MAIN MENU ***************\r\n");
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO ACCESS SENSORS DATA WRITE : SENSORS \r\n");
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO ACCESS GPS MENU WRITE : GPS\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO ACCESS LORA MENU, WRITE : LORA \r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO EXIT WRITE : EXIT\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO EXIT WRITE : EXIT\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	HAL_UART_Receive_IT(&huart2, &rxByte, 1);

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (!processing)
	{ if ((rxByte == '\r')||(rxByte=='\n'))
	{   if (rxIndex!=0)
	{
		processing=1;
		memcpy(new_buff,rxBuffer,rxIndex);
		rxBuffer[rxIndex] = '\0';
		xBytesSent=xMessageBufferSendFromISR(MessageBufferHandle,new_buff,strlen((char*)new_buff),&xHigherPriorityTaskWoken);
		//The number of bytes actually written to the message buffer.  If the
		// * message buffer didn't have enough free space for the message to be stored
		// * then 0 is returned, otherwise xDataLengthBytes is returned.
		if( xBytesSent != strlen((char*)new_buff))
		{
			HAL_UART_Transmit(&huart2, (uint8_t *)"Error Length Message Buffer\r\n",29,100);
			// The string could not be added to the message buffer because there was
			// not enough free space in the buffer.
		}
		else {HAL_UART_Transmit(&huart2, (uint8_t *)"Message sent\r\n",14,100);}
		//processing=1;
		rxIndex = 0;}
	}


	else {
		if (rxIndex < RX_BUFFER_SIZE-1) {
			rxBuffer[rxIndex++] = rxByte;
		}
		else {
			rxIndex=0;
		}
	}
	}
	HAL_UART_Receive_IT(&huart2, &rxByte, 1);

}
void processMessage(uint8_t *rxBuffer) {
	if (strcmp((char*)rxBuffer, "SET") == 0) {
		HAL_UART_Transmit(&huart2, (uint8_t*)"Success\r\n", 9, 100);

	} else if (strcmp((char*)rxBuffer, "exit") == 0) {
		HAL_UART_Transmit(&huart2, (uint8_t*)"Exiting...\r\n", 13, 100);

	} else {
		HAL_UART_Transmit(&huart2, (uint8_t*)"Unvalid command\r\n", 19, 100);
	}
	processing=0;
}

/*void processMessage(uint8_t *rxBuffer) {
    // Get the actual length of the received string
    size_t len = rxIndex;  // Use rxIndex to track valid data length

    // Check and strip `\r\n` (Carriage Return and Line Feed) from the end of the received data
    if (len > 1 && rxBuffer[len - 1] == '\n' && rxBuffer[len - 2] == '\r') {
        // Remove \r\n (both)
        rxBuffer[len - 2] = '\0';  // Remove `\r` and null-terminate at the correct position
    } else if (rxBuffer[len - 1] == '\n' || rxBuffer[len - 1] == '\r') {
        // If there's only `\n` or `\r` at the end, remove it
        rxBuffer[len - 1] = '\0';
    }

    // Convert the received message to lowercase for case-insensitive comparison
    for (size_t i = 0; i < len; i++) {
        rxBuffer[i] = (uint8_t)tolower(rxBuffer[i]);
    }

    // Now compare with "set" (all lowercase)
    if (strcmp((char*)rxBuffer, "set") == 0) {
        HAL_UART_Transmit(&huart2, (uint8_t*)"Success\r\n", 9, 100);
    } else if (strcmp((char*)rxBuffer, "exit") == 0) {
        HAL_UART_Transmit(&huart2, (uint8_t*)"Exiting...\r\n", 13, 100);
    } else {
        // For debugging, return the raw received command
        HAL_UART_Transmit(&huart2, (uint8_t*)"Command: ", 9, 100);
        HAL_UART_Transmit(&huart2, rxBuffer, len, 100);
        HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 100);
        HAL_UART_Transmit(&huart2, (uint8_t*)"Command not recognized\r\n", 24, 100);
    }
}
 */


