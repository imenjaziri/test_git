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
volatile uint8_t retour=0;
typedef void (*cmdHandler)(char *arg); // parameters: token[2], token[3] (TYPE OF VALUE+VAL), number of arguments (tokens)
char *tokens[10];
uint8_t SF_Value;
uint8_t sf_buff[35];

void Start_IHM_Task(void const * argument)
{
	/* USER CODE BEGIN Start_IHM_Task */
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
		memset(new_buff,0,sizeof(new_buff));
		tokenization((char*)received_data);
		ParseCommand();
		memset(received_data,0,sizeof(received_data));
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

	sprintf((char*)txBuffer, "TO ACCESS SATELLITE PREDICTION MENU WRITE : SATELLITE\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO ACCESS GPS MENU WRITE : GPS\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO ACCESS LORA MENU, WRITE : LORA\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO RESET TO THE PREVIOUS STATE WRITE : RESET\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO SAVE CHANGES WRITE : SAVE\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	HAL_UART_Receive_IT(&huart2, &rxByte, 1);

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (!processing)
	{ if ((rxByte == '\r')||(rxByte=='\n'))
	{   if (rxIndex!=0)
	{
		processing=1;
		rxBuffer[rxIndex] = '\0';
		memcpy(new_buff,rxBuffer,rxIndex);
		xBytesSent=xMessageBufferSendFromISR(MessageBufferHandle,new_buff,strlen((char*)new_buff),&xHigherPriorityTaskWoken);
		//The number of bytes actually written to the message buffer.  If the
		// * message buffer didn't have enough free space for the message to be stored
		// * then 0 is returned, otherwise xDataLengthBytes is returned.
		if( xBytesSent != strlen((char*)new_buff))
		{
			HAL_UART_Transmit(&huart2, (const uint8_t *)"Message sent !=buffer data\r\n",26,100);
		}
		memset(rxBuffer,0,sizeof(rxBuffer));
		rxIndex = 0;}
	}

	else { if (rxIndex > 0){

		if (rxByte == '\b') {
			rxBuffer[rxIndex]=' ';
			rxIndex=rxIndex-1;
			rxBuffer[rxIndex]=' ';
			retour=rxIndex-1;
			HAL_UART_Transmit(huart, (uint8_t *)" \b", 2, 100);

			//	for (uint8_t i=0;i< strlen((char*)rxBuffer);i++)
			//{rxBuffer[i]=rxBuffer[retour++];
			//if (i==retour)
			//rxBuffer[i]='\0';}
		}
	}
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
typedef struct CMD {
	char* Name;
	char* helper;
	cmdHandler handler;
	uint8_t MenuIndex;
}CMD;
void tokenization(char *str) //function to tokenize input string
{
	tokens[0]=strtok(str," ");
	for (uint8_t i=1; i<10;i++)
	{   tokens[i]=strtok(NULL," ");
	if (tokens[i]==NULL)
		break;
	}
}
//tableau de liste des commandes
CMD cmd_list[]={{"SETSF",(char*)0,SetSF_f,0}, //comparer value ou threshold dans la fonction pas besoin ici
		{"GETSF",(char*)0,GetSF_f,0}, //Idée: faire plusieurs tableaux selon le menu (exple lora_cmd_list)
		/*{"SETCR",'0',SetCR_f,0},
		{"GETCR",'0',GetCR_f,0},
		{"SETBW",0,SetBW_f,0},
		{"GETBW",0,GetBW_f,0},
		{"SETALTGPS",0,SetAltGPS_f,0},
		{"GETALTGPS",0,GetAltGPS_f,0},
		{"SETGPS",0,SetTempSoil_f,0},
		{"GETGPS",0,GetTempSoil_f,0},
		{"SETGPS",0,SetTempAir_f,0},
		{"GETGPS",0,GetTempsAir_f,0},
		{"SETRELATIVE",0,SetRelativeHumidity_f,0},
		{"GETRELATIVE",0,GetRelativeHumidity_f,0},
		{"SETSH",0,SetSoilHumidity_f,0},
		{"GETSH",0,GetSoilHumidity_f,0},
		{"SETLATGPS",0,SetLatGPS_f,0},
		{"GETLATGPS",0,GetLatGPS_f,0},
		{"SETGPS",0,SetTimeGPS_f,0},
		{"GETGPS",0,GetTimeGPS_f,0},
		{"SETGPS",0,SetGPS_f,0},
		{"GETGPS",0,GetGPS_f,0},*/
};
uint8_t cl_elements=sizeof(cmd_list)/sizeof(cmd_list[0]);
void SetSF_f(char* arg){
	uint8_t MAX_TH_SF=12;
	uint8_t MIN_TH_SF=6;
	uint8_t success = 0;

	    if (tokens[1] != NULL && strlen(tokens[1]) < 3) {
	        int sf_new_value = atoi(tokens[1]);

	        if (sf_new_value >= MIN_TH_SF && sf_new_value <= MAX_TH_SF) {
	            SF_Value = sf_new_value;
	            sprintf((char*)sf_buff, "SF VALUE SET TO %d SUCCESSFULLY\r\n", SF_Value);
	            HAL_UART_Transmit(&huart2, sf_buff, strlen((char*)sf_buff), 100);
	            success = 1;
	        }
	    }

	    if (success==0) {
	        HAL_UART_Transmit(&huart2, (const uint8_t*)"INVALID VALUE\r\n", 16, 100);
	    }

	    memset(sf_buff, 0, sizeof(sf_buff));  // always clear at the end
	}
void GetSF_f(char* arg)
{
	sprintf((char*)sf_buff,"SF VALUE IS %d \r\n",SF_Value);
	HAL_UART_Transmit(&huart2,sf_buff,strlen((char*)sf_buff), 100);
}

//After tokenizing , check the type , check type + subname , activate the function
//inside the function check the value
void ParseCommand() {
	uint8_t c=0;
	uint8_t correspond=0;
	while (c<cl_elements)
	{if (strcmp(tokens[0], cmd_list[c].Name)== 0)
	{ cmd_list[c].handler(tokens[1]);
	correspond=1;
	}
	c++;
	}
	if (correspond==0)
	{HAL_UART_Transmit(&huart2, (uint8_t*)"COMMAND ERROR\r\n",16,100);}
	processing=0;
}
/*void AfficherMenuCapteurs(void){
	sprintf((char*)txBuffer, "TO ACCESS TEMPERATURE FEATURES WRITE : TEMPERATURE\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);
	sprintf((char*)txBuffer, "TO ACCESS HUMIDITY FEATURES WRITE : HUMIDITY\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);
	sprintf((char*)txBuffer, "TO ACCESS AIR FEATURES WRITE : AIR\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);
	sprintf((char*)txBuffer, "TO GO BACK TO THE PREVIOUS MENU WRITE : ..\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);
}
void*/

