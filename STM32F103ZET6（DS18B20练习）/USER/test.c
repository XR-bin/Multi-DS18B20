#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "stdio.h" 
#include "nvic.h"
#include "string.h"
#include "DS18B20.h"

/********************************************************************************
*********************************************************************************
*实验项目     : 一条总线上多个DS18B20的地址搜索
*实验所用材料 : STM32F103(正点原子的精英板)  多个DS18B20  
*实验现象     : 多个DS18B20只需连接同一个IO口，当开发板复位后，STM32F103就可以通过这
*               个IO读取所用DS18B20的序列号和器件地址，及其这个IO口上连接的DS18B20个
*               数，并将其通过串口打印出来。
*
*注意：如果要用F407开发板来看实验效果，只需移植DS18B20.c文件和DS18B20.h文件，并修改
*      DS18B20.c的DS18B20_Init函数即可。
*
*在看本实验是先自己了解一次DS18B20，先上网查找一下多个DS18B20地址搜索规则，不然很难理解
*本实验思路
*
*本实验作者：许榕彬
*********************************************************************************
*********************************************************************************/



int main(void)
{	 
  u8 num = 0;
	u8 i,n;
	
	u8 buff[MAXNUM][8];
	Stm32_Clock_Init(9);//系统时钟设置
  NVIC_SetPriorityGrouping(5);  //7 - 抢占优先级二进制位数
	SysTick_Init();	 	//延时初始化
  USART1_Init(115200);
	DS18B20_Init();
	DS18B20_SingleAddr();
	DS18B20_MultiAddr(&num,buff);
	printf("num:%d\r\n",num);
	for(n=0;n<num;n++)
	{
		printf("单线系列编码：%d\r\n",buff[n][0]);
		printf("地址：");
		for(i=1;i<7;i++)
		printf("%d ",buff[n][i]);
		printf("\r\n");
		printf("CRC校验码：%d\r\n",buff[n][7]);
	}
	while(1)
	{
//		DS18B20_Read_Temperature();
//		delay_ms(1000);
	}
}




