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
uint8_t SF_Lora_Value;
uint8_t cmd_buff[100];
uint8_t Cr_new_value;
uint8_t bw_new_value;
float SoilTemp_New_Value;
float AirTemp_New_Value;
uint8_t Old_Default_Sf=7;
uint8_t Old_Default_Bw=5;
uint8_t Old_Default_Cr=6;
typedef struct {
	char* Name;
	char* helper;
	cmdHandler handler;
	uint8_t MenuIndex;
}CMD;
typedef enum {
	Main_Menu,
	Lora_Menu,
	GPS_Menu,
	Sensors_Menu,
	SysConfig_Menu
}Menu;
Menu currentMenu=Main_Menu;
typedef struct {
	uint8_t sf_l;
	uint8_t cr_l ;
	uint8_t bw_l ;
}Lora;
Lora LoraValues = {7, 5, 6};
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
		{"LORA",(char*)":TO ACCESS LORA MENU WRITE LORA",LoraMenu,Main_Menu},
		{"SENSORS",(char*)":TO ACCESS SENSORS MENU WRITE SENSORS",SensorsMenu,Main_Menu},
		{"GPS",(char*)":TO ACCESS GPS MENU WRITE GPS",GPSMenu,Main_Menu},
		{"SYSCONF",(char*)":TO ACCESS SYSTEM CONFIGURATION WRITE SYSCONF",SysConfigMenu,Main_Menu},
		//Lora Menu Command List
		{"SETSF",(char*)":TO SET SF VALUE WRITE SETSF 7\r\n[Possible Values :{6,7,8,9,10,11,12}]",SetSF_f,Lora_Menu},
		{"GETSF",(char*)":TO GET SF VALUE WRITE GETSF",GetSF_f,Lora_Menu},
		{"SETCR",(char*)":TO SET CR VALUE WRITE SETCR VALUE\r\n[Value for each CR :{4/5 -> 1 ; 4/6 -> 2 ; 4/7 -> 3 ; 4/8 -> 4}] ",SetCR_f,Lora_Menu},
		{"GETCR",(char*)":TO GET CR WRITE GETCR",GetCR_f,Lora_Menu},
		{"SETBW",(char*)":TO SET BANDWIDTH VALUE WRITE BW 4 \r\n[Values for BW :{125Khz->4 ; 250Khz->5 ; 500Khz->6}] ",SetBW_f,Lora_Menu},
		{"GETBW",(char*)":TO GET BANDWIDTH VALUE WRITE GETBW\r\nNote : you must respect the Values provided ",GetBW_f,Lora_Menu},
		//Sensors Menu Command List
		{"SETST",(char*)"TO SET SOIL TEMPERATURE VALUE WRITE SETST 30\r\n[Possible Values :-20 ->80]",SetSoilTemp_f,Sensors_Menu},
		{"GETST",(char*)":TO GET SOIL TEMPERATURE VALUE WRITE GETST",GetSoilTemp_f,Sensors_Menu},
		{"SETAT",(char*)":TO SET AIR TEMPERATURE VALUE WRITE : SETAT 27.5",SetAirTemp_f,Sensors_Menu},
		{"GETAT",(char*)":TO GET AIR TEMPERATURE VALUE WRITE GETAT",GetAirTemp_f,Sensors_Menu},
		{"SETAP",(char*)":TO SET AIR PRESSURE VALUE WRITE SETAP",SetAirPressure_f,Sensors_Menu},
		{"GETAP",(char*)":TO GET AIR PRESSURE VALUE WRITE GETAP",GetAirPressure_f,Sensors_Menu},
		{"SETRH",(char*)":TO SET RELATIVE HUMIDITY VALUE WRITE SETRH",SetRelativeHumidity_f,Sensors_Menu},
		{"GETRH",(char*)":TO GET RELATIVE HUMIDITY VALUE WRITE GETRH",GetRelativeHumidity_f,Sensors_Menu},
		{"SETSH",(char*)":TO SET SOIL HUMIDITY VALUE WRITE SETSH",SetSoilHumidity_f,Sensors_Menu},
		{"GETSH",(char*)":TO GET SOIL HUMIDITY VALUE WRITE GETSH",GetSoilHumidity_f,Sensors_Menu},
		{"SETWS",(char*)":TO SET WIND SPEED VALUE WRITE SGETWS",SetWindSpeed_f,Sensors_Menu},
		{"GETWS",(char*)":TO GET WIND SPEED VALUE WRITE GETWS",GetWindSpeed_f,Sensors_Menu},
		{"SETRN",(char*)":TO SET NET RADIATION VALUE WRITE SETRN",SetRadiation_f,Sensors_Menu},
		{"GETRN",(char*)":TO GET NET RADIATION VALUE WRITE GETRN",GetRadiation_f,Sensors_Menu},
		{"SETKC",(char*)":TO SET Kc VALUE WRITE SETKC",SetKc_f,Sensors_Menu},
		{"GETKC",(char*)":TO GET Kc VALUE WRITE GETKC",GetKc_f,Sensors_Menu},
		{"SETKP",(char*)":TO SET Kp VALUE WRITE SETKP",SetKp_f,Sensors_Menu},
		{"GETKP",(char*)":TO GET Kp VALUE WRITE GETKP",GetKp_f,Sensors_Menu},
		{"SETET0",(char*)":TO SET ET0 VALUE WRITE GETET0",SetET0_f,Sensors_Menu},
		{"GETET0",(char*)":TO GET ET0 VALUE WRITE GETET0\r\nOnly numbers with 1 digit after the decimal point are allowed / Example : 25.5",GetET0_f,Sensors_Menu},
		//GPS Menu Command List
		{"SETALT",(char*)"TO SET ALTITUDE VALUE WRITE SETALT",SetAltGPS_f,GPS_Menu},
		{"GETALT",(char*)":TO GET ALTITUDE VALUE WRITE GETALT",GetAltGPS_f,GPS_Menu},
		{"SETLAT",(char*)":TO SET LATITUDE VALUE WRITE SETLAT",SetLatGPS_f,GPS_Menu},
		{"GETLAT",(char*)":TO GET LATITUDE VALUE WRITE GETLAT",GetLatGPS_f,GPS_Menu},
		{"SETUTC",(char*)":TO SET TIME VALUE WRITE SETUTC",SetTimeGPS_f,GPS_Menu},
		{"GETUTC",(char*)":TO GET TIME VALUE WRITE GETUTC",GetTimeGPS_f,GPS_Menu},
		//SystemConfig Menu
		{"SAVE",(char*)"TO SAVE MODIFIED PARAMETERS PERMANENTLY WRITE SAVE",Save_f,SysConfig_Menu},
		{"RESTORE",(char*)"TO RESTORE OLD PARAMETERS WRITE RESTORE",Restore_f,SysConfig_Menu},

};
void MainMenu(void) {
	// Afficher tout le menu une seule fois
	currentMenu=Main_Menu;
	sprintf((char*)txBuffer,"\033[1;30;107m----------------Main Menu---------------\033[0m\n \r\n");
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	for (uint8_t l=0;l<sizeof(cmd_list)/sizeof(cmd_list[0]);l++)
	{if (cmd_list[l].MenuIndex==Main_Menu)
	{sprintf((char*)txBuffer,"%s %s \r\n",cmd_list[l].Name, cmd_list[l].helper);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	}
	}

	HAL_UART_Receive_IT(&huart2, &rxByte, 1);

}
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

	for (uint8_t l=0;l<sizeof(cmd_list)/sizeof(cmd_list[0]);l++)
	{if (cmd_list[l].MenuIndex==Lora_Menu)
	{sprintf((char*)txBuffer,"%s %s \r\n",cmd_list[l].Name, cmd_list[l].helper);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	}
	}
}
void SetSF_f(char* arg){
	uint8_t MAX_TH_SF=12;
	uint8_t MIN_TH_SF=5;
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
/*void SaveSF_f(char* arg){
switch (sf_new_value)
{
case 5:
	LoraValues.sf_l=0x05;
case 6:
	LoraValues.sf_l=0x06;
case 7:
	LoraValues.sf_l=0x07;
case 8:
	LoraValues.sf_l=0x08;
case 9:
	LoraValues.sf_l=0x09;
case 10:
	LoraValues.sf_l=0x0A;
case 11:
	LoraValues.sf_l=0x0B;
case 12:
	LoraValues.sf_l=0x0C;
}

}*/
void GetSF_f(char* arg)
{
	sprintf((char*)cmd_buff,"SF VALUE IS %d \r\nSF DEFAULT VALUE IS %d",sf_new_value,LoraValues.sf_l);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}
void SetCR_f(char* arg)
{   uint8_t cr_flag=0;
char* CR_Values[]={"1","2","3","4"};
for (uint8_t cr=0 ; cr<sizeof(CR_Values) / sizeof(CR_Values[0]);cr++)
{if (tokens[1]!=NULL &&
		strcmp(tokens[1],CR_Values[cr])==0)
{Cr_new_value=atoi(tokens[1]);
cr_flag=1;
sprintf((char*)cmd_buff, "CR VALUE SET TO %d SUCCESSFULLY\r\n", Cr_new_value);
HAL_UART_Transmit(&huart2, cmd_buff, strlen((char*)cmd_buff), 100);
memset(cmd_buff,0,sizeof(cmd_buff));
break;
}
}

if (cr_flag==0)
	HAL_UART_Transmit(&huart2,(const uint8_t*)"WRONG CR VALUE\r\n",strlen("WRONG CR VALUE\r\n"), 100);
}

void GetCR_f(char* arg)
{
	sprintf((char*)cmd_buff,"CR VALUE IS %d \r\n",Cr_new_value);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}
void SetBW_f(char* arg)
{
	uint8_t bw_flag=0;
	char* BW_Values[]={"4","5","6"};
	for (uint8_t bw=0 ; bw<sizeof(BW_Values) / sizeof(BW_Values[0]);bw++)
	{if (tokens[1]!=NULL &&
			strcmp(tokens[1],BW_Values[bw])==0)
	{bw_new_value=atoi(tokens[1]);
	bw_flag=1;
	sprintf((char*)cmd_buff, "BW VALUE SET TO %d SUCCESSFULLY\r\n", bw_new_value);
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
	sprintf((char*)cmd_buff,"BW VALUE IS : %d \r\nBW DEFAULT VALUE IS : %d\r\n TO CHANGE DEFAULT VALUE GO TO SYSCONF",bw_new_value,LoraValues.bw_l);
	HAL_UART_Transmit(&huart2,cmd_buff,strlen((char*)cmd_buff), 100);
	memset(cmd_buff,0,sizeof(cmd_buff));
}
//GPS Menu
void GPSMenu(char* arg){
	currentMenu=GPS_Menu;
	sprintf((char*)txBuffer,"\033[1;34;107m-----------------GPS Menu---------------\033[0m\n \r\n");
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	for (uint8_t l=0;l<sizeof(cmd_list)/sizeof(cmd_list[0]);l++)
	{if (cmd_list[l].MenuIndex==GPS_Menu)
	{sprintf((char*)txBuffer,"%s %s \r\n",cmd_list[l].Name, cmd_list[l].helper);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	}
	}
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
	sprintf((char*)txBuffer,"\033[1;32;107m--------------Sensors Menu---------------\033[0m\n \r\n");
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	for (uint8_t l=0;l<sizeof(cmd_list)/sizeof(cmd_list[0]);l++)
	{if (cmd_list[l].MenuIndex==Sensors_Menu)
	{sprintf((char*)txBuffer,"%s %s \r\n",cmd_list[l].Name, cmd_list[l].helper);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	}
	}
}

void SetSoilTemp_f(char* arg){
	uint8_t soiltemp = 0;
	if (tokens[1] != NULL && strlen(tokens[1]) < 5) {
		SoilTemp_New_Value = atof(tokens[1]);

		if (SoilTemp_New_Value >= MIN_SOIL_TEMP && sf_new_value <= MAX_SOIL_TEMP) {
			sprintf((char*)cmd_buff, "SOIL TEMPERATURE VALUE SET TO %.1f°C SUCCESSFULLY\r\n", SoilTemp_New_Value);
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
			sprintf((char*)cmd_buff, "AIR TEMPERATURE VALUE SET TO %.1f°C SUCCESSFULLY\r\n", AirTemp_New_Value);
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
void SysConfigMenu(char* arg){
	currentMenu=SysConfig_Menu;
	sprintf((char*)txBuffer,"------------------ SYSTEM CONFIGURATION MENU -----------------\r\n");
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);

	for (uint8_t l=0;l<sizeof(cmd_list)/sizeof(cmd_list[0]);l++)
	{if (cmd_list[l].MenuIndex==SysConfig_Menu)
	{sprintf((char*)txBuffer,"%s %s \r\n",cmd_list[l].Name, cmd_list[l].helper);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	}
	}

}
void Save_f(char* arg){
	//Saving Lora Values
	Old_Default_Sf=LoraValues.sf_l;
	LoraValues.sf_l=sf_new_value;
	sprintf((char*)txBuffer,"The default SF Value is now %d\r\n",LoraValues.sf_l);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	Old_Default_Bw=LoraValues.bw_l;
	LoraValues.bw_l=bw_new_value;
	sprintf((char*)txBuffer,"The default Bandwidth Value is now %d\r\n",LoraValues.bw_l);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	Old_Default_Cr=LoraValues.cr_l;
	LoraValues.cr_l=Cr_new_value;
	sprintf((char*)txBuffer,"The default CR buffer is now %d\r\n",LoraValues.cr_l);
	//Saving GPS Values
	//Saving Sensors Values

}
void Restore_f(char* arg){
	//Restoring Lora Values
	LoraValues.sf_l=Old_Default_Sf;
	sprintf((char*)txBuffer,"SF value restored to %d\r\n",LoraValues.sf_l);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	LoraValues.bw_l=Old_Default_Bw;
	sprintf((char*)txBuffer,"BW value restored to %d\r\n",LoraValues.bw_l);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	LoraValues.cr_l=Old_Default_Cr;
	sprintf((char*)txBuffer,"CR value restored to %d\r\n",LoraValues.cr_l);
	HAL_UART_Transmit(&huart2, txBuffer, strlen((char*)txBuffer), 100);
	//Restoring GPS Values
	//Restoring Sensors Values
}

