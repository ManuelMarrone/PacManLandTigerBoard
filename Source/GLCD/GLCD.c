/****************************************Copyright (c)**************************************************                         
**
**                                 http://www.powermcu.com
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			GLCD.c
** Descriptions:		Has been tested SSD1289、ILI9320、R61505U、SSD1298、ST7781、SPFD5408B、ILI9325、ILI9328、
**						HX8346A、HX8347A
**------------------------------------------------------------------------------------------------------
** Created by:			AVRman
** Created date:		2012-3-10
** Version:					1.3
** Descriptions:		The original version
**
**------------------------------------------------------------------------------------------------------
** Modified by:			Paolo Bernardi
** Modified date:		03/01/2020
** Version:					2.0
** Descriptions:		simple arrangement for screen usage
********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "GLCD.h" 
#include "AsciiLib.h"
#include <stdbool.h>
#include <stdio.h> /*for sprintf*/

/* Private variables ---------------------------------------------------------*/
static uint8_t LCD_Code;

/* Private define ------------------------------------------------------------*/
#define  ILI9320    0  /* 0x9320 */
#define  ILI9325    1  /* 0x9325 */
#define  ILI9328    2  /* 0x9328 */
#define  ILI9331    3  /* 0x9331 */
#define  SSD1298    4  /* 0x8999 */
#define  SSD1289    5  /* 0x8989 */
#define  ST7781     6  /* 0x7783 */
#define  LGDP4531   7  /* 0x4531 */
#define  SPFD5408B  8  /* 0x5408 */
#define  R61505U    9  /* 0x1505 0x0505 */
#define  HX8346A		10 /* 0x0046 */  
#define  HX8347D    11 /* 0x0047 */
#define  HX8347A    12 /* 0x0047 */	
#define  LGDP4535   13 /* 0x4535 */  
#define  SSD2119    14 /* 3.5 LCD 0x9919 */

/*******************************************************************************
* Function Name  : Lcd_Configuration
* Description    : Configures LCD Control lines
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_Configuration(void)
{
	/* Configure the LCD Control pins */
	
	/* EN = P0.19 , LE = P0.20 , DIR = P0.21 , CS = P0.22 , RS = P0.23 , RS = P0.23 */
	/* RS = P0.23 , WR = P0.24 , RD = P0.25 , DB[0.7] = P2.0...P2.7 , DB[8.15]= P2.0...P2.7 */  
	LPC_GPIO0->FIODIR   |= 0x03f80000;
	LPC_GPIO0->FIOSET    = 0x03f80000;
}

/*******************************************************************************
* Function Name  : LCD_Send
* Description    : LCD写数据
* Input          : - byte: byte to be sent
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_Send (uint16_t byte) 
{
	LPC_GPIO2->FIODIR |= 0xFF;          /* P2.0...P2.7 Output */
	LCD_DIR(1)		   				    				/* Interface A->B */
	LCD_EN(0)	                        	/* Enable 2A->2B */
	LPC_GPIO2->FIOPIN =  byte;          /* Write D0..D7 */
	LCD_LE(1)                         
	LCD_LE(0)														/* latch D0..D7	*/
	LPC_GPIO2->FIOPIN =  byte >> 8;     /* Write D8..D15 */
}

/*******************************************************************************
* Function Name  : wait_delay
* Description    : Delay Time
* Input          : - nCount: Delay Time
* Output         : None
* Return         : None
* Return         : None
* Attention		 : None 
*******************************************************************************/
static void wait_delay(int count)
{
	while(count--);
}

/*******************************************************************************
* Function Name  : LCD_Read
* Description    : LCD读数据
* Input          : - byte: byte to be read
* Output         : None
* Return         : 返回读取到的数据
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) uint16_t LCD_Read (void) 
{
	uint16_t value;
	
	LPC_GPIO2->FIODIR &= ~(0xFF);              /* P2.0...P2.7 Input */
	LCD_DIR(0);		   				           				 /* Interface B->A */
	LCD_EN(0);	                               /* Enable 2B->2A */
	wait_delay(30);							   						 /* delay some times */     //delay software per mantenere i tempi richiesti 
	value = LPC_GPIO2->FIOPIN0;                /* Read D8..D15 */
	LCD_EN(1);	                               /* Enable 1B->1A */
	wait_delay(30);							   						 /* delay some times */
	value = (value << 8) | LPC_GPIO2->FIOPIN0; /* Read D0..D7 */
	LCD_DIR(1);
	return  value;
}

/*******************************************************************************
* Function Name  : LCD_WriteIndex
* Description    : LCD写寄存器地址
* Input          : - index: 寄存器地址
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_WriteIndex(uint16_t index)
{
	LCD_CS(0);
	LCD_RS(0);
	LCD_RD(1);
	LCD_Send( index ); 
	wait_delay(22);	
	LCD_WR(0);  
	wait_delay(1);
	LCD_WR(1);
	LCD_CS(1);
}

/*******************************************************************************
* Function Name  : LCD_WriteData
* Description    : LCD写寄存器数据
* Input          : - index: 寄存器数据
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_WriteData(uint16_t data)
{				
	LCD_CS(0);
	LCD_RS(1);   
	LCD_Send( data );
	LCD_WR(0);     
	wait_delay(1);
	LCD_WR(1);
	LCD_CS(1);
}

/*******************************************************************************
* Function Name  : LCD_ReadData
* Description    : 读取控制器数据
* Input          : None
* Output         : None
* Return         : 返回读取到的数据
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) uint16_t LCD_ReadData(void)
{ 
	uint16_t value;
	
	LCD_CS(0);
	LCD_RS(1);
	LCD_WR(1);
	LCD_RD(0);
	value = LCD_Read();
	
	LCD_RD(1);
	LCD_CS(1);
	
	return value;
}

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Writes to the selected LCD register.
* Input          : - LCD_Reg: address of the selected register.
*                  - LCD_RegValue: value to write to the selected register.
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue)
{ 
	/* Write 16-bit Index, then Write Reg */  
	LCD_WriteIndex(LCD_Reg);         
	/* Write 16-bit Reg */
	LCD_WriteData(LCD_RegValue);  
}

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Reads the selected LCD Register.
* Input          : None
* Output         : None
* Return         : LCD Register Value.
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
	uint16_t LCD_RAM;
	
	/* Write 16-bit Index (then Read Reg) */
	LCD_WriteIndex(LCD_Reg);
	/* Read 16-bit Reg */
	LCD_RAM = LCD_ReadData();      	
	return LCD_RAM;
}

