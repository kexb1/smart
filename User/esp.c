#include "stm32f10x.h"                  // Device header
#include "MyUSART.h"
#include <stdio.h>
#include <string.h>
#include "Delay.h"
#include "OLED.H"
#include <stdbool.h>
extern int Temperature;
extern int Shidu;
extern char Switch1;
extern char Switch2;
extern char RECS[250];
const char* WIFI ="OnePlus";
const char* WIFIASSWORD="12345678";
const char* ClintID="hxyphvk6Zgo.home|securemode=2\\,signmethod=hmacsha256\\,timestamp=1669522076858|";
const char* username="home&hxyphvk6Zgo";
const char* passwd="e463cda1a507d3ebac338d4cd1a2076c7c830476d68b87c6b82b06349002935b";
const char* Url="iot-06z00gy7jtp0cg7.mqtt.iothub.aliyuncs.com";
const char* pubtopic="/sys/hxyphvk6Zgo/home/thing/event/property/post";
const char* subtopic="/sys/hxyphvk6Zgo/home/thing/event/property/post_reply";
const char* func1="temperature";
const char* func2="Humidity";
const char* func3="PowerSwitch_1";
const char* func4="PowerSwitch_2";
extern u32 CO2Data;
int fputc(int ch,FILE *f )   //printf重定向  
{
	USART_SendData(USART1,(uint8_t)ch);
	while(USART_GetFlagStatus (USART1,USART_FLAG_TC) == RESET);
	return ch;
}
char esp_Init(void)
{
//	char num = 0 ,RECS2[200];
	memset(RECS,0,sizeof(RECS));
	printf("AT+RST\r\n");  //重启
	Delay_ms(2000);
	
	memset(RECS,0,sizeof(RECS));
	printf("ATE0\r\n");    //关闭回显
	Delay_ms(10);
	if(strcmp(RECS,"OK"))
		return 1;
	
	printf("AT+CWMODE=1\r\n"); //Station模式
	Delay_ms(1000);
	if(strcmp(RECS,"OK"))
		return 2;
	
	memset(RECS,0,sizeof(RECS));
	printf("AT+CWJAP=\"%s\",\"%s\"\r\n",WIFI,WIFIASSWORD); //连接热点
	Delay_ms(2000);
//	while(RECS[num++] != '\0');//连接WIFI时会受到几条字符串，只有最后才是OK
//	RECS2[0] = RECS[num-2];RECS2[1] = RECS[num-1];RECS2[2] = RECS[num];
	if(strcmp(RECS,"OK"))
		return 3;
	
	memset(RECS,0,sizeof(RECS));
	printf("AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",ClintID,username,passwd);//用户信息配置
	Delay_ms(10);
	if(strcmp(RECS,"OK"))
		return 4;
	
	memset(RECS,0,sizeof(RECS));
	printf("AT+MQTTCONN=0,\"%s\",1883,1\r\n",Url); //连接服务器
	Delay_ms(1000);
	if(strcmp(RECS,"OK"))
		return 5;
	
	printf("AT+MQTTSUB=0,\"%s\",1\r\n",subtopic); //订阅消息
	Delay_ms(500);
	if(strcmp(RECS,"OK"))
		return 5;
	memset(RECS,0,sizeof(RECS));
	return 0;
}
//功能：esp发送消息
//参数：无
//返回值：0：发送成功；1：发送失败
char Esp_PUB(void)
{
	memset(RECS,0,sizeof(RECS));
	printf("AT+MQTTPUB=0,\"%s\",\"{\\\"method\\\":\\\"thing.event.property.post\\\"\\,\\\"params\\\":{\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d}}\",0,0\r\n",pubtopic,func1,Temperature,func2,Shidu,func3,Switch1,func4,Switch2);
	//while(RECS[0]);//等待ESP返回数据
	Delay_ms(200);//延时等待数据接收完成
	if(strcmp(RECS,"ERROR")==0)
		return 1;
	return 0;
}
void CommandAnalyse(void)
{
	if(strncmp(RECS,"+MQTTSUBRECV:",13)==0)
	{
		uint8_t i=0;
		while(RECS[i++] != '\0')             
		{
			
			if(strncmp((RECS+i),func3,13)==0)
			{
				while(RECS[i++] != ':');  
				Switch1=RECS[i] ;
				Switch1 -= 48 ;
			}
			if(strncmp((RECS+i),func4,13)==0)
			{
				while(RECS[i++] != ':');
				Switch2=RECS[i] ;
				Switch2 -= 48 ;
			}
		}
	}
}









