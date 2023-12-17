#ifndef __DS18B20_H
#define __DS18B20_H

    /****************   外部头文件声明   ****************/
    #include "sys.h"
    #include "delay.h"
    #include "stdio.h"



    /********************   宏定义   ********************/
    /* DQ宏定义 */
    #define DQ_H     GPIOG->ODR |= (1<<11)                          /* DQ线拉高 */
    #define DQ_L     GPIOG->ODR &= ~(1<<11)                         /* DQ线拉低 */
    #define DQ_IN   {GPIOG->CRH&=0XFFFF0FFF;GPIOG->CRH|=8<<12;}     /* 切换为输入模式 */
    #define DQ_OUT  {GPIOG->CRH&=0XFFFF0FFF;GPIOG->CRH|=3<<12;}     /* 切换为输出模式 */
    #define DQ_READ  GPIOG->IDR & (1<<11)                           /* 读数据 */
    #define DQ_ACK   GPIOG->IDR & (1<<11)                           /* 判断应答 */

    /* 最多能挂载DS18B20的个数 */
    #define MAXNUM     4



    /****************    函数外部声明   *****************/
    void DS18B20_Init(void);                   /* DS18B20初始化 */
    void DS18B20_RST(void);                    /* 复位 */
    uint8_t DS18B20_ACK(void);                 /* 等待应答 */
    void DS18B20_Write_Byte(uint8_t data);     /* 发送1个字节数据 */
    void DS18B20_WriteBit(uint8_t bit);        /* 发送1个bit数据 */
    uint8_t DS18B20_Read_Bit(void);            /* 读取1个bit数据 */
    uint8_t DS18B20_Read2Bit(void);            /* 读取2个bit数据 */
    uint8_t DS18B20_Read_Byte(void);           /* 读1个字节数据 */

    int16_t DS18B20_Read_Temperature(void);    /* 读取温度   无寻址   单个DS18B20 */
    void DS18B20_SingleAddr(uint8_t* arr);     /* 获取获取ROM数据(器件信息) 单个DS18B20 */

    void DS18B20_MultiAddr_Test(void);         /* 搜索总线上DS18B20并获取ROM数据(器件信息)  测试版本 只能搜索1个 */
    void DS18B20_MultiAddr(uint8_t (*buff)[8], uint8_t *num);  /* 搜索总线上DS18B20并获取他们的ROM数据(器件信息)，并记录数量  正式版本 */
    int16_t DS18B20_Addr_Temperature(uint8_t* address);        /* 通过DS18B20器件地址读取某个DS18B20的温度 */

#endif


