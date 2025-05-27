#include "ihm.h"
#define RX_BUFFER_SIZE 64
#define CR_VALUE_SIZE 7
#define BW_VALUE_SIZE 10
#define MIN_SOIL_TEMP -20
#define MAX_SOIL_TEMP 80
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
uint8_t sf_new_value;
uint8_t cmd_buff[60];
char Cr_new_value[7];
char Cr_Value[CR_VALUE_SIZE];
char bw_new_value[10];
char Bandwidth_Value[BW_VALUE_SIZE];
float SoilTemp_New_Value;
float AirTemp_New_Value;


typedef struct CMD {
	char* Name;
	char* helper;
	cmdHandler handler;
	uint8_t MenuIndex;
}CMD;
typedef enum {
	Main_Menu,
	Lora_Menu,
	GPS_Menu,
	Sensors_Menu
}Menu;
Menu currentMenu=Main_Menu;

//Useful functions for the code
void UpperCase(char *str){
	while (*str)
	{
		*str=toupper(*str);
		str++;
	}
}

void Start_IHM_Task(void const * argument)
{
	/* USER CODE BEGIN Start_IHM_Task */
	MainMenu();
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
		UpperCase((char*)received_data);
		tokenization((char*)received_data);
		ParseCommand();
		memset(received_data,0,sizeof(received_data));
		osDelay(1);
	}
	/* USER CODE END Start_IHM_Task */
}

// Fonction pour afficher le menu principal complet
void MainMenu(void) {
	// Afficher tout le menu une seule fois
	currentMenu=Main_Menu;
	sprintf((char*)txBuffer,"\033[1;30;107m----------------Main Menu---------------\033[0m\n \r\n");
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO ACCESS SENSORS DATA WRITE : SENSORS \r\n");
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO ACCESS SATELLITE PREDICTION MENU WRITE : SATELLITE\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO ACCESS GPS MENU WRITE : GPS\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO ACCESS LORA MENU, WRITE : LORA\r\n");
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
//tableau de liste des commandes
CMD cmd_list[]={
		{"LORA",(char*)0,LoraMenu,Main_Menu},
		{"SENSORS",(char*)0,SensorsMenu,Main_Menu},
		{"GPS",(char*)0,GPSMenu,Main_Menu},
		//Lora Menu Command List
		{"SETSF",(char*)0,SetSF_f,Lora_Menu},
		{"GETSF",(char*)0,GetSF_f,Lora_Menu},
		{"SAVESF",(char*)0,SaveSF_f,Lora_Menu},
		{"SETCR",(char*)0,SetCR_f,Lora_Menu},
		{"GETCR",(char*)0,GetCR_f,Lora_Menu},
		{"SAVECR",(char*)0,SaveCR_f,Lora_Menu},
		{"SETBW",(char*)0,SetBW_f,Lora_Menu},
		{"GETBW",(char*)0,GetBW_f,Lora_Menu},
		//Sensors Menu Command List
		{"SETST",(char*)0,SetSoilTemp_f,Sensors_Menu},
		{"GETST",(char*)0,GetSoilTemp_f,Sensors_Menu},
		{"SETAT",(char*)0,SetAirTemp_f,Sensors_Menu},
		{"GETAT",(char*)0,GetAirTemp_f,Sensors_Menu},
		{"SETAP",(char*)0,SetAirPressure_f,Sensors_Menu},
		{"GETAP",(char*)0,GetAirPressure_f,Sensors_Menu},
		{"SETRH",(char*)0,SetRelativeHumidity_f,Sensors_Menu},
		{"GETRH",(char*)0,GetRelativeHumidity_f,Sensors_Menu},
		{"SETSH",(char*)0,SetSoilHumidity_f,Sensors_Menu},
		{"GETSH",(char*)0,GetSoilHumidity_f,Sensors_Menu},
		{"SETWS",(char*)0,SetWindSpeed_f,Sensors_Menu},
		{"GETWS",(char*)0,GetWindSpeed_f,Sensors_Menu},
		{"SETRN",(char*)0,SetRadiation_f,Sensors_Menu},
		{"GETRN",(char*)0,GetRadiation_f,Sensors_Menu},
		{"SETKC",(char*)0,SetKc_f,Sensors_Menu},
		{"SETKC",(char*)0,GetKc_f,Sensors_Menu},
		{"SETKP",(char*)0,SetKp_f,Sensors_Menu},
		{"SETKP",(char*)0,GetKp_f,Sensors_Menu},
		{"SETET0",(char*)0,SetET0_f,Sensors_Menu},
		{"GETET0",(char*)0,GetET0_f,Sensors_Menu},
		//GPS Menu Command List
		{"SETALTGPS",(char*)0,SetAltGPS_f,GPS_Menu},
		{"GETALTGPS",(char*)0,GetAltGPS_f,GPS_Menu},
		{"SETLATGPS",(char*)0,SetLatGPS_f,GPS_Menu},
		{"GETLATGPS",(char*)0,GetLatGPS_f,GPS_Menu},
		{"SETUTC",(char*)0,SetTimeGPS_f,GPS_Menu},
		{"GETUTC",(char*)0,GetTimeGPS_f,GPS_Menu},

};
void tokenization(char *str) //function to tokenize input string
{
	tokens[0]=strtok(str," ");
	for (uint8_t i=1; i<10;i++)
	{   tokens[i]=strtok(NULL," ");
	if (tokens[i]==NULL)
		break;
	}
}
uint8_t cl_elements=sizeof(cmd_list)/sizeof(cmd_list[0]);
void ParseCommand() {
	uint8_t c=0;
	uint8_t correspond=0;
	uint8_t true=0;
	uint8_t goback=0;
	if (strcmp(tokens[0],"..")==0)
	{MainMenu();
	goback=1;}
	while (c<cl_elements)
	{if (strcmp(tokens[0], cmd_list[c].Name)== 0)
	{ true=1;
	if (currentMenu==cmd_list[c].MenuIndex)
	{cmd_list[c].handler(tokens[1]);
	correspond=1;}
	else HAL_UART_Transmit(&huart2, (uint8_t*)"Wrong Menu\r\n",strlen("Wrong Menu\r\n"),100);
	break;
	}
	c++;
	}
	if (true==0 && correspond==0 && goback==0)
		HAL_UART_Transmit(&huart2, (uint8_t*)"COMMAND ERROR\r\n",16,100);

	processing=0;
}

//Lora Menu Code
void LoraMenu(char* arg){
	currentMenu=Lora_Menu;
	sprintf((char*)txBuffer,"\033[1;38;2;25;25;112;107m--------------- LORA Menu ---------------\033[0m\n \r\n");
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO SET VALUE WRITE : SETSF 5 \r\n");
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO GET SF VALUE WRITE : GETSF\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO SET CR VALUE WRITE : SETCR 4/5 \r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO GET CR VALUE WRITE : GETCR\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO RESET TO THE PREVIOUS VALUE WRITE : RESET\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO SAVE CHANGES WRITE : SAVE\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);
	sprintf((char*)txBuffer, "TO GO BACK TO THE PREVIOUS MENU WRITE : .. \r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);
}
void SetSF_f(char* arg){
	uint8_t MAX_TH_SF=12;
	uint8_t MIN_TH_SF=6;
	uint8_t success = 0;

	if (tokens[1] != NULL && strlen(tokens[1]) < 3) {
		int sf_new_value = atoi(tokens[1]);

		if (sf_new_value >= MIN_TH_SF && sf_new_value <= MAX_TH_SF) {
			sprintf((char*)cmd_buff, "SF VALUE SET TO %d SUCCESSFULLY\r\n", sf_new_value);
			HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
			success = 1;
		}
	}

	if (success==0) {
		HAL_UART_Transmit(&huart2, (const uint8_t*)"INVALID VALUE\r\n", 16, 100);
	}

	memset(cmd_buff, 0, sizeof(cmd_buff));  // always clear at the end
}
void SaveSF_f(char* arg){
	SF_Value=sf_new_value;
	sprintf((char*)cmd_buff, "CR VALUE SAVED PERMANENTLY TO %d SUCCESSFULLY\r\n", SF_Value);
	HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));


}
void GetSF_f(char* arg)
{
	sprintf((char*)cmd_buff,"SF VALUE IS %d \r\n",SF_Value);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}
