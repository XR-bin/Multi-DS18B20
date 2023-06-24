#include "DS18B20.h"
#include "delay.h"



/**
******************************************************************************
* @file      ：.\Drivers\BSP\src\DS18B20.c
*              .\Drivers\BSP\inc\DS18B20.h
* @author    ：XRbin
* @version   ：V1.0
* @date      ：2023-06-16
* @brief     ：DS18B20驱动配置代码
******************************************************************************
* @attention
*   我的GitHub   ：https://github.com/XR-bin
*   我的gitee    ：https://gitee.com/xrbin
*   我的leetcode ：https://leetcode.cn/u/xrbin/
******************************************************************************
*/



/**********************************************************
* @funcName ：DS18B20_Init
* @brief    ：对DS18B20对应的GPIO口进行初始化设置
* @param    ：void
* @retval   ：void
* @details  ：
*            DQ  ---  PG11
* @fn       ：
************************************************************/
void DS18B20_Init(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    DQ_GPIO_CLK_ENABLE();                                 /* DS18B20引脚相关时钟使能 */
    
    /* DQ引脚配置 */
    gpio_init_struct.Pin = DQ_GPIO_PIN;                   /* DQ引脚 */
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;          /* 推挽输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                  /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;        /* 高速 */
    HAL_GPIO_Init(DQ_GPIO_PORT, &gpio_init_struct);       /* 初始化DQ引脚 */

    DQ_L;             /* DQ线拉低(单总线协议中单总线低电平时为空闲状态) */
    
    DS18B20_RST();
}



/**********************************************************
* @funcName ：DS18B20_RST
* @brief    ：对DS18B20的复位信号
* @param    ：void
* @retval   ：void
* @details  ：
* @fn       ：
************************************************************/
void DS18B20_RST(void)
{
    DQ_OUT;           /* DQ进入输出模式 */
    DQ_L;             /* 将DQ电平拉低 */
    delay_us(700);    /* 持续480~960us */
    DQ_H;             /* 释放DQ，恢复空闲 */
    delay_us(15);
    DS18B20_ACK();    /* 等待应答 */
}



/**********************************************************
* @funcName ：DS18B20_ACK
* @brief    ：DS18B20的应答信号
* @param    ：void
* @retval   ：uint8_t
* @details  ：
* @fn       ：
************************************************************/
uint8_t DS18B20_ACK(void)
{
    uint8_t ack = 0;
    uint8_t temp = 60;

    DQ_IN;/* DQ进入输入模式 */
    
    /* DS18B20会等待15~60us后给单片机应答 */
    while(DQ_ACK)
    {
        temp--;
        if(temp == 60) return ack;
        delay_us(1);
    }
    
    temp = 240;
    
    /* DS18B20会将DQ拉低60~240us来作为应答 */
    while(temp--)
    {
        if(DQ_ACK)
        {
            ack = 1;
            break;
        }
        delay_us(1);
    }

    return ack;
}



/**********************************************************
* @funcName ：DS18B20_Write_Byte
* @brief    ：对DS18B20进行写一个字节数据
* @param    ：uint8_t data
* @retval   ：void
* @details  ：
* @fn       ：
************************************************************/
void DS18B20_Write_Byte(uint8_t data)
{
    uint8_t i;

    /* DQ进入输出模式 */
    DQ_OUT;
    
    for(i=0;i<8;i++)
    {
        DQ_L;                    /* 拉低DQ，持续10~15us */
        delay_us(12);

        if(data & 0x01) DQ_H;    /* 判断当前要发送的数据最低为是 1 还是 0 */
        else DQ_L;

        delay_us(55);            /* 保持当前DQ电平持续45us */

        DQ_H;                    /* DQ释放，持续1us */
        delay_us(1);
        
        data >>= 1;              /* 要发送的数据右移一位 */
    }
}



/**********************************************************
* @funcName ：DS18B20_Read_Bit
* @brief    ：对DS18B20进行读1个bit(位)数据
* @param    ：void
* @retval   ：uint8_t
* @details  ：
* @fn       ：
************************************************************/
uint8_t DS18B20_Read_Bit(void)
{
    uint8_t data;
    
    DQ_OUT;              /* DQ进入输出模式 */
    
    DQ_L; 
    delay_us(2);
    DQ_H;
    
    DQ_IN;               /* DQ进入输入模式 */
    delay_us(12);
    
    if(DQ_READ)data=1;   /* 读取一个bit的数据 */
    else data=0;
    
    delay_us(50);
    
    return data;
}



