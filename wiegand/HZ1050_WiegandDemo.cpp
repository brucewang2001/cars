 /******************************************************************************
 * 
 * HZ1050_WiegandDemo.c
 * 
 * author : ruten.proteus
 * date   : 2014/10
 * version: 0.1
 * 
 * Compile: sudo gcc wiegand.cpp HZ1050_WiegandDemo.cpp -lwiringPi -o hz1050_wieganddemo
 * 
 * Description: @Raspberry Pi 700MHz
 * 
 *  在整合型 LCD 上顯示 RFID 卡的號碼，並與預設卡號碼做比對顯示結果，
 *  使用 Wiegand 方式傳輸資料。
 * 
 * result: Work !
 * 
 * ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>         // exit
#include <unistd.h>         // usleep
#include <errno.h>
#include <string.h>         // strcnimp
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "wiegand.h"
#include <lcd.h>

//USE WIRINGPI PIN NUMBERS
#define LCD_RS  25               //Register select pin
#define LCD_E   24               //Enable Pin
#define LCD_D4  23               //Data pin 4
#define LCD_D5  22               //Data pin 5
#define LCD_D6  21               //Data pin 6
#define LCD_D7  14               //Data pin 7


//******************************************************************************
// LCD
//******************************************************************************
#define LCD_I2C_ADDR        0x3C
#define IIC_REG_COMMAND     0x80
#define IIC_REG_WRITEDATA   0x40

int pIIC_LCD;

void lcd_init()
{
    // Fucntion set
    //  D0 = 1, 4-bit interface; 0: 8-bit interface
    //  N  = 1, 2-line display
    //  F  = 0, 5x8 font-size
    wiringPiI2CWriteReg8( pIIC_LCD, IIC_REG_COMMAND, 0x3c );
    // Display ON/OFF
    //  D = 1, display on
    //  C = 0, cursor off
    //  B = 0, blick  off
    wiringPiI2CWriteReg8( pIIC_LCD, IIC_REG_COMMAND, 0x0c );
    wiringPiI2CWriteReg8( pIIC_LCD, IIC_REG_COMMAND, 0x01 );   // Clear display
    // Entry Mode
    //  I/D = 1, increment
    //  S   = 0, shift of entire display is not perform.
    wiringPiI2CWriteReg8( pIIC_LCD, IIC_REG_COMMAND, 0x06 );   // Extry Mode    
}

void lcd_Clear()
{
    wiringPiI2CWriteReg8( pIIC_LCD, IIC_REG_COMMAND, 0x01 );
}

/*在指定行和列位置顯示指定的字母、數字（5*7點陣的）*/
void lcd_DisplayCharAt(int line, int column, const char dp[], int len)
{
    int i;
    wiringPiI2CWriteReg8( pIIC_LCD, IIC_REG_COMMAND, (0x80 + ( line - 1) * 0x40 + ( column - 1 )))  ;
    for(i = 0; i < len; i++)
        wiringPiI2CWriteReg8( pIIC_LCD, IIC_REG_WRITEDATA, dp[i] );
}
//******************************************************************************
// WIEGAND
//******************************************************************************

// 要比對的 RFID Tag ID；可以更改為
//char hexTagID[11] = "0x002285EA";  // 一定要 8 個 16 進位，不足的前方補 0
long decTagID     = 2262506;            // 2262506 = 0x2285EA

// 設定與 HZ-1050 板子上面的 D0 與 D1 與樹莓派何接腳相接
int WIEGAND::_pinD0 = 23;
int WIEGAND::_pinD1 = 24;

WIEGAND wg;

//******************************************************************************
//******************************************************************************
//#define DEBUG       // if DEBUG

int main()
{
    int i;

    // 初始化 wiringPi，並使用 BCM_GPIO 的接腳號碼
    if (wiringPiSetupGpio() == -1)
    {
        fprintf (stderr, "Unable to setup wiringPi GPIO, errno: %s\n", strerror (errno)) ;
        exit(1);
    }
    
    // 取得存取 LCD IIC 的 FD
    if((pIIC_LCD = wiringPiI2CSetup(LCD_I2C_ADDR)) == -1)
    {
        fprintf (stderr, "Unable to get file handle of IIC (LCD), errno: %s\n", strerror (errno)) ;
        exit(1) ;
    }
        
    //lcd_init();
    //delay(100);
    //lcd_Clear();
    //delay(100);

printf("before wg");
fflush(stdout);

    // 開始 Wiegand 辨識
    wg.begin();
  
//    int lcd;
//    wiringPiSetup();
//    lcd = lcdInit (2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7, 0, 0, 0, 0);

  
    for(;;)
    {
        if( wg.available() )
        {
            #ifdef DEBUG
                printf( "Wiegand HEX = 0x%8lX", wg.getCode() );
        		printf( ", DECIMAL = %10lu", wg.getCode() );
        		printf( ", Type W%2d", wg.getWiegandType() );
            #endif

printf( "DECIMAL = %10lu\nW%2d", wg.getCode(),wg.getWiegandType() );

//long c = wg.getCode();
//char lcdbuflcd[17];
//sprintf( lcdbuflcd, "HEX%8lX, W%d", c, wg.getWiegandType() );

//lcdPosition(lcd, 0, 0);
//lcdPuts(lcd, lcdbuflcd);
//sleep(2);
//lcdClear(lcd);
//sleep(2);        
    
            // 顯示資料在 LCD 上
        	char lcdbuf[17];
    		sprintf( lcdbuf, "HEX%8lX, W%d", wg.getCode(), wg.getWiegandType() );
    		lcd_DisplayCharAt( 1, 1, lcdbuf, 16 );
    		sprintf( lcdbuf, "DEC%10lu,", wg.getCode(), 14 );
    		lcd_DisplayCharAt( 2, 1, lcdbuf, 14 );
            if( (wg.getCode()) == decTagID )
        	{
    			printf( "DEC Compare OK !");
                fflush(stdout);
                lcd_DisplayCharAt( 2, 15, "OK", 2 );
    			/*
    				增加比對成功的處理碼在這邊
    				...			
    			*/
    		}
    		else
    		{
                //printf( "DEC Compare NG !");
                fflush(stdout);
    			lcd_DisplayCharAt( 2, 15, "NG", 2 );
    		}
        }
    }
    return 0;
}
