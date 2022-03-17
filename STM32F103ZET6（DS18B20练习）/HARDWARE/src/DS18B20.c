#include "sys.h"
#include "DS18B20.h"
#include "delay.h"
#include "stdio.h"

/*****************************************************
*函数功能  ：对DS18B20对应的GPIO口进行初始化设置
*函数名    ：DS18B20_Init
*函数参数  ：void
*函数返回值：void
*描述      ：
*            DQ  ---  PG11
********************************************************/
void DS18B20_Init(void)
{
	//GPIOx时能时钟
	RCC->APB2ENR |= 1<<8;   	 	//使能PORTG口时钟
	//端口配置寄存器
	//推挽输出   100MHz
	GPIOG->CRH &= ~(0xf<<4*(11-8));
  GPIOG->CRH |= (0x3<<4*(11-8));
	
	//端口数据输出寄存器
	//DQ线拉低(单总线协议中单总线低电平时为空闲状态)
	GPIOG->ODR &= ~(1<<11);    //空闲状态
	
	DS18B20_RST();
}


/*****************************************************
*函数功能  ：对DS18B20的复位信号
*函数名    ：DS18B20_RST
*函数参数  ：void
*函数返回值：void
*描述      ：
*            DQ  ---  PG11
********************************************************/
void DS18B20_RST(void)
{
	//DQ进入输出模式
	DQ_OUT;
	//将DQ电平拉低
	DQ_L;
	//持续480~960us
	delay_us(700);
	//释放DQ，恢复空闲
	DQ_H;
	delay_us(15);
	//等待应答
  DS18B20_ACK();
}

/*****************************************************
*函数功能  ：DS18B20的应答信号
*函数名    ：DS18B20_ACK
*函数参数  ：void
*函数返回值：u8
*描述      ：
*            DQ  ---  PG11
********************************************************/
u8 DS18B20_ACK(void)
{
	u8 ACK = 0;
	u8 temp = 60;
	
	//DQ进入输入模式
	DQ_IN;
	//DS18B20会等待15~60us后给单片机应答
	while(DQ_ACK)
	{
		temp--;
		if(temp == 60) return ACK;
		delay_us(1);
	}
	temp = 240;
	//DS18B20会将DQ拉低60~240us来作为应答
	while(temp--)
	{
		if(DQ_ACK)
		{
			ACK = 1;
			break;
		}
		delay_us(1);
	}
	
	return ACK;
}

/*****************************************************
*函数功能  ：对DS18B20进行写一个字节数据
*函数名    ：DS18B20_Write_Byte
*函数参数  ：u8 data
*函数返回值：void
*描述      ：
*            DQ  ---  PG11
********************************************************/
void DS18B20_Write_Byte(u8 data)
{
	u8 i;
	//DQ进入输出模式
	DQ_OUT;
	for(i=0;i<8;i++)
	{
		//拉低DQ，持续10~15us
		DQ_L;
		delay_us(12);
		//判断当前要发送的数据最低为是 1 还是 0
		if(data & 0x01) DQ_H;
		else DQ_L;
		//保持当前DQ电平持续45us
		delay_us(55);
		//DQ释放，持续1us
		DQ_H;
		delay_us(1);
		//要发送的数据右移一位
		data >>= 1;
	}
}

/*****************************************************
*函数功能  ：对DS18B20进行读1个bit(位)数据
*函数名    ：DS18B20_Read_Bit
*函数参数  ：void
*函数返回值：u8 
*描述      ：
*            DQ  ---  PG11
********************************************************/
u8 DS18B20_Read_Bit(void) 	 
{
    u8 data;
		DQ_OUT;	//SET PG11 OUTPUT
    DQ_L; 
	delay_us(2);
    DQ_H; 
	DQ_IN;	//SET PG11 INPUT
	delay_us(12);
	if(DQ_READ)data=1;
    else data=0;	 
    delay_us(50);           
    return data;
}

/*****************************************************
*函数功能  ：对DS18B20进行读一个字节数据
*函数名    ：DS18B20_Read_Byte
*函数参数  ：void
*函数返回值：u8 
*描述      ：
*            DQ  ---  PG11
********************************************************/
u8 DS18B20_Read_Byte(void)
{
	u8 i;
	u8 data = 0;
	
	for(i=0;i<8;i++)
	{
		//DQ进入输出模式
		DQ_OUT;
		//拉低DQ持续1us
		DQ_L;
		delay_us(1);
		//拉高DQ持续13us
		DQ_H;
		delay_us(12);
		DQ_IN;
		//读取数据
		if(DQ_READ) data |= (1<<i);
		//延时45us
		delay_us(50);
		//DQ释放，持续1us
		DQ_H;
		delay_us(2);
	}
	return data;
}