/**********************************************************
* @funcName ：DS18B20_Read_Byte
* @brief    ：对DS18B20进行读一个字节数据
* @param    ：void
* @retval   ：uint8_t
* @details  ：
* @fn       ：
************************************************************/
uint8_t DS18B20_Read_Byte(void)
{
    uint8_t i;
    uint8_t data = 0;

    for(i=0;i<8;i++)
    {
        DQ_OUT;                     /* DQ进入输出模式 */
        
        DQ_L;                       /* 拉低DQ持续1us */
        delay_us(1);

        DQ_H;                       /* 拉高DQ持续13us */
        delay_us(12);

        DQ_IN;                      /* DQ进入输入模式 */

        if(DQ_READ) data |= (1<<i); /* 读取数据 */
        delay_us(50);               /* 延时45us */

        DQ_H;                       /* DQ释放，持续1us */
        delay_us(2);
    }
    
    return data;
}



/**********************************************************
* @funcName ：DS18B20_Read_Temperature
* @brief    ：获得DS18B20测量的温度
* @param    ：void
* @retval   ：int16_t
* @details  ：
*            0xcc --- 跳过ROM相关指令
*            0x44 --- 采集温度指令
**           0xbe --- 读取温度数据指令
* @fn       ：
*            注意：这个函数只适用于总线上只有一个DS18B20
************************************************************/
int16_t DS18B20_Read_Temperature(void)
{
    uint8_t data_l ,data_h;
    int16_t data;

    DS18B20_RST();              /* 复位 */
    DS18B20_Write_Byte(0xcc);   /* 跳过ROM相关指令，即不需要确定DS18B20地址 */
    DS18B20_Write_Byte(0x44);   /* 启动温度数据转换 */
    DS18B20_RST();
    DS18B20_Write_Byte(0xcc);
    DS18B20_Write_Byte(0xbe);     /* 读取温度数据 */
    data_l = DS18B20_Read_Byte(); /* 获取温度数据低位 */
    data_h = DS18B20_Read_Byte(); /* 获取温度数据高位 */

    /* 处理温度数据 */
    if(data_h > 0x07)
    {
        data_l = ~data_l;
        data_h = ~data_h;
    }
    data = (data_h<<8)+data_l;
    data *= 0.625;

    printf("%f\r\n",(float)data/10);
    
    return data;
}



/**********************************************************
* @funcName ：DS18B20_SingleAddr
* @brief    ：获得DS18B20的系列号、器件地址、CRC校验码
* @param    ：uint8_t* arr
* @retval   ：void
* @details  ：
*            0x33 --- 读取DS18B20内部的ROM
* @fn       ：
*            DS18B20内部的ROM存放着DS18B20的系列号、器件地址、CRC校验码
*            注意：这个函数只适用于总线上只有一个DS18B20
************************************************************/
void DS18B20_SingleAddr(uint8_t* arr)
{
    uint8_t i;

    DS18B20_RST();
    DS18B20_Write_Byte(0x33);
    
    for(i=0;i<8;i++)
    arr[i] = DS18B20_Read_Byte();

    printf("单线系列编码：%d\r\n",arr[0]);
    printf("地址：");
    for(i=1;i<7;i++) printf("%d ",arr[i]);
    
    printf("\r\n");
    printf("CRC校验码：%d\r\n",arr[7]);
}



/**********************************************************
* @funcName ：DS18B20_Read2Bit
* @brief    ：对DS18B20进行读两个bit(位)数据
* @param    ：void
* @retval   ：uint8_t
* @details  ：
* @fn       ：
************************************************************/
uint8_t DS18B20_Read2Bit(void)
{  
    uint8_t i;
    uint8_t data = 0;

    for(i=0;i<2;i++)
    {
        data <<= 1;                    /* 左移一位 */
        
        DQ_OUT;                        /* DQ进入输出模式 */
        
        DQ_L;                          /* 拉低DQ持续1us */
        delay_us(1);
        DQ_H;                          /* 拉高DQ持续13us */
        delay_us(12);
        
        DQ_IN;                         /* DQ进入输入模式 */

        if(DQ_READ) data |= 0x01;      /* 读取数据 持续45us */
        delay_us(50);

        DQ_H;                          /* DQ释放，持续1us */
        delay_us(2);
    }
    
    return data;
}



/**********************************************************
* @funcName ：DS18B20_WriteBit
* @brief    ：对DS18B20进行写一个bit(位)数据
* @param    ：uint8_t bit
* @retval   ：void
* @details  ：
* @fn       ：
************************************************************/
void DS18B20_WriteBit(uint8_t bit)
{
    DQ_OUT;                  /*DQ进入输出模式*/

    DQ_L;                    /* 拉低DQ，持续10~15us */
    delay_us(12);

    if(bit & 0x01) DQ_H;     /* 判断当前要发送的数据最低为是 1 还是 0 */
    else DQ_L;

    delay_us(55);            /* 保持当前DQ电平持续45us */

    DQ_H;                    /* DQ释放，持续1us */
    delay_us(1);
}



