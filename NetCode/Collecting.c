/* 
 * Auth. : Bravo Bruce
 * Func. : Serve as a Controller in the E-Tag sensor net.
 * 			1. Maintain a heartbeat monitor
 * 			2. Listening for E-Tag message
 * 			3. Condition alert via the E-Tag message
*/

//Including libraries
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define _DEBUG TRUE
#ifdef _DEBUG
#define DEBUG(format, args...) printf("[%s:%d] "format, __FILE__, __LINE__, ##args)
#else
#define DEBUG(args...)
#endif

char buffer[256];

// Show error msg
void error( char *msg ) {
  perror(  msg );
  exit(1);
}

int func( int a ) {
   return 2 * a;
}

void sendData( int sockfd, int x ) {
  int n;

  char buffer[32];
  sprintf( buffer, "%d\n", x );
  if ( (n = write( sockfd, buffer, strlen(buffer) ) ) < 0 )
    error( const_cast<char *>( "ERROR writing to socket") );
  buffer[n] = '\0';
}

char* getData( int sockfd ) {
  int n;

  if ( (n = read(sockfd,buffer,255) ) < 0 )
    error( const_cast<char *>( "ERROR reading from socket") );
  buffer[n] = '\0';
  return buffer;
}

// Program entry point
int main(int argc, char *argv[]) {
	// Prepair variables for TCP Connection
    int sockfd, newsockfd, portno = 51717, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    char lbuffer[256];
    int n;
    int data;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
		error( const_cast<char *>("ERROR opening socket") );
    bzero((char *) &serv_addr, sizeof(serv_addr));

    DEBUG( "listening IP #%d\n", (char *)&serv_addr );
    DEBUG( "using port #%d\n", portno );

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons( portno );
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error( const_cast<char *>( "ERROR on binding" ) );
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
  
    //--- infinite wait on a connection ---
    while ( 1 ) {
		printf( "waiting for new client...\n" );
        if ( ( newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen) ) < 0 )
			error( const_cast<char *>("ERROR on accept") );
		printf( "opened new communication with client\n" );
        while ( 1 ) {
			//---- wait for a number from client ---
            sprintf(lbuffer,"%s",getData( newsockfd ));
            printf( "got %s\n", lbuffer );
            //if ( data < 0 ) 
			//	break;
            //data = func( data );
            //--- send new data back --- 
            //printf( "sending back %d\n", data );
            //sendData( newsockfd, data );
        }
        close( newsockfd );

        //--- if -2 sent by client, we can quit ---
        if ( data == -2 )
			break;
	}
    close(sockfd);
    //system("mpg321 /home/pi/car.mp3");
    return 0; 
}
