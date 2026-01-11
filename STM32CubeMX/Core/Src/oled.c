/********************************************************************************

* @File oled.c

* @Author: Ma Ziteng

* @Version: 1.0

* @Date: 2025-11

* @Description: 0.96 OLED��ʾ�������μ���Ʒ�ֲ��д

********************************************************************************/
#include "oled.h"
#include "oledfont.h"

#include "i2c.h"

/**
 *  @note �ṩ���½ӿ�
 *  I2C_Transmit
 *  OLED_ADDRESS
 *  TIMEOUT
 */
#define     I2C_Transmit(ADDRESS,Control,Data,Len,Timeout)    HAL_I2C_Mem_Write(&hi2c1,ADDRESS,Control,I2C_MEMADD_SIZE_8BIT,Data,Len,Timeout)
#define     OLED_ADDRESS    0x78
#define     TIMEOUT         1000
//������
uint8_t command[23]={
    0xAE,
    0xD5,0x80,
    0xA8,0x3F,
    0xD3,0x00,
    0x40,
    0xA1,
    0xC8,
    0xDA,0x12,
    0x81,0xCF,
    0xD9,0xF1,
    0xDB,0x30,
    0xA4,0xA6,
    0x8D,0x14,
    0xAF
};

//��������,ռ��1kb�ռ䡣8�У�ÿ��һ���ֽڿ���128�У�ÿ��һ��λ��
uint8_t OLED_DisplayBuf[8][128];



/**
* @brief    д������
* @param    Command       �����ֽ�
* @retval   1   ERROR
* @retval   2   BUSY
* @retval   3   TIMEOUT
* @retval   0   OK
*/
uint8_t OLED_WriteCommand(uint8_t Command) {
    //Control�ֽ� Co->0 ����������һ�������ֽ�+n�������ֽ� D/C->0 ��ʾ�����ȫΪ�����ֽ� ��Ϊ0000 0000
    return I2C_Transmit(OLED_ADDRESS,0x00,&Command,1,TIMEOUT);
}

/**
* @brief    д������
* @param    Data       �����ֽ�
* @retval   1   ERROR
* @retval   2   BUSY
* @retval   3   TIMEOUT
* @retval   0   OK
*/
uint8_t OLED_WriteData(uint8_t Data) {
    return I2C_Transmit(OLED_ADDRESS,0x40,&Data,1,TIMEOUT);
}
/**
* @brief    д�볤����
* @param    Data       �����ֽ�
* @param    Len        ���ݳ���
* @retval   1   ERROR
* @retval   2   BUSY
* @retval   3   TIMEOUT
* @retval   0   OK
*/

/**
* @brief    д�볤����
* @param    Command       �����ֽ�
* @param    Len           �����
* @retval   1   ERROR
* @retval   2   BUSY
* @retval   3   TIMEOUT
* @retval   0   OK
*/
uint8_t OLED_WriteLenCommand(uint8_t* Command,uint8_t Len) {
    return I2C_Transmit(OLED_ADDRESS,0x00,Command,Len,TIMEOUT);
}

uint8_t OLED_WriteLenData(uint8_t* Data,uint8_t Len) {
    return I2C_Transmit(OLED_ADDRESS,0x40,Data,Len,TIMEOUT);
}


/**
* @brief    ���ù��λ��
* @param    Page    ҳλ��
* @param    x       x����λ��
* @return   ��
*/
void OLED_SetCursor(uint8_t Page,uint8_t x) {
    OLED_WriteCommand(0x00 | (x & 0x0F));//0x00-> 0000 0000 ����λ��λΪ0��ʾд��x�����4λ
    OLED_WriteCommand(0x10 | ((x & 0xF0)>>4));//0x10-> 0001 0000 ����λ��λΪ1��ʾд��x�����4λ
    OLED_WriteCommand(0xB0 | Page);      //0xB0-> 1011 0000 ����λ��ʾд��ҳ��ַ
}