/*****************************************************
*函数功能  ：获得DS18B20侧脸的温度
*函数名    ：DS18B20_Read_Temperature
*函数参数  ：void
*函数返回值：void
*描述      ：
*            DQ  ---  PG11
********************************************************/
void DS18B20_Read_Temperature(void)
{
	u8 data_l ,data_h;
	short data;
	
	DS18B20_RST();
	DS18B20_Write_Byte(0xcc);
	DS18B20_Write_Byte(0x44);
	DS18B20_RST();
	DS18B20_Write_Byte(0xcc);
	DS18B20_Write_Byte(0xbe);
	data_l = DS18B20_Read_Byte();
	data_h = DS18B20_Read_Byte();
	
	if(data_h > 0x07)
	{
		data_l = ~data_l;
		data_h = ~data_h;
	}
	data = (data_h<<8)+data_l;
	data *= 0.625;
	
	printf("%d\r\n",data);

}

/*****************************************************
*函数功能  ：获得DS18B20的系列号、器件地址、CRC校验码
*函数名    ：DS18B20_SingleAddr
*函数参数  ：void
*函数返回值：void
*描述      ：
*            DQ  ---  PG11
********************************************************/
void DS18B20_SingleAddr(void)
{
	u8 data[8];
	u8 i;
	DS18B20_RST();
	DS18B20_Write_Byte(0x33);
	for(i=0;i<8;i++)
	data[i] = DS18B20_Read_Byte();
	
	printf("单线系列编码：%d\r\n",data[0]);
	printf("地址：");
	for(i=1;i<7;i++)
	printf("%d ",data[i]);
	printf("\r\n");
	printf("CRC校验码：%d\r\n",data[7]);
}

/*****************************************************
*函数功能  ：对DS18B20进行读两个bit(位)数据
*函数名    ：DS18B20_Read2Bit
*函数参数  ：void
*函数返回值：u8 
*描述      ：
*            DQ  ---  PG11
********************************************************/
u8 DS18B20_Read2Bit(void)  
{  
	u8 i;
	u8 data = 0;
	
	for(i=0;i<2;i++)
	{
		//左移一位
		data <<= 1;
		//DQ进入输出模式
		DQ_OUT;
		//拉低DQ持续1us
		DQ_L;
		delay_us(1);
		//拉高DQ持续13us
		DQ_H;
		delay_us(12);
		DQ_IN;
		//读取数据
		if(DQ_READ) data |= 0x01;
		//延时45us
		delay_us(50);
		//DQ释放，持续1us
		DQ_H;
		delay_us(2);
	}
	return data;
}

/*****************************************************
*函数功能  ：对DS18B20进行写一个bit(位)数据
*函数名    ：DS18B20_WriteBit
*函数参数  ：u8 bit
*函数返回值：void
*描述      ：
*            DQ  ---  PG11
********************************************************/
void DS18B20_WriteBit(u8 bit)
{
	//DQ进入输出模式
	DQ_OUT;
	//拉低DQ，持续10~15us
	DQ_L;
	delay_us(12);
	//判断当前要发送的数据最低为是 1 还是 0
	if(bit & 0x01) DQ_H;
	else DQ_L;
	//保持当前DQ电平持续45us
	delay_us(55);
	//DQ释放，持续1us
	DQ_H;
	delay_us(1);
}

/****************************************************************************************
*函数功能  ：搜索一条总线上DS18B20的个数，并获取它们的ROM数据(系列号、器件地址、CRC校验码)
*函数名    ：DS18B20_MultiAddr_Test  (实验版，正式函数是下一个函数) 
*函数参数  ：void
*函数返回值：void
*描述      ：
*            这个函数只是用来检验理思路，只测单个，看看0xf0指令和0x33指令
*        得到的ROM数据是否一致。
*            注意：此函数思路比较简单，可以先看此函数了解思路再看下一个正式
*        版函数，正式版的函数思路与此函数有所类似。
*****************************************************************************************/
void DS18B20_MultiAddr_Test(void)
{
	u8 i,j;
	u8 data;
	u8 buff[8];
	u8 bit8 = 0;
	
	DS18B20_RST();								//复位DS18B20总线  
  DS18B20_Write_Byte(0xf0);	    //搜索ROM 
	for(i=0;i<8;i++)
	{
		for(j=0;j<8;j++)
		{
			bit8 >>= 1;
			data = DS18B20_Read2Bit();
			data &= 0x3;
			if(data == 0x01)  //此位到位0
			{
				DS18B20_WriteBit(0);  //继续链接此位为0的地址
			}
			else if(data == 0x02)
			{
				bit8 |= 0x80;
				DS18B20_WriteBit(1);  //继续链接此位为1的地址
			}
		}
		buff[i] = bit8;
		bit8 = 0;
	}
	
	printf("单线系列编码：%d\r\n",buff[0]);
	printf("地址：");
	for(i=1;i<7;i++)
	printf("%d ",buff[i]);
	printf("\r\n");
	printf("CRC校验码：%d\r\n",buff[7]);
}

