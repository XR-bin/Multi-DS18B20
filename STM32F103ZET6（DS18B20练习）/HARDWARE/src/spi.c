#include "sys.h"
#include "stdio.h"

/******************************************************************
*函数功能  ：对SPI1通信要用的GPIO口进行初始化和对SPI1的寄存器进行设置
*函数名    ：SPI2_Init
*函数参数  ：void
*函数返回值：void
*描述      ：
*           因为spi2只有这三个引脚，所以要选择复用
*           PB13   SPI2_SCK   时钟线  复用推挽
*           PB14   SPI2_MISO  主输入  复用推挽/推挽模式也可以视为输入模式
*           PB15   SPI2_MOSI  主输出  复用推挽
*******************************************************************/
void SPI2_Init(void)
{
  //GPIOx时钟使能
  RCC->APB2ENR |= 1<<3;
  //端口配置寄存器
  GPIOB->CRH &= ~(0xfff00000);
  GPIOB->CRH |= 0xbbb00000;
  //复用重映射寄存器  //无
  //端口输出寄存器输出高(上拉)
  GPIOB->ODR|=0X7<<13; 
  
  /*SPI2寄存器初始化设置*/
  //SPI2时钟使能
  RCC->APB1ENR |= 1<<14;
  //SPI2控制寄存器1
  SPI2->CR1 &= ~(0xffff);
  SPI2->CR1 |= (1<<9);
  SPI2->CR1 |= (1<<8);
  SPI2->CR1 |= (1<<3);
  SPI2->CR1 |= (1<<2);  
  //SPI2控制寄存器2
  SPI2->CR2 &= ~(0xff);
  //SPI_I2S配置寄存器
  SPI2->I2SCFGR &= ~(0xfff);
  //使能SPI2
  SPI2->CR1|=1<<6;
}

/******************************************************************
*函数功能  ：SPI1接收/发送一个字节数据（8位）
*函数名    ：SPI1_RS_Byte
*函数参数  ：u8 data
*函数返回值：u8
*描述      ：
*           PB3   SPI1_SCK   时钟线
*           PB4   SPI1_MISO  主输入
*           PB5   SPI1_MOSI  主输出
*******************************************************************/
u8 SPI2_RS_Byte(u8 data)
{
  //判断发送缓存区是否为空
  while(!(SPI2->SR & (1<<1)));
  SPI2->DR = data;
  //判断接收缓存区是否为空
  while(!(SPI2->SR & (1<<0)));
  data = SPI2->DR;
  
  return data;
}