/**
* @brief    ������ˢ�µ��Դ���
* @param    ��
* @return   ��
*/
void OLED_UpdateScreen(uint8_t Begin,uint8_t End) {
    uint8_t i;
    for (i=Begin;i<End;i++) {
        OLED_SetCursor(i,0);
        OLED_WriteLenData(OLED_DisplayBuf[i],128);
    }
}
/**
* @brief    OLED����
* @param    ��
* @return   ��
*/
void OLED_Clear(void) {
    uint8_t i,j;
    for (i=0;i<8;i++)
        for (j=0;j<128;j++)
            OLED_DisplayBuf[i][j]=0;//д�����ݺ󣬹���Զ��ƶ����¸��ֽڣ����ƣ�
    OLED_UpdateScreen(0,8);
}
/**
* @brief    OLED��ʾ�ַ�
* @param    x   ��Ļ�Ϻ����꣨0~15��
* @param    y   ��Ļ�������꣨0~3��
* @param    Char   Ҫ��ʾ���ַ�
* @return   ��
*/
void OLED_ShowChar(uint8_t x,uint8_t y,char Char) {
    uint8_t i;
    for (i=0;i<8;i++)
        OLED_DisplayBuf[2*y][8*x+i] = F8X16[16*(Char - ' ') + i];
    for (;i<16;i++)
        OLED_DisplayBuf[2*y+1][8*x+i-8] = F8X16[16*(Char - ' ') + i];
    OLED_UpdateScreen(2*y,2*y+2);
}
/**
* @brief    OLED��ʾ�ַ���
* @param    x   ��Ļ�Ϻ����꣨0~15��
* @param    y   ��Ļ�������꣨0~3��
* @param    Str   Ҫ��ʾ���ַ���
* @param    Len   Ҫ��ʾ���ַ�������
* @return   ��
*/
void OLED_ShowString(uint8_t x,uint8_t y,const char* Str,uint8_t Len) {
    uint8_t i,j,Char;
    for (i=0;i<Len;i++,x++) {
        Char = Str[i];
        for (j=0;j<8;j++)
            OLED_DisplayBuf[2*y][8*x+j] = F8X16[16*(Char - ' ') + j];
        for (;j<16;j++)
            OLED_DisplayBuf[2*y+1][8*x+j-8] = F8X16[16*(Char - ' ') + j];
    }
    OLED_UpdateScreen(2*y,2*y+2);
}
/**
* @brief    OLED��ʾ�޷�������
* @param    x   ��Ļ�Ϻ����꣨0~15��
* @param    y   ��Ļ�������꣨0~3��
* @param    Num   Ҫ��ʾ������
* @param    Len   Ҫ��ʾ�����ֵĳ���
* @return   ��
*/
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t Num,uint8_t Len) {
    uint8_t i,j,Char;
    x+=(Len-1);
    for (i=0;i<Len;i++,x--) {
        Char = Num % 10 ;
        Num /= 10;
        for (j=0;j<8;j++)
            OLED_DisplayBuf[2*y][8*x+j] = F8X16[16*(Char + '0' - ' ') + j];
        for (;j<16;j++)
            OLED_DisplayBuf[2*y+1][8*x+j-8] = F8X16[16*(Char + '0' - ' ') + j];
    }
    OLED_UpdateScreen(2*y,2*y+2);
}
/**
* @brief    OLED��ʾ�з�������
* @param    x   ��Ļ�Ϻ����꣨0~15��
* @param    y   ��Ļ�������꣨0~3��
* @param    Num   Ҫ��ʾ������
* @param    Len   Ҫ��ʾ�����ֵĳ���
* @return   ��
*/
void OLED_ShowSignNum(uint8_t x,uint8_t y,int32_t Num,uint8_t Len) {
    uint8_t i,j,Char;
    if (Num<0) {
        for (j=0;j<8;j++)
            OLED_DisplayBuf[2*y][8*x+j] = F8X16[16*('-' - ' ') + j];
        for (;j<16;j++)
            OLED_DisplayBuf[2*y+1][8*x+j-8] = F8X16[16*('-' - ' ') + j];
        x++,Len--,Num=-Num;
    }
    x+=(Len-1);
    for (i=0;i<Len;i++,x--) {
        Char = Num % 10 ;
        Num /= 10;
        for (j=0;j<8;j++)
            OLED_DisplayBuf[2*y][8*x+j] = F8X16[16*(Char + '0' - ' ') + j];
        for (;j<16;j++)
            OLED_DisplayBuf[2*y+1][8*x+j-8] = F8X16[16*(Char + '0' - ' ') + j];
    }
    OLED_UpdateScreen(2*y,2*y+2);
}
/**
* @brief    OLED��ʼ������������
* @param    ��
* @return   ��
*/
void OLED_Init(void) {
    OLED_WriteLenCommand(command,23);
    OLED_SetCursor(0,0);
    OLED_Clear();
}