void SetCR_f(char* arg)
{   uint8_t cr_flag=0;
char* CR_Values[]={"4/5","4/6","4/7","4/8"};
for (uint8_t cr=0 ; cr<sizeof(CR_Values) / sizeof(CR_Values[0]);cr++)
{if (tokens[1]!=NULL &&
		strcmp(tokens[1],CR_Values[cr])==0)
{strcpy(Cr_new_value,tokens[1]);
cr_flag=1;
sprintf((char*)cmd_buff, "CR VALUE SET TO %s SUCCESSFULLY\r\n", Cr_new_value);
HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
memset(cmd_buff,0,sizeof(cmd_buff));
break;
}
}

if (cr_flag==0)
	HAL_UART_Transmit(&huart2,(const uint8_t*)"WRONG CR VALUE\r\n",strlen("WRONG CR VALUE\r\n"), 100);
}
void SaveCR_f(char* arg)
{
	strcpy(Cr_Value,Cr_new_value);
	sprintf((char*)cmd_buff, "CR VALUE SAVED PERMANENTLY TO %s SUCCESSFULLY\r\n", Cr_Value);
	HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}
void GetCR_f(char* arg)
{
	strcpy(Cr_Value,Cr_new_value);
	sprintf((char*)cmd_buff,"CR VALUE IS %s \r\n",Cr_Value);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}
void SetBW_f(char* arg)
{
	uint8_t bw_flag=0;
	char* BW_Values[]={"125","250","500"};
	for (uint8_t bw=0 ; bw<sizeof(BW_Values) / sizeof(BW_Values[0]);bw++)
	{if (tokens[1]!=NULL &&
			strcmp(tokens[1],BW_Values[bw])==0)
	{strcpy(bw_new_value,tokens[1]);
	bw_flag=1;
	sprintf((char*)cmd_buff, "BW VALUE SET TO %s SUCCESSFULLY\r\n", bw_new_value);
	HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
	break;
	}
	}

	if (bw_flag==0)
		HAL_UART_Transmit(&huart2,(const uint8_t*)"WRONG BW VALUE\r\n",strlen("WRONG CR VALUE\r\n"), 100);
}
void GetBW_f(char* arg)
{
	strcpy(Bandwidth_Value,bw_new_value);
	sprintf((char*)cmd_buff,"BW VALUE IS %s \r\n",Bandwidth_Value);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}
