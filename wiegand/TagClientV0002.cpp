/* 
 * Auth. : Bravo Bruce
 * Func. : Connect to an E-Tag Reader and report tag code to the Server
 * 			1. Connecting to E-Tag Reader
 * 			2. Connecting to Server
 * 			3. Listening tag code from the Reader
 * 			4. Report tag code to Server on reading
*/

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
// 設定 E-Tag 讀頭所需 WIEGAND 協定相關變數
//******************************************************************************

// 要比對的 RFID Tag ID；可以更改為
//char hexTagID[11] = "0x002285EA";  // 一定要 8 個 16 進位，不足的前方補 0
long decTagID     = 6602151;            // 2262506 = 0x2285EA

// 設定與 HZ-1050 板子上面的 D0 與 D1 與樹莓派何接腳相接
int WIEGAND::_pinD0 = 23;
int WIEGAND::_pinD1 = 24;

WIEGAND wg;

int error(char *msg) {
    perror(msg);
    return 0;
}

void sendData( int sockfd, char* msg ) {
  int n;

  char buffer[256];
  sprintf( buffer, "%s", msg );
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
    char buffer2[256];
    int data;
    long int start_time;
	long int time_difference;
	struct timespec gettime_now;
	int delay_us;
    
    int constat = 0;
    int doit = 1;
	clock_gettime(CLOCK_REALTIME, &gettime_now);
	start_time = gettime_now.tv_sec;		//Get nS value    
    while(constat == 0)
	{    
		if(doit==1)
		{
			//printf("looo\n");
			//fflush(stdout);
			constat = 1;
			// init the connection to server
			if ( ( sockfd = socket(AF_INET, SOCK_STREAM, 0) ) < 0 )
				constat = error( const_cast<char *>( "ERROR opening socket") );
			//printf("222");
			//fflush(stdout);
			if ( ( server = gethostbyname( serverIp ) ) == NULL ) 
				constat = error( const_cast<char *>("ERROR, no such host\n") );
			//printf("333");
			//fflush(stdout);
			bzero( (char *) &serv_addr, sizeof(serv_addr));
			serv_addr.sin_family = AF_INET;
			bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
			serv_addr.sin_port = htons(portno);
			//printf("444");
			//fflush(stdout);
			if ( connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
				constat = error( const_cast<char *>( "ERROR connecting") );
			//printf("after connect code");
			//fflush(stdout);
		}
		if (constat == 0)
		{
			//printf("wait");
			//fflush(stdout);
			close( sockfd );
			doit = 0;
			clock_gettime(CLOCK_REALTIME, &gettime_now);
			time_difference = gettime_now.tv_sec - start_time;
			if (time_difference > 10)
			{
				clock_gettime(CLOCK_REALTIME, &gettime_now);
				start_time = gettime_now.tv_sec;		//Get nS value  
				doit = 1;
			}
		}
    }
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
	start_time = gettime_now.tv_sec;		//Get nS value
	for(;;)
    {
		clock_gettime(CLOCK_REALTIME, &gettime_now);
		time_difference = gettime_now.tv_sec - start_time;
		if (time_difference > 60*5 )
		{
			clock_gettime(CLOCK_REALTIME, &gettime_now);
			start_time = gettime_now.tv_sec;		//Get nS value
			//time_difference = 0;
			sprintf(buffer2, "IMALIVE:112");
			sendData(sockfd, buffer2);
			
		}
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
    			printf( "DEC Compare OK ! at:%d\n", time_difference);
                fflush(stdout);
    			/*
    				增加比對成功的處理碼在這邊
    				...			
    			*/
    		}
    		else
    		{
                printf( "DEC Compare NG ! at:%d\n", time_difference);
                fflush(stdout);
    		}
    		
        }
    }
    close( sockfd );
    return 0;
}
