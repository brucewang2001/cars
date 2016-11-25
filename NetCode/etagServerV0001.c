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
// include for mysql 
#include <my_global.h>
#include <mysql.h>
#include <ctype.h>

#define _DEBUG TRUE
#ifdef _DEBUG
#define DEBUG(format, args...) printf("[%s:%d] "format, __FILE__, __LINE__, ##args)
#else
#define DEBUG(args...)
#endif

char buffer[256];

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);        
}

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

// Note: This function returns a pointer to a substring of the original string.
// If the given string was allocated dynamically, the caller must not overwrite
// that pointer with the returned value, since the original pointer must be
// deallocated using the same allocator with which it was allocated.  The return
// value must NOT be deallocated using free() etc.
char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}


// Program entry point
int main(int argc, char *argv[]) {
	
	// Prepair variables for TCP Connection
    int sockfd, newsockfd, portno = 51717, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    char lbuffer[256];
    int n;
    int data;
    
    // Prepair variables and connection for DB
    MYSQL *con = mysql_init(NULL); 
	if (con == NULL)
	{
		fprintf(stderr, "mysql_init() failed\n");
		exit(1);
	}    
	if (mysql_real_connect(con, "localhost", "root", "Wky10021", 
          "cars", 0, NULL, 0) == NULL) 
	{
		finish_with_error(con);
	}  
    
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
            // sprintf(lbuffer,"%s",);
            char * tmp = getData(newsockfd);
            sprintf(lbuffer, "INSERT INTO collects (nums) VALUES('%s')", tmp);
            if(!tmp[0] == '\0')
            {
				if(!strcmp(tmp, "IMALIVE:112"))
				{
					//printf("Client 112 is still alive!!!\n");
					printf("%s\n",tmp);
				}
				else
				{				
					printf( "%s\n", lbuffer );
					if (mysql_query(con, lbuffer)) 
					{
						finish_with_error(con);
					}

					tmp = trimwhitespace(tmp);
					printf("%s\n", tmp);					
					sprintf(lbuffer, "SELECT * FROM cars WHERE nums='%s'",tmp);
					if (mysql_query(con, lbuffer)) 
					{
						finish_with_error(con);
					}
					MYSQL_RES *result = mysql_store_result(con); 
					if (result == NULL) 
					{
						finish_with_error(con);
					}
					if (mysql_fetch_row(result) > 0)
						system("mpg321 /home/pi/incar.mp3");
				}
			}  
			else
				break;
            //if ( data < 0 ) 
			//	break;
            //data = func( data );
            //--- send new data back
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