/**********************************************************
* @funcName ：DS18B20_MultiAddr_Test  (实验版，正式函数是下一个函数) 
* @brief    ：搜索一条总线上DS18B20的个数，并获取它们的ROM数据(系列号、器件地址、CRC校验码)
* @param    ：void
* @retval   ：void
* @details  ：
*            0x33 --- 读取DS18B20内部的ROM
*            0xf0 --- 搜索一条总线上所有DS18B20内部的ROM
* @fn       ：
*            这个函数只是用来检验理思路，只测单个，看看0xf0指令和0x33指令
*        得到的ROM数据是否一致。
*            注意：此函数思路比较简单，可以先看此函数了解思路再看下一个正式
*        版函数，正式版的函数思路与此函数有所类似。
*
*            注意：这个函数只适用于总线上只有一个DS18B20
************************************************************/
void DS18B20_MultiAddr_Test(void)
{
    uint8_t i,j;
    uint8_t data;
    uint8_t buff[8];
    uint8_t bit8 = 0;

    DS18B20_RST();                    /* 复位DS18B20总线 */
    DS18B20_Write_Byte(0xf0);         /* 搜索ROM */
    for(i=0;i<8;i++)
    {
        for(j=0;j<8;j++)
        {
            bit8 >>= 1;
            data = DS18B20_Read2Bit();
            data &= 0x3;
            /**************************************************************************
            *data的值如果是：
            *   01 ：还受ROM搜索指令控制的所有DS18B20的该位都是0
            *   10 ：还受ROM搜索指令控制的所有DS18B20的该位都是1
            *   00 ：还受ROM搜索指令控制的所有DS18B20的该位有的是0有的是1，即该位位冲突位
            *   11 ：总线上没有DS18B20
            ***************************************************************************/
            if(data == 0x01)           /* 此位到位0 */
            {
                DS18B20_WriteBit(0);  /* 继续链接此位为0的地址 */
            }
            else if(data == 0x02)
            {
                bit8 |= 0x80;
                DS18B20_WriteBit(1);  /* 继续链接此位为1的地址 */
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




/**********************************************************
* @funcName ：DS18B20_MultiAddr  (正式版ROM搜索函数函数) 
* @brief    ：搜索一条总线上DS18B20的个数，并获取它们的ROM数据(系列号、器件地址、CRC校验码)
* @param    ：uint8_t (*buff)[8], uint8_t *num
* @retval   ：void
* @details  ：
*            0xf0 --- 搜索一条总线上所有DS18B20内部的ROM
* @fn       ：
************************************************************/
void DS18B20_MultiAddr(uint8_t (*buff)[8], uint8_t *num)
{
    uint8_t i,j;
    uint8_t data;             /* 用来接收DS18B20发来的两位数据 */
    uint8_t bit8 = 0;         /* 用于临时装第二层for循环获取到的8个bit的ROM数据(一个字节数据) */
    uint8_t temp[64] = {0};   /* 接收当前搜索的DS18B20的ROM数据，给下一次搜索提供参照(主要用处体现在下一次搜索冲突位的筛选) */
    uint8_t conflict;         /* 发生冲突的位 */
    uint8_t stack[65] = {0};  /* 栈结构   用于标记冲突位*/
    uint8_t top = 0;          /* 栈顶     用于标记最高位的冲突位(ROM数据一共有64个数据位,末尾的8位数据是CRC，前面一样，CRC基本一样，所以冲突位基本不会出现在末尾的后8位) */
    *num = 0;                 /* 初始化这个num，不然如果外面没有定义时赋值0的话会出现错误 */

    do
    {
        DS18B20_RST();              /* 复位DS18B20总线 */
        DS18B20_Write_Byte(0xf0);   /* 发送搜索ROM */
        
        for(i=0;i<8;i++)
        {
            for(j=0;j<8;j++)
            {
                /* 因为搜索都是从低位开始的 */
                bit8 >>= 1; 
                /* 读取两位数据 */
                data = DS18B20_Read2Bit();
                /* 虽然这一步有点多余，但还是形式上写一下 */
                data &= 0x3;
                /**************************************************************************
                *data的值如果是：
                *   01 ：还受ROM搜索指令控制的所有DS18B20的该位都是0
                *   10 ：还受ROM搜索指令控制的所有DS18B20的该位都是1
                *   00 ：还受ROM搜索指令控制的所有DS18B20的该位有的是0有的是1，即该位位冲突位
                *   11 ：总线上没有DS18B20
                ***************************************************************************/
                if(data == 0x01)             /* 还受ROM搜索指令控制的所有DS18B20的该位都是0 */
                {
                    temp[i*8+j] = 0;         /* 记录该位为0 */
                    DS18B20_WriteBit(0);     /* 筛选该位是0的DS18B20(如果该位不是0就不再受ROM搜索指令控制) */
                }
                else if(data == 0x02)        /* 还受ROM搜索指令控制的所有DS18B20的该位都是1 */
                {
                    temp[i*8+j] = 1;         /* 记录该位为1 */
                    bit8 |= 0x80;            /* 记录该位为1 */
                    DS18B20_WriteBit(1);     /* 筛选该位是1的DS18B20(如果该位不是1就不再受ROM搜索指令控制) */
                }
                else if(data == 0x00)        /* 还受ROM搜索指令控制的所有DS18B20的该位有的是0有的是1，即该位位冲突位 */
                {
                    conflict = i*8 + j +1;   /* 第几位发生冲突(ROM数据一共有64个数据位) */

                    /* 如果conflict > stack[top] 说明：有新的更高的冲突位 */
                    if(conflict > stack[top])
                    {
                        /* 记录冲突位 */
                        top++;                  /* 第top次冲突位 */
                        stack[top] = conflict;  /* 记录第几位发生冲突 */
                        /* 我们先把0的搜索出来 */
                        /* 筛选该位为0的DS18B20(不是0的就不再受ROM搜索指令控制) */
                        DS18B20_WriteBit(0);
                        temp[i*8+j] = 0;
                    }
                    /* 进行下面判断时top必然不等于0了，而且已经是第2个及以上的ROM搜索了 */
                    /* 如果该冲突位不是最高冲突位(或者说不是数值最大的)就进行if里的程序 */
                    else if(conflict < stack[top])
                    {
                        /* 因为不是最高的冲突位，所以该位的数值依然和上一次搜索到的ROM数据该位的值一样 */
                        bit8 = bit8 | ((temp[(i*8+j)]&0x01)<<7);
                        /* 和上一次ROM搜索到该位时的筛选一样 */
                        DS18B20_WriteBit (temp[i*8+j]);
                    }
                    /* 这里的判断也一样，进行下面判断时top必然不等于0了，而且已经是第2个及以上的ROM搜索了 */
                    /* 如果该冲突位是最高冲突位(或者说是数值最大的)就进行if里的程序 */
                    else if(conflict == stack[top])
                    {
                        /**
                        * 因为我们总是先把最高冲突为为0的DS18B20的ROM数据读出来，
                        * 所以这次该是最高冲突为为1的DS18B20了
                        */
                        /* 记录该位这次的DS18B20的ROM数据 */
                        bit8 = bit8 | 0x80; 
                        temp[i*8+j]=1;
                        /* 筛选该位为1的DS18B20(不是1的就不再受ROM搜索指令控制) */
                        DS18B20_WriteBit (1);  
                        /* 解决了这个最高冲突位后最高冲突位就要降级了 */
                        stack[top] = 0;
                        top = top - 1;
                    }
                }
                else
                {
                    printf("总线上没有DS18B20器件\r\n");
                    *num=0;
                    return;
                }
            }
            /* 记录每个DS18B20的ROM数据 */
            buff[*num][i] = bit8;
            bit8 = 0;
        }
        /* 记录个数 */
        (*num)++;
    }while((stack[top]!=0) && (*num<=MAXNUM));
}



/**********************************************************
* @funcName ：DS18B20_Addr_Temperature
* @brief    ：通过地址获得某个DS18B20测量的温度
* @param    ：void
* @retval   ：int16_t
* @details  ：
*            0x55 --- 勋章ID指令
*            0x44 --- 采集温度指令
*            0xbe --- 读取温度数据指令
* @fn       ：
*            注意：这个函数只适用于总线上只有一个DS18B20
************************************************************/
int16_t DS18B20_Addr_Temperature(uint8_t* address)
{
    uint8_t data_l ,data_h;
    int16_t data;
    uint8_t i;

    DS18B20_RST();              /* 复位 */
    DS18B20_Write_Byte(0x55);   /* 寻找ID命令 */
    for(i=0; i<8; i++)
    {
        DS18B20_Write_Byte(address[i]);
    }
    DS18B20_Write_Byte(0x44);   /* 启动温度数据转换 */
    
    DS18B20_RST();
    DS18B20_Write_Byte(0x55);
    for(i=0; i<8; i++)
    {
        DS18B20_Write_Byte(address[i]);
    }
    DS18B20_Write_Byte(0xbe);     /* 读取温度数据 */
    
    data_l = DS18B20_Read_Byte(); /* 获取温度数据低位 */
    data_h = DS18B20_Read_Byte(); /* 获取温度数据高位 */

    /* 处理温度数据 */
    if(data_h > 0x07)
    {
        data_l = ~data_l;
        data_h = ~data_h;
    }
    data = (data_h<<8)+data_l;
    data *= 0.625;

    printf("温度:%f\r\n",(float)data/10);
    
    return data;
}