/*******************************************************************************
* Function Name  : LCD_SetCursor
* Description    : Sets the cursor position.
* Input          : - Xpos: specifies the X position.
*                  - Ypos: specifies the Y position. 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_SetCursor(uint16_t Xpos,uint16_t Ypos)
{
    #if  ( DISP_ORIENTATION == 90 ) || ( DISP_ORIENTATION == 270 )
	
 	uint16_t temp = Xpos;

			 Xpos = Ypos;
			 Ypos = ( MAX_X - 1 ) - temp;  

	#elif  ( DISP_ORIENTATION == 0 ) || ( DISP_ORIENTATION == 180 )
		
	#endif

  switch( LCD_Code )
  {
     default:		 /* 0x9320 0x9325 0x9328 0x9331 0x5408 0x1505 0x0505 0x7783 0x4531 0x4535 */
          LCD_WriteReg(0x0020, Xpos );     
          LCD_WriteReg(0x0021, Ypos );     
	      break; 

     case SSD1298: 	 /* 0x8999 */
     case SSD1289:   /* 0x8989 */
	      LCD_WriteReg(0x004e, Xpos );      
          LCD_WriteReg(0x004f, Ypos );          
	      break;  

     case HX8346A: 	 /* 0x0046 */
     case HX8347A: 	 /* 0x0047 */
     case HX8347D: 	 /* 0x0047 */
	      LCD_WriteReg(0x02, Xpos>>8 );                                                  
	      LCD_WriteReg(0x03, Xpos );  

	      LCD_WriteReg(0x06, Ypos>>8 );                           
	      LCD_WriteReg(0x07, Ypos );    
	
	      break;     
     case SSD2119:	 /* 3.5 LCD 0x9919 */
	      break; 
  }
}

/*******************************************************************************
* Function Name  : LCD_Delay
* Description    : Delay Time
* Input          : - nCount: Delay Time
* Output         : None
* Return         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void delay_ms(uint16_t ms)    
{ 
	uint16_t i,j; 
	for( i = 0; i < ms; i++ )
	{ 
		for( j = 0; j < 1141; j++ );
	}
} 


/*******************************************************************************
* Function Name  : LCD_Initializtion
* Description    : Initialize TFT Controller.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_Initialization(void)
{
	uint16_t DeviceCode;
	
	LCD_Configuration();
	delay_ms(100);
	DeviceCode = LCD_ReadReg(0x0000);		/* 读取屏ID	*/	
	
	if( DeviceCode == 0x9325 || DeviceCode == 0x9328 )	
	{
		LCD_Code = ILI9325;
		LCD_WriteReg(0x00e7,0x0010);      
		LCD_WriteReg(0x0000,0x0001);  	/* start internal osc */
		LCD_WriteReg(0x0001,0x0100);     
		LCD_WriteReg(0x0002,0x0700); 	/* power on sequence */
		LCD_WriteReg(0x0003,(1<<12)|(1<<5)|(1<<4)|(0<<3) ); 	/* importance */
		LCD_WriteReg(0x0004,0x0000);                                   
		LCD_WriteReg(0x0008,0x0207);	           
		LCD_WriteReg(0x0009,0x0000);         
		LCD_WriteReg(0x000a,0x0000); 	/* display setting */        
		LCD_WriteReg(0x000c,0x0001);	/* display setting */        
		LCD_WriteReg(0x000d,0x0000); 			        
		LCD_WriteReg(0x000f,0x0000);
		/* Power On sequence */
		LCD_WriteReg(0x0010,0x0000);   
		LCD_WriteReg(0x0011,0x0007);
		LCD_WriteReg(0x0012,0x0000);                                                                 
		LCD_WriteReg(0x0013,0x0000);                 
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0010,0x1590);   
		LCD_WriteReg(0x0011,0x0227);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0012,0x009c);                  
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0013,0x1900);   
		LCD_WriteReg(0x0029,0x0023);
		LCD_WriteReg(0x002b,0x000e);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0020,0x0000);                                                            
		LCD_WriteReg(0x0021,0x0000);           
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0030,0x0007); 
		LCD_WriteReg(0x0031,0x0707);   
		LCD_WriteReg(0x0032,0x0006);
		LCD_WriteReg(0x0035,0x0704);
		LCD_WriteReg(0x0036,0x1f04); 
		LCD_WriteReg(0x0037,0x0004);
		LCD_WriteReg(0x0038,0x0000);        
		LCD_WriteReg(0x0039,0x0706);     
		LCD_WriteReg(0x003c,0x0701);
		LCD_WriteReg(0x003d,0x000f);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0050,0x0000);        
		LCD_WriteReg(0x0051,0x00ef);   
		LCD_WriteReg(0x0052,0x0000);     
		LCD_WriteReg(0x0053,0x013f);
		LCD_WriteReg(0x0060,0xa700);        
		LCD_WriteReg(0x0061,0x0001); 
		LCD_WriteReg(0x006a,0x0000);
		LCD_WriteReg(0x0080,0x0000);
		LCD_WriteReg(0x0081,0x0000);
		LCD_WriteReg(0x0082,0x0000);
		LCD_WriteReg(0x0083,0x0000);
		LCD_WriteReg(0x0084,0x0000);
		LCD_WriteReg(0x0085,0x0000);
		  
		LCD_WriteReg(0x0090,0x0010);     
		LCD_WriteReg(0x0092,0x0000);  
		LCD_WriteReg(0x0093,0x0003);
		LCD_WriteReg(0x0095,0x0110);
		LCD_WriteReg(0x0097,0x0000);        
		LCD_WriteReg(0x0098,0x0000);  
		/* display on sequence */    
		LCD_WriteReg(0x0007,0x0133);
		
		LCD_WriteReg(0x0020,0x0000);  /* 行首址0 */                                                          
		LCD_WriteReg(0x0021,0x0000);  /* 列首址0 */     
	}

    delay_ms(50);   /* delay 50 ms */	
}

