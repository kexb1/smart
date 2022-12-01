#include "sgp30.h"
#include "delay.h"

u32 CO2Data,TVOCData;//定义CO2浓度变量与TVOC浓度变量
u32 sgp30_dat;

void SGP30_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(SGP30_SCL_GPIO_CLK | SGP30_SDA_GPIO_SDA, ENABLE);

  GPIO_InitStructure.GPIO_Pin = SGP30_SCL_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(SGP30_SCL_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = SGP30_SDA_GPIO_PIN;
  GPIO_Init(SGP30_SDA_GPIO_PORT, &GPIO_InitStructure);
}


void SDA_OUT(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = SGP30_SDA_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(SGP30_SDA_GPIO_PORT, &GPIO_InitStructure);
}

void SDA_IN(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = SGP30_SDA_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(SGP30_SDA_GPIO_PORT, &GPIO_InitStructure);
}

//产生IIC起始信号
void SGP30_IIC_Start(void)
{
  SDA_OUT();
  SGP30_SDA = 1;
  SGP30_SCL = 1;
  Delay_us(20);

  SGP30_SDA = 0;	//START:when CLK is high,DATA change form high to low
  Delay_us(20);
  SGP30_SCL = 0; 	//钳住I2C总线，准备发送或接收数据
}

//产生IIC停止信号
void SGP30_IIC_Stop(void)
{
  SDA_OUT();
  SGP30_SCL = 0;
  SGP30_SDA = 0;	//STOP:when CLK is high DATA change form low to high
  Delay_us(20);
  SGP30_SCL = 1;
  SGP30_SDA = 1;	//发送I2C总线结束信号
  Delay_us(20);
}

//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 SGP30_IIC_Wait_Ack(void)
{
  u8 ucErrTime = 0;
  SDA_IN();
  SGP30_SDA = 1;
  Delay_us(10);
  SGP30_SCL = 1;
  Delay_us(10);
  while(SGP30_SDA_READ())
  {
    ucErrTime++;
    if(ucErrTime > 250)
    {
      SGP30_IIC_Stop();
      return 1;
    }
  }
  SGP30_SCL = 0;  		//时钟输出0
  return 0;
}

//产生ACK应答
void SGP30_IIC_Ack(void)
{
  SGP30_SCL = 0;
  SDA_OUT();
  SGP30_SDA = 0;
  Delay_us(20);
  SGP30_SCL = 1;
  Delay_us(20);
  SGP30_SCL = 0;
}

//不产生ACK应答
void SGP30_IIC_NAck(void)
{
  SGP30_SCL = 0;
  SDA_OUT();
  SGP30_SDA = 1;
  Delay_us(20);
  SGP30_SCL = 1;
  Delay_us(20);
  SGP30_SCL = 0;
}

//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答
void SGP30_IIC_Send_Byte(u8 txd)
{
  u8 t;
  SDA_OUT();
  SGP30_SCL = 0; 	    	//拉低时钟开始数据传输
  for(t = 0; t < 8; t++)
  {
    if((txd & 0x80) >> 7)
      SGP30_SDA = 1;
    else
      SGP30_SDA = 0;
    txd <<= 1;
    Delay_us(20);
    SGP30_SCL = 1;
    Delay_us(20);
    SGP30_SCL = 0;
    Delay_us(20);
  }
  Delay_us(20);

}

//读1个字节，ack=1时，发送ACK，ack=0，发送nACK
u16 SGP30_IIC_Read_Byte(u8 ack)
{
  u8 i;
  u16 receive = 0;
  SDA_IN();
  for(i = 0; i < 8; i++ )
  {
    SGP30_SCL = 0;
    Delay_us(20);
    SGP30_SCL = 1;
    receive <<= 1;
    if(SGP30_SDA_READ())
      receive++;
    Delay_us(20);
  }
  if (!ack)
    SGP30_IIC_NAck();//发送nACK
  else
    SGP30_IIC_Ack(); //发送ACK
  return receive;
}


//初始化IIC接口
void SGP30_Init(void)
{
  SGP30_GPIO_Init();
  SGP30_Write(0x20, 0x03);

	Delay_ms(100);
	SGP30_Write(0x20,0x08);
	sgp30_dat = SGP30_Read();//读取SGP30的值
	CO2Data = (sgp30_dat & 0xffff0000) >> 16;
	TVOCData = sgp30_dat & 0x0000ffff;	
	//SGP30模块开机需要一定时间初始化，在初始化阶段读取的CO2浓度为400ppm，TVOC为0ppd且恒定不变，因此上电后每隔一段时间读取一次
	//SGP30模块的值，如果CO2浓度为400ppm，TVOC为0ppd，发送“正在检测中...”，直到SGP30模块初始化完成。
	//初始化完成后刚开始读出数据会波动比较大，属于正常现象，一段时间后会逐渐趋于稳定。
	//气体类传感器比较容易受环境影响，测量数据出现波动是正常的，可自行添加滤波函数。
	while(CO2Data == 400 && TVOCData == 0)
	{
		SGP30_Write(0x20,0x08);
		sgp30_dat = SGP30_Read();//读取SGP30的值
		CO2Data = (sgp30_dat & 0xffff0000) >> 16;//取出CO2浓度值
		TVOCData = sgp30_dat & 0x0000ffff;			 //取出TVOC值
		Delay_ms(500);
	}
//	SGP30_ad_write(0x20,0x61);
//	SGP30_ad_write(0x01,0x00);
}


void SGP30_Write(u8 a, u8 b)
{
  SGP30_IIC_Start();
  SGP30_IIC_Send_Byte(SGP30_write); //发送器件地址+写指令
  SGP30_IIC_Wait_Ack();
  SGP30_IIC_Send_Byte(a);		//发送控制字节
  SGP30_IIC_Wait_Ack();
  SGP30_IIC_Send_Byte(b);
  SGP30_IIC_Wait_Ack();
  SGP30_IIC_Stop();
  Delay_ms(100);
}

u32 SGP30_Read(void)
{
  u32 dat;
  u8 crc;
  SGP30_IIC_Start();
  SGP30_IIC_Send_Byte(SGP30_read); //发送器件地址+读指令
  SGP30_IIC_Wait_Ack();
  dat = SGP30_IIC_Read_Byte(1);
  dat <<= 8;
  dat += SGP30_IIC_Read_Byte(1);
  crc = SGP30_IIC_Read_Byte(1); //crc数据，舍去
  crc = crc;  //为了不让出现编译警告
  dat <<= 8;
  dat += SGP30_IIC_Read_Byte(1);
  dat <<= 8;
  dat += SGP30_IIC_Read_Byte(0);
  SGP30_IIC_Stop();
  return(dat);
}
void SGP30_Data_Get( void )
{
	SGP30_Write(0x20,0x08);
	sgp30_dat = SGP30_Read();//读取SGP30的值
	CO2Data = (sgp30_dat & 0xffff0000) >> 16;//取出CO2浓度值
	TVOCData = sgp30_dat & 0x0000ffff;       //取出TVOC值
}