//GPS Menu
void GPSMenu(char* arg){
	currentMenu=GPS_Menu;
	sprintf((char*)txBuffer,"------------------ GPS MENU -----------------\r\n");
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO SET VALUE WRITE : SETSF 5 \r\n");
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO GET SF VALUE WRITE : GETSF\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO SET CR VALUE WRITE : SETCR 4/5 \r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO GET CR VALUE WRITE : GETCR\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);
	sprintf((char*)txBuffer, "TO GO BACK TO THE PREVIOUS MENU WRITE : .. \r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);
}
void SetAltGPS_f(char* arg){
	;
}
void GetAltGPS_f(char* arg){
	;
}
void SetLatGPS_f(char* arg){
	;
}
void GetLatGPS_f(char* arg){
	;
}
void SetTimeGPS_f(char* arg){
	;
}
void GetTimeGPS_f(char* arg){
	;
}

void SensorsMenu(char* arg){
	currentMenu=Sensors_Menu;
	sprintf((char*)txBuffer,"------------------ SENSORS MENU -----------------\r\n");
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO SET SOIL TEMPERATURE VALUE WRITE : SETST 26 \r\n");
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO GET SOIL TEMPERATURE VALUE WRITE : GETST \r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO SET AIR TEMPERATURE VALUE WRITE : SETAT 27.5 \r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO GET AIR TEMPERATURE VALUE WRITE : GETAT \r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO SET RELATIVE HUMIDITY VALUE WRITE : SETRH 100\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);

	sprintf((char*)txBuffer, "TO GET RELATIVE HUMIDITY VALUE WRITE : GETRH \r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);
	sprintf((char*)txBuffer, "TO SET SOIL HUMIDITY VALUE WRITE : SETSH 100\r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);
	sprintf((char*)txBuffer, "TO GET SOIL HUMIDITY VALUE WRITE : GETSH \r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);
	sprintf((char*)txBuffer, "TO GO BACK TO THE PREVIOUS MENU WRITE : .. \r\n");
	HAL_UART_Transmit(&huart2,txBuffer, strlen((char*)txBuffer), 100);
}

void SetSoilTemp_f(char* arg){
	uint8_t soiltemp = 0;
	if (tokens[1] != NULL && strlen(tokens[1]) < 5) {
		SoilTemp_New_Value = atof(tokens[1]);

		if (SoilTemp_New_Value >= MIN_SOIL_TEMP && sf_new_value <= MAX_SOIL_TEMP) {
			sprintf((char*)cmd_buff, "SF VALUE SET TO %f SUCCESSFULLY\r\n", SoilTemp_New_Value);
			HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
			soiltemp = 1;
		}
	}

	if (soiltemp==0) {
		HAL_UART_Transmit(&huart2, (const uint8_t*)"INVALID VALUE\r\n", 16, 100);
	}

	memset(cmd_buff, 0, sizeof(cmd_buff));  // always clear at the end
}

void GetSoilTemp_f(char* arg){
	;
}
void SetAirTemp_f(char* arg) {
	uint8_t airtemp = 0;
	if (tokens[1] != NULL && strlen(tokens[1]) < 5) {
		AirTemp_New_Value = atof(tokens[1]);

		if (AirTemp_New_Value >= MIN_SOIL_TEMP && sf_new_value <= MAX_SOIL_TEMP) {
			sprintf((char*)cmd_buff, "AIR TEMPERATURE VALUE SET TO %f SUCCESSFULLY\r\n", AirTemp_New_Value);
			HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
			airtemp = 1;
		}
	}

	if (airtemp==0) {
		HAL_UART_Transmit(&huart2, (const uint8_t*)"INVALID VALUE\r\n", 16, 100);
	}

	memset(cmd_buff, 0, sizeof(cmd_buff));
}
void GetAirTemp_f(char* arg){
	;
}
void SetRelativeHumidity_f(char* arg){
	;
}
void GetRelativeHumidity_f(char* arg){
	;
}
void SetSoilHumidity_f(char* arg){
	;
}
void GetSoilHumidity_f(char* arg){
	;
}
void SetWindSpeed_f(char* arg){
	;
}
void GetWindSpeed_f(char* arg){
	;
}
void SetRadiation_f(char* arg){
	;
}
void GetRadiation_f(char* arg){
	;
}
void SetKc_f(char* arg){
	;
}
void GetKc_f(char* arg){
	;
}
void SetKp_f(char* arg){
	;
}
void GetKp_f(char* arg){
	;
}
void GetET0_f(char* arg){
	;
}
void SetET0_f(char* arg){
	;
}
void SetAirPressure_f(char* arg){
	;

}
void GetAirPressure_f(char* arg){
	;

}