/*******************************************************************************
* Function Name  : LCD_Clear
* Description    : 将屏幕填充成指定的颜色，如清屏，则填充 0xffff
* Input          : - Color: Screen Color
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_Clear(uint16_t Color)
{
	uint32_t index;
	
	if( LCD_Code == HX8347D || LCD_Code == HX8347A )
	{
		LCD_WriteReg(0x02,0x00);                                                  
		LCD_WriteReg(0x03,0x00);  
		                
		LCD_WriteReg(0x04,0x00);                           
		LCD_WriteReg(0x05,0xEF);  
		                 
		LCD_WriteReg(0x06,0x00);                           
		LCD_WriteReg(0x07,0x00);    
		               
		LCD_WriteReg(0x08,0x01);                           
		LCD_WriteReg(0x09,0x3F);     
	}
	else
	{	
		LCD_SetCursor(0,0); 
	}	

	LCD_WriteIndex(0x0022);
	for( index = 0; index < MAX_X * MAX_Y; index++ )
	{
		LCD_WriteData(Color);
	}
}

/******************************************************************************
* Function Name  : LCD_BGR2RGB
* Description    : RRRRRGGGGGGBBBBB 改为 BBBBBGGGGGGRRRRR 格式
* Input          : - color: BRG 颜色值  
* Output         : None
* Return         : RGB 颜色值
* Attention		 : 内部函数调用
*******************************************************************************/
static uint16_t LCD_BGR2RGB(uint16_t color)
{
	uint16_t  r, g, b, rgb;
	
	b = ( color>>0 )  & 0x1f;
	g = ( color>>5 )  & 0x3f;
	r = ( color>>11 ) & 0x1f;
	
	rgb =  (b<<11) + (g<<5) + (r<<0);
	
	return( rgb );
}

/******************************************************************************
* Function Name  : LCD_GetPoint
* Description    : 获取指定座标的颜色值
* Input          : - Xpos: Row Coordinate
*                  - Xpos: Line Coordinate 
* Output         : None
* Return         : Screen Color
* Attention		 : None
*******************************************************************************/
uint16_t LCD_GetPoint(uint16_t Xpos,uint16_t Ypos)
{
	uint16_t dummy;
	
	LCD_SetCursor(Xpos,Ypos);
	LCD_WriteIndex(0x0022);  
	
	switch( LCD_Code )
	{
		case ST7781:
		case LGDP4531:
		case LGDP4535:
		case SSD1289:
		case SSD1298:
             dummy = LCD_ReadData();   /* Empty read */
             dummy = LCD_ReadData(); 	
 		     return  dummy;	      
	    case HX8347A:
	    case HX8347D:
             {
		        uint8_t red,green,blue;
				
				dummy = LCD_ReadData();   /* Empty read */

		        red = LCD_ReadData() >> 3; 
                green = LCD_ReadData() >> 2; 
                blue = LCD_ReadData() >> 3; 
                dummy = (uint16_t) ( ( red<<11 ) | ( green << 5 ) | blue ); 
		     }	
	         return  dummy;

        default:	/* 0x9320 0x9325 0x9328 0x9331 0x5408 0x1505 0x0505 0x9919 */
             dummy = LCD_ReadData();   /* Empty read */
             dummy = LCD_ReadData(); 	
 		     return  LCD_BGR2RGB( dummy );
	}
}

