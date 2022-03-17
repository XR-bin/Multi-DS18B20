#ifndef _DS18B20_H
#define _DS18B20_H

	#include "sys.h"
	
	//DQ宏定义
	#define DQ_H     GPIOG->ODR |= (1<<11)   //DQ线拉高
	#define DQ_L     GPIOG->ODR &= ~(1<<11)  //DQ线拉低
  #define DQ_IN   {GPIOG->CRH&=0XFFFF0FFF;GPIOG->CRH|=8<<12;}
	#define DQ_OUT  {GPIOG->CRH&=0XFFFF0FFF;GPIOG->CRH|=3<<12;}
	#define DQ_READ  GPIOG->IDR & (1<<11)    //读数据
	#define DQ_ACK   GPIOG->IDR & (1<<11)    //判断应答
	
	//最多能挂载DS18B20的个数
	#define MAXNUM     4
	
	void DS18B20_Init(void);
	void DS18B20_RST(void);
	u8 DS18B20_ACK(void);
	void DS18B20_Write_Byte(u8 data);
	u8 DS18B20_Read_Byte(void);
	void DS18B20_Read_Temperature(void);
	void DS18B20_SingleAddr(void);
	
	u8 DS18B20_Read2Bit(void);
	void DS18B20_WriteBit(u8 bit);
	void DS18B20_MultiAddr_Test(void);
	void DS18B20_MultiAddr(u8 *num,u8 (*buff)[8]);

#endif




