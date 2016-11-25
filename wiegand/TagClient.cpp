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
// include for e-tag
#include <stdio.h>
#include <stdlib.h>         // exit
#include <unistd.h>         // usleep
#include <errno.h>
#include <string.h>         // strcnimp
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "wiegand.h"
#include <lcd.h>
// include for net
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <arpa/inet.h> 
#include <time.h>

//USE WIRINGPI PIN NUMBERS
#define LCD_RS  25               //Register select pin
#define LCD_E   24               //Enable Pin
#define LCD_D4  23               //Data pin 4
#define LCD_D5  22               //Data pin 5
#define LCD_D6  21               //Data pin 6
#define LCD_D7  14               //Data pin 7

//******************************************************************************
// WIEGAND
//******************************************************************************

// 要比對的 RFID Tag ID；可以更改為
//char hexTagID[11] = "0x002285EA";  // 一定要 8 個 16 進位，不足的前方補 0
long decTagID     = 6602151;            // 2262506 = 0x2285EA

// 設定與 HZ-1050 板子上面的 D0 與 D1 與樹莓派何接腳相接
int WIEGAND::_pinD0 = 23;
int WIEGAND::_pinD1 = 24;

WIEGAND wg;

void error(char *msg) {
    perror(msg);
    exit(0);
}

void sendData( int sockfd, char* msg ) {
  int n;

  char buffer[256];
  sprintf( buffer, "%s\n", msg );
  if ( (n = write( sockfd, buffer, strlen(buffer) ) ) < 0 )
      error( const_cast<char *>( "ERROR writing to socket") );
  buffer[n] = '\0';
}

int getData( int sockfd ) {
  char buffer[32];
  int n;

  if ( (n = read(sockfd,buffer,31) ) < 0 )
       error( const_cast<char *>( "ERROR reading from socket") );
  buffer[n] = '\0';
  return atoi( buffer );
}

int main()
{
    int i;

    int sockfd, portno = 51717, n;
    char serverIp[] = "192.168.100.111";
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    int data;
    long int start_time;
	long int time_difference;
	struct timespec gettime_now;
    
    // init the connection to server
    if ( ( sockfd = socket(AF_INET, SOCK_STREAM, 0) ) < 0 )
        error( const_cast<char *>( "ERROR opening socket") );

    if ( ( server = gethostbyname( serverIp ) ) == NULL ) 
        error( const_cast<char *>("ERROR, no such host\n") );
    
    bzero( (char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if ( connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error( const_cast<char *>( "ERROR connecting") );
    
    // 初始化 wiringPi，並使用 BCM_GPIO 的接腳號碼
    if (wiringPiSetupGpio() == -1)
    {
        fprintf (stderr, "Unable to setup wiringPi GPIO, errno: %s\n", strerror (errno)) ;
        exit(1);
    }
    
	printf("before wg");
	fflush(stdout);

    // 開始 Wiegand 辨識
    wg.begin();
	clock_gettime(CLOCK_REALTIME, &gettime_now);
	start_time = gettime_now.tv_nsec;		//Get nS value
	for(;;)
    {

		clock_gettime(CLOCK_REALTIME, &gettime_now);
		time_difference = gettime_now.tv_nsec - start_time;
        if( wg.available() )
        {
            #ifdef DEBUG
                printf( "Wiegand HEX = 0x%8lX", wg.getCode() );
        		printf( ", DECIMAL = %10lu", wg.getCode() );
        		printf( ", Type W%2d", wg.getWiegandType() );
            #endif

			//printf( "DECIMAL = %10lu\nW%2d", wg.getCode(),wg.getWiegandType() );
			sprintf(buffer, "%10lu",wg.getCode());
			sendData( sockfd, buffer);
            if( (wg.getCode()) == decTagID )
        	{
    			printf( "DEC Compare OK !");
                fflush(stdout);
    			/*
    				增加比對成功的處理碼在這邊
    				...			
    			*/
    		}
    		else
    		{
                printf( "DEC Compare NG !");
                fflush(stdout);
    		}
        }
    }
    close( sockfd );
    return 0;
}
