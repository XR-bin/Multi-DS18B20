#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "ds18b20.h"

int main(void)
{
//    uint8_t arr[8]={0};

    uint8_t buff[MAXNUM][8];
    uint8_t num=0;
    uint8_t n=0, i=0;

    sys_stm32_clock_init(9);  /* 系统时钟初始化*/
    SysTick_Init();           /* 延时初始化 */
    USART1_Init(115200);      /* 串口初始 115200 */
    DS18B20_Init();           /* DS18B20初始化 */

    /* 用0x33指令获取单个DS18B20内部ROM数据 */
//    DS18B20_SingleAddr(arr);
    /* 用0xf0指令获取单个DS18B20内部ROM数据 */
//    DS18B20_MultiAddr_Test();

    /* 搜索总线上多个DS18B20 */
    DS18B20_MultiAddr(buff, &num);
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

    delay_ms(1000);

    DS18B20_Addr_Temperature(buff[0]);

    while(1)
    {
    }
}




