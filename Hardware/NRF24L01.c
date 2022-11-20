#include "stm32f10x.h"                  // Device header
#include "NRF24L01.H"
#include "MySPI.H"
#include "Delay.h"
#include "OLED.H"
unsigned char ADDR[TX_ADR_WIDTH]={0x1A,0x2A,0x2E,0x2B,0x99};

#define CE GPIO_Pin_8
#define CSN GPIO_Pin_11
#define IRQ GPIO_Pin_12
void NRF_Init(void)
{
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = CE | CSN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = IRQ;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA,CSN);			//CSN=1
	GPIO_ResetBits(GPIOA,CE);     //CE=0
	MySPI_Init();
}

/******************************
功能：单字节地写寄存器值
参数：reg 要写入的寄存器映射地址；value 要写入的值
返回值：状态寄存器的值
********************************/
unsigned char NRF_WriteReg(unsigned char reg,unsigned char value)
{
	
	unsigned char status;
	GPIO_ResetBits(GPIOA,CSN);                 //片选信号CSN=0
	status=SPI_RW(reg);
	SPI_RW(value);
	GPIO_SetBits(GPIOA,CSN);//CSN=1
	
	return status;
}
/******************************
功能：多字节地写寄存器值
参数：reg 要写入的寄存器映射地址；buf 要写入的值数组的首地址；Length 字符串长度
返回值：状态寄存器的值
********************************/
unsigned char NRF_WriteBuff(unsigned char reg,unsigned char *buf ,unsigned char Length)
{
	unsigned char status,i;
	GPIO_ResetBits(GPIOA,CSN); //CSN=0
	status = SPI_RW(reg);
	for(i=0;i<Length;i++)
	{
		SPI_RW(buf[i]);
	}
	GPIO_SetBits(GPIOA,CSN);//CSN=1
	
	return status;
}
/******************************
功能：单字节地读取状态寄存器的值
参数：reg 要读取的寄存器映射地址；
返回值：状态寄存器的值
********************************/
unsigned char NRF_ReadReg(unsigned char reg)
{
	unsigned char value;
	GPIO_ResetBits(GPIOA,CSN); //CSN=0
	SPI_RW(reg);
	value = SPI_RW(0);
	GPIO_SetBits(GPIOA,CSN); //CSN=1
	return value;
}
/******************************
功能：多字节地读取状态寄存器的值
参数：reg 要读取的寄存器映射地址；buf 接受缓冲区首地址 ；Length 接受数据地字节数
返回值：状态寄存器的值
********************************/
unsigned char NRF_ReadBuff(unsigned char reg,unsigned char * buf,unsigned char Length)
{
	unsigned char status,i;
	GPIO_ResetBits(GPIOA,CSN); //CSN=0
	
	status = SPI_RW(reg);
	for(i=0;i<Length;i++)
		buf[i] = SPI_RW(0);
	
	GPIO_SetBits(GPIOA,CSN); //CSN=1
	
	return status;
}

void NRF_RXMode(void)
{
	GPIO_ResetBits(GPIOA,CE);//CE=0
	NRF_WriteBuff(NRF_WRITE_REG+ RX_ADDR_P0,ADDR,TX_ADR_WIDTH); //写命令，数据管道0接收地址
	NRF_WriteReg(NRF_WRITE_REG+EN_AA,0x01); //  使能数据管道0自动确认（应答）
	NRF_WriteReg(NRF_WRITE_REG+EN_RXADDR,0x01);//使能接受数据管道0地址
	NRF_WriteReg(NRF_WRITE_REG+RF_CH,0x30);//设置信道
	NRF_WriteReg(NRF_WRITE_REG+RX_PW_P0,TX_PLOAD_WIDTH);//配置接受字节数
	NRF_WriteReg(NRF_WRITE_REG+RF_SETUP,0x0f);//发射配置
	NRF_WriteReg(NRF_WRITE_REG+CONFIG,0x0f);//寄存器配置
	NRF_WriteReg(NRF_WRITE_REG+STATUS,0xff);//将所有状态寄存器清零
	GPIO_SetBits(GPIOA,CE);//芯片使能
}

void NRF_TXMode(void)
{
	GPIO_ResetBits(GPIOA,CE);//CE=0
	
	NRF_WriteBuff(NRF_WRITE_REG + TX_ADDR, ADDR, TX_ADR_WIDTH);    //发射方的发射地址
	NRF_WriteBuff(NRF_WRITE_REG + RX_ADDR_P0, ADDR, TX_ADR_WIDTH); //为了接收ACK
	NRF_WriteReg(NRF_WRITE_REG+EN_AA,0x01); //  使能数据管道0自动确认（应答）
	NRF_WriteReg(NRF_WRITE_REG+EN_RXADDR,0x01);//使能接受数据管道0地址
	NRF_WriteReg(NRF_WRITE_REG+SETUP_RETR,0x0a);//自动重发配置
	NRF_WriteReg(NRF_WRITE_REG+RF_CH,0x30);//设置信道
	NRF_WriteReg(NRF_WRITE_REG+RF_SETUP,0x0f);//发射配置
	NRF_WriteReg(NRF_WRITE_REG+CONFIG,0x0e);//寄存器配置
	
	GPIO_SetBits(GPIOA,CE);//CE=1
	Delay_us(10);//等待进入发射模式
}

unsigned char NRF_RXPacket(unsigned char * rxbuf)
{
	unsigned char state;
	state = NRF_ReadReg(STATUS);
	NRF_WriteReg(NRF_WRITE_REG+STATUS,state);
	if(state & RX_DR)
	{
		NRF_ReadBuff(RD_RX_PLOAD,rxbuf,TX_PLOAD_WIDTH);
		NRF_WriteReg(FLUSH_RX,0xff);
		return 0;
	}
	return 1;
}

unsigned char NRF_TXPacket(unsigned char * txbuf)
{
	unsigned char state;
	
	GPIO_ResetBits(GPIOA,CE);Delay_us(2);
	NRF_WriteBuff(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);
	GPIO_SetBits(GPIOA,CE);
	
	while(GPIO_ReadInputDataBit(GPIOA,IRQ)==SET);
	state = NRF_ReadReg(STATUS);
	
	if(state & MAX_RT)//达到最大重发次数,失败
	{
		NRF_WriteReg(FLUSH_TX,0xff);
		return MAX_RT;
	}
	if(state & TX_DS)
		{return TX_DS;} //发送完成
		
	return  0xff;//发送失败
}
