/**********************************************************************************************************************
 * Author: Yikai Huang
 * Description: 21/5/2025
 * 
 * Version:1.0
 *********************************************************************************************************************/

#ifndef XINDONGLIB_DISPLAY_H_
#define XINDONGLIB_DISPLAY_H_

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include "EI2C.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
/*FontSize参数取值*/
/*此参数值不仅用于判断，而且用于计算横向字符偏移，默认值为字体像素宽度*/
#define OLED_8X16				8
#define OLED_6X8				6

/*IsFilled参数数值*/
#define OLED_UNFILLED			0
#define OLED_FILLED				1

#define OLED_I2C_ADDRESS        0x78	//I2C地址

/*中文字符字节宽度*/
#define OLED_CHN_CHAR_WIDTH         3       //UTF-8编码格式给3，GB2312编码格式给2
/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/
extern uint8 OLED_DisplayBuf[8][128];

/*********************************************************************************************************************/
/*-------------------------------------------------Data Structures---------------------------------------------------*/
/*********************************************************************************************************************/
/*字模基本单元*/
typedef struct
{
    uint8 Data[32];                     //字模数据
    char Index[OLED_CHN_CHAR_WIDTH + 1];    //汉字索引
} ChineseCell_t;
/*********************************************************************************************************************/
/*--------------------------------------------Private Variables/Constants--------------------------------------------*/
/*********************************************************************************************************************/
/*ASCII字模数据声明*/
extern const uint8 OLED_F8x16[][16];
extern const uint8 OLED_F6x8[][6];

/*汉字字模数据声明*/
extern const ChineseCell_t OLED_CF16x16[];

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/
/*初始化函数*/
void OLED_Init(void);

/*更新函数*/
void OLED_Update(void);
void OLED_UpdateArea(uint8 X, uint8 Y, uint8 Width, uint8 Height);

/*显存控制函数*/
void OLED_Clear(void);
void OLED_ClearArea(uint8 X, uint8 Y, uint8 Width, uint8 Height);
void OLED_Reverse(void);
void OLED_ReverseArea(uint8 X, uint8 Y, uint8 Width, uint8 Height);

/*显示函数*/
void OLED_ShowChar(uint8 X, uint8 Y, char Char, uint8 FontSize);
void OLED_ShowString(uint8 X, uint8 Y, char *String, uint8 FontSize);
void OLED_ShowNum(uint8 X, uint8 Y, uint32 Number, uint8 Length, uint8 FontSize);
void OLED_ShowSignedNum(uint8 X, uint8 Y, uint32 Number, uint8 Length, uint8 FontSize);
void OLED_ShowHexNum(uint8 X, uint8 Y, uint32 Number, uint8 Length, uint8 FontSize);
void OLED_ShowBinNum(uint8 X, uint8 Y, uint32 Number, uint8 Length, uint8 FontSize);
void OLED_ShowFloatNum(uint8 X, uint8 Y, double Number, uint8 IntLength, uint8 FraLength, uint8 FontSize);
void OLED_ShowChinese(uint8 X, uint8 Y, char *Chinese);
void OLED_ShowImage(uint8 X, uint8 Y, uint8 Width, uint8 Height, const uint8 *Image);
void OLED_Printf(uint8 X, uint8 Y, uint8 FontSize, char *format, ...);

/*绘图函数*/
void OLED_DrawPoint(uint8 X, uint8 Y);
uint8 OLED_GetPoint(uint8 X, uint8 Y);
void OLED_DrawLine(uint8 X0, uint8 Y0, uint8 X1, uint8 Y1);
void OLED_DrawRectangle(uint8 X, uint8 Y, uint8 Width, uint8 Height, uint8 IsFilled);
void OLED_DrawTriangle(uint8 X0, uint8 Y0, uint8 X1, uint8 Y1, uint8 X2, uint8 Y2, uint8 IsFilled);
void OLED_DrawCircle(uint8 X, uint8 Y, uint8 Radius, uint8 IsFilled);
void OLED_DrawEllipse(uint8 X, uint8 Y, uint8 A, uint8 B, uint8 IsFilled);
void OLED_DrawArc(uint8 X, uint8 Y, uint8 Radius, uint16 StartAngle, uint16 EndAngle, uint8 IsFilled);

//自定义函数
/*********************函数声明*/
//void OLED_Rotation_C_P(int8 CX, int8 CY, float* PX, float* PY, int16 Angle);	//点旋转
//void OLED_Rotation_Block(int8 X, int8 Y, int8 Width, int16 Angle);			//区块旋转
//void bresenham_ellipse(int x0, int y0, int rx, int ry);
//void ellipse_algorithm(int8 x0, int8 y0, int8 a, int8 b);

#endif /* XINDONGLIB_DISPLAY_H_ */