/******************************************************************************
* Function Name  : LCD_SetPoint
* Description    : 在指定座标画点
* Input          : - Xpos: Row Coordinate
*                  - Ypos: Line Coordinate 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_SetPoint(uint16_t Xpos,uint16_t Ypos,uint16_t point)
{
	if( Xpos >= MAX_X || Ypos >= MAX_Y )
	{
		return;
	}
	LCD_SetCursor(Xpos,Ypos);
	LCD_WriteReg(0x0022,point);
}




/******************************************************************************
* Function Name  : LCD_DrawLine
* Description    : Bresenham's line algorithm
* Input          : - x1: A点行座标
*                  - y1: A点列座标 
*				   - x2: B点行座标
*				   - y2: B点列座标 
*				   - color: 线颜色
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/	 
void LCD_DrawLine( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1 , uint16_t color ) //linea da x0 a y0 a x1 y1
{
    short dx,dy;      /* 定义X Y轴上增加的变量值 */
    short temp;       /* 起点 终点大小比较 交换数据时的中间变量 */

    if( x0 > x1 )     /* X轴上起点大于终点 交换数据 */
    {
	    temp = x1;
		x1 = x0;
		x0 = temp;   
    }
    if( y0 > y1 )     /* Y轴上起点大于终点 交换数据 */
    {
		temp = y1;
		y1 = y0;
		y0 = temp;   
    }
  
	dx = x1-x0;       /* X轴方向上的增量 */
	dy = y1-y0;       /* Y轴方向上的增量 */

    if( dx == 0 )     /* X轴上没有增量 画垂直线 */ 
    {
        do
        { 
            LCD_SetPoint(x0, y0, color);   /* 逐点显示 描垂直线 */
            y0++;
        }
        while( y1 >= y0 ); 
		return; 
    }
    if( dy == 0 )     /* Y轴上没有增量 画水平直线 */ 
    {
        do
        {
            LCD_SetPoint(x0, y0, color);   /* 逐点显示 描水平线 */
            x0++;
        }
        while( x1 >= x0 ); 
		return;
    }
	/* 布兰森汉姆(Bresenham)算法画线 */
    if( dx > dy )                         /* 靠近X轴 */
    {
	    temp = 2 * dy - dx;               /* 计算下个点的位置 */         
        while( x0 != x1 )
        {
	        LCD_SetPoint(x0,y0,color);    /* 画起点 */ 
	        x0++;                         /* X轴上加1 */
	        if( temp > 0 )                /* 判断下下个点的位置 */
	        {
	            y0++;                     /* 为右上相邻点，即（x0+1,y0+1） */ 
	            temp += 2 * dy - 2 * dx; 
	 	    }
            else         
            {
			    temp += 2 * dy;           /* 判断下下个点的位置 */  
			}       
        }
        LCD_SetPoint(x0,y0,color);
    }  
    else
    {
	    temp = 2 * dx - dy;                      /* 靠近Y轴 */       
        while( y0 != y1 )
        {
	 	    LCD_SetPoint(x0,y0,color);     
            y0++;                 
            if( temp > 0 )           
            {
                x0++;               
                temp+=2*dy-2*dx; 
            }
            else
			{
                temp += 2 * dy;
			}
        } 
        LCD_SetPoint(x0,y0,color);
	}
} 


/******************************************************************************
* Function Name  : PutChar
* Description    : 将Lcd屏上任意位置显示一个字符
* Input          : - Xpos: 水平坐标 
*                  - Ypos: 垂直坐标  
*				   - ASCI: 显示的字符
*				   - charColor: 字符颜色   
*				   - bkColor: 背景颜色 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void PutChar( uint16_t Xpos, uint16_t Ypos, uint8_t ASCI, uint16_t charColor, uint16_t bkColor )
{
	uint16_t i, j;
    uint8_t buffer[16], tmp_char;
    GetASCIICode(buffer,ASCI);  /* 取字模数据 */
    for( i=0; i<16; i++ )
    {
        tmp_char = buffer[i];
        for( j=0; j<8; j++ )
        {
            if( ((tmp_char >> (7 - j)) & 0x01) == 0x01 )
            {
                LCD_SetPoint( Xpos + j, Ypos + i, charColor );  /* 字符颜色 */
            }
            else
            {
                LCD_SetPoint( Xpos + j, Ypos + i, bkColor );  /* 背景颜色 */
            }
        }
    }
}

/******************************************************************************
* Function Name  : GUI_Text
* Description    : 在指定座标显示字符串
* Input          : - Xpos: 行座标
*                  - Ypos: 列座标 
*				   - str: 字符串
*				   - charColor: 字符颜色   
*				   - bkColor: 背景颜色 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void GUI_Text(uint16_t Xpos, uint16_t Ypos, uint8_t *str,uint16_t Color, uint16_t bkColor) //Xpos e Ypos rappresentano la posizione del primo carattere
{
    uint8_t TempChar;
    do
    {
        TempChar = *str++;  
        PutChar( Xpos, Ypos, TempChar, Color, bkColor );    
        if( Xpos < MAX_X - 8 )
        {
            Xpos += 8;
        } 
        else if ( Ypos < MAX_Y - 16 )
        {
            Xpos = 0;
            Ypos += 16;
        }   
        else
        {
            Xpos = 0;
            Ypos = 0;
        }    
    }
    while ( *str != 0 );
}

/*Funzioni personalizzate*/

uint16_t y;
uint16_t x1;
uint16_t x2;
int i;
int powerPills[6];
int k;
int value;

//Generazione palla