/****************************************************************************************
*函数功能  ：搜索一条总线上DS18B20的个数，并获取它们的ROM数据(系列号、器件地址、CRC校验码)
*函数名    ：DS18B20_MultiAddr  (正式版ROM搜索函数函数) 
*函数参数  ：u8 *num,u8 (*buff)[8]
*函数返回值：void
*描述      ：
*****************************************************************************************/
void DS18B20_MultiAddr(u8 *num,u8 (*buff)[8])
{
	u8 i,j;
	u8 data;             //用来接收DS18B20发来的两位数据
	u8 bit8 = 0;         //用于临时装第二层for循环获取到的8个bit的ROM数据(一个字节数据)
	u8 temp[64] = {0};   //接收当前搜索的DS18B20的ROM数据，给下一次搜索提供参照(主要用处体现在下一次搜索冲突位的筛选)
	u8 conflict;         //发生冲突的位
	u8 sign[64] = {0};   //用于标记冲突位
	u8 s = 0;            //用于标记最高位的冲突位(ROM数据一共有64个数据位,末尾的8位数据是CRC，前面一样，CRC基本一样，所以冲突位基本不会出现在末尾的后8位)
	*num = 0;            //初始化这个num，不然如果外面没有定义时赋值0的话会出现错误现在
	do
	{
		DS18B20_RST();								//复位DS18B20总线  
		DS18B20_Write_Byte(0xf0);	    //发送搜索ROM 
		for(i=0;i<8;i++)
		{
			for(j=0;j<8;j++)
			{
				//因为搜索都是从低位开始的
				bit8 >>= 1; 
				//读取两位数据
				data = DS18B20_Read2Bit();
				//虽然这一步有点多余，但还是形式上写一下
				data &= 0x3;
				/**************************************************************************
				*data的值如果是：
				*   01 ：还受ROM搜索指令控制的所有DS18B20的该位都是0
				*   10 ：还受ROM搜索指令控制的所有DS18B20的该位都是1
				*   00 ：还受ROM搜索指令控制的所有DS18B20的该位有的是0有的是1，即该位位冲突位
				*   11 ：总线上没有DS18B20
				***************************************************************************/
				if(data == 0x01)           //还受ROM搜索指令控制的所有DS18B20的该位都是0
				{
					temp[i*8+j] = 0;         //记录该位为0
					DS18B20_WriteBit(0);     //筛选该位是0的DS18B20(如果该位不是0就不再受ROM搜索指令控制)
				}
				else if(data == 0x02)      //还受ROM搜索指令控制的所有DS18B20的该位都是1
				{
					temp[i*8+j] = 1;         //记录该位为1
					bit8 |= 0x80;            //记录该位为1
					DS18B20_WriteBit(1);     //筛选该位是1的DS18B20(如果该位不是1就不再受ROM搜索指令控制)
				}
				else if(data == 0x00)      //还受ROM搜索指令控制的所有DS18B20的该位有的是0有的是1，即该位位冲突位 
				{
					
					conflict = i*8 + j +1;    //第几位发生冲突(ROM数据一共有64个数据位)
					
					//如果s = 0 说明：之前没出现过冲突位
					if(conflict > sign[s])
					{
						//记录冲突位
						s++;                 //第s次冲突位
						sign[s] = conflict;  //记录第几位发生冲突
						//我们先把0的搜索出来
						//筛选该位为0的DS18B20(不是0的就不再受ROM搜索指令控制)
						DS18B20_WriteBit(0);
						temp[i*8+j] = 0;     
					}
					//进行下面判断时s必然不等于0了，而且已经是第2个及以上的ROM搜索了
					//如果该冲突位不是最高冲突位(或者说不是数值最大的)就进行if里的程序
					else if(conflict > sign[s])
					{
						//因为不是最高的冲突位，所以该位的数值依然和上一次搜索到的ROM数据该位的值一样
						bit8 = bit8 | ((temp[(i*8+j)]&0x01)<<7);
						//和上一次ROM搜索到该位时的筛选一样
						DS18B20_WriteBit (temp[i*8+j]);
					}
					//这里的判断也一样，进行下面判断时s必然不等于0了，而且已经是第2个及以上的ROM搜索了
					//如果该冲突位是最高冲突位(或者说是数值最大的)就进行if里的程序
					else if(conflict == sign[s])
					{
						//因为我们总是先把最高冲突为为0的DS18B20的ROM数据读出来，
						//所以这次该是最高冲突为为1的DS18B20了
						//记录该位这次的DS18B20的ROM数据
						bit8 = bit8 | 0x80; 
						temp[i*8+j]=1;
						//筛选该位为1的DS18B20(不是1的就不再受ROM搜索指令控制)
						DS18B20_WriteBit (1);  
						//解决了这个最高冲突位后最高冲突位就要降级了
						sign[s] = 0;
						s = s - 1;
					}
				}
				else
				{
					*num = 0;
					printf("没有DS18B20\r\n");
					return ;
				}
			}
			//记录每个DS18B20的ROM数据
			buff[*num][i] = bit8;
			bit8 = 0;
		}
		//记录个数
		(*num)++;
	}while((sign[s]!=0) && (*num<=MAXNUM));
	
}