void LCD_DrawFilledCircle(uint16_t xCenter, uint16_t yCenter, uint16_t radius, uint16_t color) {
    int x = radius - 1;  // Coordinate iniziali
    int y = 0;
    int dx = 1;
    int dy = 1;
		int i;
    int err = dx - (radius << 1);

    while (x >= y) {
        // Disegna i punti simmetrici e riempi tra loro
        for (i = xCenter - x; i <= xCenter + x; i++) {
            LCD_SetPoint(i, yCenter + y, color);
            LCD_SetPoint(i, yCenter - y, color);
        }
        for (i = xCenter - y; i <= xCenter + y; i++) {
            LCD_SetPoint(i, yCenter + x, color);
            LCD_SetPoint(i, yCenter - x, color);
        }

        // Aggiorna i valori per il prossimo punto
        if (err <= 0) {
            y++;
            err += dy;
            dy += 2;
        }
        if (err > 0) {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

//Generazione cella

void LCD_DrawFilledSquare(uint16_t startX, uint16_t startY, uint16_t size, uint16_t color) {
    // Disegna un quadrato riempito di dimensione specificata
    for (y = startY; y < startY + size; y++) {
        LCD_DrawLine(startX, y, startX + size - 1, y, color); // Riempi linea per linea
    }
}

//Generazione celle in sequenza

void LCD_DrawConsecutiveSquares(uint16_t startX, uint16_t startY, uint16_t numSquares, uint16_t color) {
    uint16_t size = 10; // Dimensione fissa del quadrato
    uint16_t currentX = startX;
    uint16_t currentY = startY;

    for (i = 0; i < numSquares; i++) {
        LCD_DrawFilledSquare(currentX, currentY, size, color); // Disegna il quadrato
        currentX += size; // Sposta a destra per il prossimo quadrato
    }
}

//Controllo presenza o meno di un determinato elemento nelle power pills

bool isInPower(int el)
{
	for(k=0;k<6;k++)
	{
		if(el == powerPills[k])
		{
			return true;
		}
	}
	return false;
}

//Generazione pills

void pills()
{
	int coordinates[240][2] = {
    {55, 45}, {55, 55}, {55, 65}, {55, 75}, {55, 85}, {55, 95}, {55, 105},
    {55, 115}, {55, 125}, {55, 135}, {55, 145}, {55, 155}, {55, 165}, {55, 175},
    {55, 185}, {55, 195}, {55, 205}, {55, 215}, {55, 225}, {55, 235}, {55, 245},
		{55, 255}, {185, 55}, {185, 65}, {185, 75}, {185, 85}, {185, 95}, {185, 105},
		{185, 115}, {185, 125}, {185, 135}, {185, 145}, {185, 155}, {185, 165}, {185, 175},
		{185, 185}, {185, 195}, {185, 205}, {185, 215}, {185, 225}, {185, 235}, {185, 245},
		{185, 255}, {65, 45}, {75, 45}, {85, 45}, {95, 45}, {105, 45}, {65, 205},
		{75, 205}, {85, 205}, {95, 205}, {105, 205}, {135, 205}, {145, 205}, {155, 205},
		{165, 205}, {175, 205},  {65, 75}, {75, 75}, {85, 75}, {95, 75}, {105, 75},
		{115, 75}, {125, 75}, {135, 75}, {145, 75}, {155, 75}, {165, 75}, {175, 75},
    {105, 55}, {105, 65}, {135, 55}, {135, 65}, {85, 85}, {85, 95}, {85, 105},
    {155, 85}, {155, 95}, {155, 105}, {95, 105}, {105, 105}, {135, 105}, {145, 105},
    {85, 185}, {95, 185}, {105, 185}, {115, 185}, {125, 185}, {135, 185}, {145, 185},
    {155, 185}, {85, 195}, {155, 195}, {195, 205}, {205, 205}, {215, 205}, {225, 205},
    {225, 215}, {225, 225}, {225, 235}, {215, 235}, {205, 235}, {205, 245}, {205, 255},
    {195, 255}, {155, 245}, {135, 215}, {135, 225}, {105, 215}, {105, 225}, {45, 205},
		{35, 205}, {25, 205}, {15, 205}, {15, 215}, {15, 225}, {15, 235}, {35, 235},
		{35, 245}, {35, 255}, {25, 235}, {45, 255}, {65, 235}, {75, 235}, {85, 235},
		{95, 235}, {105, 235}, {135, 235}, {145, 235}, {155, 235}, {165, 235},
		{175, 235}, {135, 45}, {145, 45}, {155, 45}, {165, 45}, {175, 45}, {185, 45},
    {85, 245},  {45, 45}, {35, 45}, {25, 45}, {45, 75}, {35, 75}, {25, 75},
    {45, 105}, {35, 105}, {25, 105}, {15, 45}, {15, 55}, {15, 65}, {15, 75},
		{15, 85}, {15, 95}, {15, 105}, {195, 45}, {205, 45}, {215, 45}, {195, 75},
		{205, 75}, {215, 75}, {195, 105}, {205, 105}, {215, 105}, {225, 45}, {225, 55},
		{225, 65}, {225, 75}, {225, 85}, {225, 95}, {225, 105}, {65, 155}, {75, 155},
    {85, 125}, {85, 135}, {85, 145}, {85, 155}, {95, 125}, {105, 125}, {105, 115},
    {135, 115}, {135, 125}, {145, 125}, {155, 125}, {155, 135}, {155, 145}, {155, 155},
    {165, 155}, {175, 155},  {15, 285}, {25, 285}, {35, 285}, {45, 285}, {55, 285},
		{65, 285},  {75, 285}, {85, 285}, {95, 285}, {105, 285}, {115, 285}, {125, 285},
    {135, 285}, {145, 285}, {155, 285}, {165, 285}, {175, 285}, {185, 285}, {195, 285},
		{205, 285}, {215, 285}, {225, 285},  {25, 255}, {15, 255}, {15, 265}, {15, 275},
    {215, 255}, {225, 255}, {225, 265}, {225, 275}, {85, 255}, {95, 255}, {105, 255},
		{105, 265}, {105, 275}, {155, 255}, {145, 255}, {135, 255}, {135, 265}, {135, 275},
    {35,155},{45,155}, {195,155},{205,155}, {85, 165}, {85, 175}, {155,165},
		{155,175},	{115, 125}, {125, 125}
};
	//power pills
	srand(LPC_TIM0->TC); 
	for(i=0;i<6;i++)
	{
		value = rand()%240;
		if(!isInPower(value))
		{
			powerPills[i]=value;
		}
		else
		{
			i--;
		}
	}
	// Iterazione sulle coordinate e salvataggio in variabili
	for (i = 0; i < 240; i++) {
			int x = coordinates[i][0];
			int y = coordinates[i][1];
			if(isInPower(i))
			{
				LCD_DrawFilledCircle(x, y, 3, Magenta);//power
			}
			else
			{
					LCD_DrawFilledCircle(x, y, 2, White);
			}
	}
	
}

//Generazione schermata di vittoria

void victoryDisplay()
{
	LCD_Clear(Black);
	GUI_Text(90,50,(uint8_t *)"Victory!", Black, White);
	//pacman
	LCD_DrawConsecutiveSquares(90,100,6,Yellow);
	LCD_DrawConsecutiveSquares(80,110,4,Yellow);
	LCD_DrawConsecutiveSquares(120,110,1,Black);
	LCD_DrawConsecutiveSquares(130,110,1,White);
	LCD_DrawConsecutiveSquares(140,110,2,Yellow);
	LCD_DrawConsecutiveSquares(70,120,5,Yellow);
	LCD_DrawConsecutiveSquares(120,120,2,Black);
	LCD_DrawConsecutiveSquares(140,120,3,Yellow);
	LCD_DrawConsecutiveSquares(60,130,10,Yellow);
	LCD_DrawConsecutiveSquares(60,140,8,Yellow);
	LCD_DrawConsecutiveSquares(60,150,6,Yellow);
	LCD_DrawConsecutiveSquares(60,160,8,Yellow);
	LCD_DrawConsecutiveSquares(60,170,10,Yellow);
	LCD_DrawConsecutiveSquares(70,180,10,Yellow);
	LCD_DrawConsecutiveSquares(80,190,8,Yellow);
	LCD_DrawConsecutiveSquares(90,200,6,Yellow);
	//blu
	LCD_DrawConsecutiveSquares(150,250,1,Blue);
	LCD_DrawConsecutiveSquares(170,250,1,Blue);
	LCD_DrawConsecutiveSquares(190,250,1,Blue);
	LCD_DrawConsecutiveSquares(150,260,5,Blue);
	LCD_DrawConsecutiveSquares(150,270,5,Blue);
	LCD_DrawConsecutiveSquares(150,280,1,Blue);
	LCD_DrawConsecutiveSquares(160,280,1,Black);
	LCD_DrawConsecutiveSquares(170,280,1,Blue);
	LCD_DrawConsecutiveSquares(180,280,1,Black);
	LCD_DrawConsecutiveSquares(190,280,1,Blue);
	LCD_DrawConsecutiveSquares(150,290,5,Blue);
	LCD_DrawConsecutiveSquares(160,300,3,Blue);
	//rosso
	LCD_DrawConsecutiveSquares(190,100,1,Red);
	LCD_DrawConsecutiveSquares(210,100,1,Red);
	LCD_DrawConsecutiveSquares(230,100,1,Red);
	LCD_DrawConsecutiveSquares(190,110,5,Red);
	LCD_DrawConsecutiveSquares(190,120,5,Red);
	LCD_DrawConsecutiveSquares(190,130,1,Red);
	LCD_DrawConsecutiveSquares(200,130,1,Black);
	LCD_DrawConsecutiveSquares(210,130,1,Red);
	LCD_DrawConsecutiveSquares(220,130,1,Black);
	LCD_DrawConsecutiveSquares(230,130,1,Red);
	LCD_DrawConsecutiveSquares(190,140,5,Red);
	LCD_DrawConsecutiveSquares(200,150,3,Red);
	//arancione
	LCD_DrawConsecutiveSquares(10,200,1,Orange);
	LCD_DrawConsecutiveSquares(30,200,1,Orange);
	LCD_DrawConsecutiveSquares(50,200,1,Orange);
	LCD_DrawConsecutiveSquares(10,210,5,Orange);
	LCD_DrawConsecutiveSquares(10,220,5,Orange);
	LCD_DrawConsecutiveSquares(10,230,1,Orange);
	LCD_DrawConsecutiveSquares(20,230,1,Black);
	LCD_DrawConsecutiveSquares(30,230,1,Orange);
	LCD_DrawConsecutiveSquares(40,230,1,Black);
	LCD_DrawConsecutiveSquares(50,230,1,Orange);
	LCD_DrawConsecutiveSquares(10,240,5,Orange);
	LCD_DrawConsecutiveSquares(20,250,3,Orange);
}

//Generazione schermata di game over

void gameOverDisplay()
{
	LCD_Clear(Black);
	GUI_Text(80,50,(uint8_t *)"Game Over!", Black, Red);
	//pacman
	LCD_DrawConsecutiveSquares(90,100,6,Yellow);
	LCD_DrawConsecutiveSquares(80,110,3,Yellow);
	LCD_DrawConsecutiveSquares(110,110,1,Black);
	LCD_DrawConsecutiveSquares(120,110,1,Yellow);
		LCD_DrawConsecutiveSquares(130,110,1,Black);
	LCD_DrawConsecutiveSquares(140,110,2,Yellow);
	LCD_DrawConsecutiveSquares(70,120,5,Yellow);
	LCD_DrawConsecutiveSquares(120,120,1,Black);
	LCD_DrawConsecutiveSquares(130,120,4,Yellow);
	LCD_DrawConsecutiveSquares(60,130,5,Yellow);
	LCD_DrawConsecutiveSquares(110,130,1,Black);
	LCD_DrawConsecutiveSquares(120,130,1,Yellow);
	LCD_DrawConsecutiveSquares(130,130,1,Black);
	LCD_DrawConsecutiveSquares(140,130,2,Yellow);
	LCD_DrawConsecutiveSquares(60,140,8,Yellow);
	LCD_DrawConsecutiveSquares(60,150,6,Yellow);
	LCD_DrawConsecutiveSquares(60,160,8,Yellow);
	LCD_DrawConsecutiveSquares(60,170,10,Yellow);
	LCD_DrawConsecutiveSquares(70,180,10,Yellow);
	LCD_DrawConsecutiveSquares(80,190,8,Yellow);
	LCD_DrawConsecutiveSquares(90,200,6,Yellow);
	//blu
	LCD_DrawConsecutiveSquares(150,300,1,Blue);
	LCD_DrawConsecutiveSquares(170,300,1,Blue);
	LCD_DrawConsecutiveSquares(190,300,1,Blue);
	LCD_DrawConsecutiveSquares(150,290,5,Blue);
	LCD_DrawConsecutiveSquares(150,280,5,Blue);
	LCD_DrawConsecutiveSquares(150,270,1,Blue);
	LCD_DrawConsecutiveSquares(160,270,1,Black);
	LCD_DrawConsecutiveSquares(170,270,1,Blue);
	LCD_DrawConsecutiveSquares(180,270,1,Black);
	LCD_DrawConsecutiveSquares(190,270,1,Blue);
	LCD_DrawConsecutiveSquares(150,260,5,Blue);
	LCD_DrawConsecutiveSquares(160,250,3,Blue);
	//rosso
	LCD_DrawConsecutiveSquares(190,150,1,Red);
	LCD_DrawConsecutiveSquares(210,150,1,Red);
	LCD_DrawConsecutiveSquares(230,150,1,Red);
	LCD_DrawConsecutiveSquares(190,140,5,Red);
	LCD_DrawConsecutiveSquares(190,130,5,Red);
	LCD_DrawConsecutiveSquares(190,120,1,Red);
	LCD_DrawConsecutiveSquares(200,120,1,Black);
	LCD_DrawConsecutiveSquares(210,120,1,Red);
	LCD_DrawConsecutiveSquares(220,120,1,Black);
	LCD_DrawConsecutiveSquares(230,120,1,Red);
	LCD_DrawConsecutiveSquares(190,110,5,Red);
	LCD_DrawConsecutiveSquares(200,100,3,Red);
	//arancione
	LCD_DrawConsecutiveSquares(10,250,1,Orange);
	LCD_DrawConsecutiveSquares(30,250,1,Orange);
	LCD_DrawConsecutiveSquares(50,250,1,Orange);
	LCD_DrawConsecutiveSquares(10,240,5,Orange);
	LCD_DrawConsecutiveSquares(10,230,5,Orange);
	LCD_DrawConsecutiveSquares(10,220,1,Orange);
	LCD_DrawConsecutiveSquares(20,220,1,Black);
	LCD_DrawConsecutiveSquares(30,220,1,Orange);
	LCD_DrawConsecutiveSquares(40,220,1,Black);
	LCD_DrawConsecutiveSquares(50,220,1,Orange);
	LCD_DrawConsecutiveSquares(10,210,5,Orange);
	LCD_DrawConsecutiveSquares(20,200,3,Orange);
}

//Generazione labirinto

void drawMaze()
{
	LCD_DrawConsecutiveSquares(0,30,32,Blue); 
	LCD_DrawConsecutiveSquares(0,40,1,Blue);
	LCD_DrawConsecutiveSquares(110,40,2,Blue);
	LCD_DrawConsecutiveSquares(230,40,1,Blue);
	LCD_DrawConsecutiveSquares(0,50,1,Blue);
	LCD_DrawConsecutiveSquares(20,50,3,Blue);
	LCD_DrawConsecutiveSquares(60,50,4,Blue);
	LCD_DrawConsecutiveSquares(110,50,2,Blue);
	LCD_DrawConsecutiveSquares(140,50,4,Blue);
	LCD_DrawConsecutiveSquares(190,50,3,Blue);
	LCD_DrawConsecutiveSquares(230,50,1,Blue);
	LCD_DrawConsecutiveSquares(0,60,1,Blue);
	LCD_DrawConsecutiveSquares(20,60,3,Blue);
	LCD_DrawConsecutiveSquares(60,60,4,Blue);
	LCD_DrawConsecutiveSquares(110,60,2,Blue);
	LCD_DrawConsecutiveSquares(140,60,4,Blue);
	LCD_DrawConsecutiveSquares(190,60,3,Blue);
	LCD_DrawConsecutiveSquares(230,60,1,Blue);
	LCD_DrawConsecutiveSquares(0,70,1,Blue);
	LCD_DrawConsecutiveSquares(230,70,1,Blue);
	LCD_DrawConsecutiveSquares(0,80,1,Blue);
	LCD_DrawConsecutiveSquares(20,80,3,Blue);
	LCD_DrawConsecutiveSquares(60,80,2,Blue);
	LCD_DrawConsecutiveSquares(90,80,6,Blue);	
	LCD_DrawConsecutiveSquares(190,80,3,Blue);
	LCD_DrawConsecutiveSquares(160,80,2,Blue);
	LCD_DrawConsecutiveSquares(230,80,1,Blue);
	LCD_DrawConsecutiveSquares(0,90,1,Blue);
	LCD_DrawConsecutiveSquares(20,90,3,Blue);
	LCD_DrawConsecutiveSquares(60,90,2,Blue);
	LCD_DrawConsecutiveSquares(90,90,6,Blue);
	LCD_DrawConsecutiveSquares(160,90,2,Blue);
	LCD_DrawConsecutiveSquares(190,90,3,Blue);
	LCD_DrawConsecutiveSquares(230,90,1,Blue);
	LCD_DrawConsecutiveSquares(0,100,1,Blue);
	LCD_DrawConsecutiveSquares(60,100,2,Blue);
	LCD_DrawConsecutiveSquares(110,100,2,Blue);
	LCD_DrawConsecutiveSquares(160,100,2,Blue);
	LCD_DrawConsecutiveSquares(230,100,1,Blue);
	LCD_DrawConsecutiveSquares(60,110,4,Blue);
	LCD_DrawConsecutiveSquares(110,110,2,Blue);
	LCD_DrawConsecutiveSquares(140,110,4,Blue);
	LCD_DrawConsecutiveSquares(190,110,5,Blue);
	LCD_DrawConsecutiveSquares(0,110,5,Blue);
	LCD_DrawConsecutiveSquares(40,120,1,Blue);
	LCD_DrawConsecutiveSquares(60,120,2,Blue);
	LCD_DrawConsecutiveSquares(160,120,2,Blue);
	LCD_DrawConsecutiveSquares(190,120,1,Blue);
	LCD_DrawConsecutiveSquares(40,130,1,Blue);
	LCD_DrawConsecutiveSquares(60,130,2,Blue);
	LCD_DrawConsecutiveSquares(90,130,6,Blue);
	LCD_DrawConsecutiveSquares(160,130,2,Blue);
	LCD_DrawConsecutiveSquares(190,130,1,Blue);
	LCD_DrawConsecutiveSquares(0,140,5,Blue);
	LCD_DrawConsecutiveSquares(60,140,2,Blue);
	LCD_DrawConsecutiveSquares(90,140,1,Blue);
	LCD_DrawConsecutiveSquares(140,140,1,Blue);
	LCD_DrawConsecutiveSquares(160,140,2,Blue);
	LCD_DrawConsecutiveSquares(190,140,5,Blue);
	LCD_DrawConsecutiveSquares(90,150,1,Blue);
	LCD_DrawConsecutiveSquares(140,150,1,Blue);
	LCD_DrawConsecutiveSquares(0,160,5,Blue);
	LCD_DrawConsecutiveSquares(60,160,2,Blue);
	LCD_DrawConsecutiveSquares(90,160,1,Blue);
	LCD_DrawConsecutiveSquares(140,160,1,Blue);
	LCD_DrawConsecutiveSquares(160,160,2,Blue);
	LCD_DrawConsecutiveSquares(190,160,5,Blue);
	LCD_DrawConsecutiveSquares(40,170,1,Blue);
	LCD_DrawConsecutiveSquares(60,170,2,Blue);
	LCD_DrawConsecutiveSquares(90,170,6,Blue);
	LCD_DrawConsecutiveSquares(160,170,2,Blue);
	LCD_DrawConsecutiveSquares(190,170,1,Blue);
	LCD_DrawConsecutiveSquares(40,180,1,Blue);
	LCD_DrawConsecutiveSquares(60,180,2,Blue);
	LCD_DrawConsecutiveSquares(160,180,2,Blue);
	LCD_DrawConsecutiveSquares(190,180,1,Blue);
	LCD_DrawConsecutiveSquares(0,190,5,Blue);
	LCD_DrawConsecutiveSquares(60,190,2,Blue);
	LCD_DrawConsecutiveSquares(90,190,6,Blue);
	LCD_DrawConsecutiveSquares(160,190,2,Blue);
	LCD_DrawConsecutiveSquares(190,190,5,Blue);
	LCD_DrawConsecutiveSquares(0,200,1,Blue);
	LCD_DrawConsecutiveSquares(110,200,2,Blue);
	LCD_DrawConsecutiveSquares(230,200,1,Blue);
	LCD_DrawConsecutiveSquares(0,210,1,Blue);
	LCD_DrawConsecutiveSquares(20,210,3,Blue);
	LCD_DrawConsecutiveSquares(60,210,4,Blue);
	LCD_DrawConsecutiveSquares(110,210,2,Blue);
	LCD_DrawConsecutiveSquares(140,210,4,Blue);
	LCD_DrawConsecutiveSquares(190,210,3,Blue);
	LCD_DrawConsecutiveSquares(230,210,1,Blue);
	LCD_DrawConsecutiveSquares(0,220,1,Blue);
	LCD_DrawConsecutiveSquares(20,220,3,Blue);
	LCD_DrawConsecutiveSquares(60,220,4,Blue);
	LCD_DrawConsecutiveSquares(110,220,2,Blue);
	LCD_DrawConsecutiveSquares(140,220,4,Blue);
	LCD_DrawConsecutiveSquares(190,220,3,Blue);
	LCD_DrawConsecutiveSquares(230,220,1,Blue);
	LCD_DrawConsecutiveSquares(0,230,1,Blue);
	LCD_DrawConsecutiveSquares(40,230,1,Blue);
	LCD_DrawConsecutiveSquares(190,230,1,Blue);
	LCD_DrawConsecutiveSquares(230,230,1,Blue);
	LCD_DrawConsecutiveSquares(0,240,3,Blue);
	LCD_DrawConsecutiveSquares(40,240,1,Blue);
	LCD_DrawConsecutiveSquares(60,240,2,Blue);
	LCD_DrawConsecutiveSquares(90,240,6,Blue);
	LCD_DrawConsecutiveSquares(160,240,2,Blue);
	LCD_DrawConsecutiveSquares(190,240,1,Blue);
	LCD_DrawConsecutiveSquares(210,240,3,Blue);
	LCD_DrawConsecutiveSquares(0,250,1,Blue);
	LCD_DrawConsecutiveSquares(60,250,2,Blue);
	LCD_DrawConsecutiveSquares(110,250,2,Blue);
	LCD_DrawConsecutiveSquares(160,250,2,Blue);
	LCD_DrawConsecutiveSquares(230,250,1,Blue);
	LCD_DrawConsecutiveSquares(0,260,1,Blue);
	LCD_DrawConsecutiveSquares(20,260,8,Blue);
	LCD_DrawConsecutiveSquares(110,260,2,Blue);
	LCD_DrawConsecutiveSquares(140,260,8,Blue);
	LCD_DrawConsecutiveSquares(230,260,1,Blue);
	LCD_DrawConsecutiveSquares(0,270,1,Blue);
	LCD_DrawConsecutiveSquares(20,270,8,Blue);
	LCD_DrawConsecutiveSquares(110,270,2,Blue);
	LCD_DrawConsecutiveSquares(140,270,8,Blue);
	LCD_DrawConsecutiveSquares(230,270,1,Blue);
	LCD_DrawConsecutiveSquares(0,280,1,Blue);
	LCD_DrawConsecutiveSquares(230,280,1,Blue);
	LCD_DrawConsecutiveSquares(0,290,32,Blue);
}



/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
