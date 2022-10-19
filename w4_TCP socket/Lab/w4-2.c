#include <stdio.h>      
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>     // usleep()

#define STRBUFSIZE 16384

int main(int argc, char *argv[])
{
    int sock;                        
    struct sockaddr_in echoServAddr; 
    unsigned short echoServPort;     
    char *servIP;                    
    char echoString[STRBUFSIZE];       
    char echoBuffer[STRBUFSIZE];        
    int bytesRcvd;                    

    if ( ( argc < 2 ) || (argc > 3 ) )    
    {
       fprintf( stderr, "Usage: %s <Server IP> [<Echo Port>]\n", argv[0] );
       exit(1);
    }

    servIP = argv[1];             

    if ( argc == 3 )
    { 
        echoServPort = atoi(argv[2]); 
    }
    else
    {
        echoServPort = 7;  
    } 

    if ( ( sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP ) ) < 0 )
    {
        perror("socket() failed");
        exit(1);
    }
    
    memset(&echoServAddr, 0, sizeof(echoServAddr));     
    echoServAddr.sin_family      = AF_INET;             
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);   
    echoServAddr.sin_port        = htons(echoServPort); 

    if ( connect( sock, (struct sockaddr *) &echoServAddr, sizeof( echoServAddr ) ) < 0 )
    {
        perror( "connect() failed" );
        exit(1);
    }
    
    if ( ( bytesRcvd = recv( sock, echoBuffer, STRBUFSIZE - 1, 0) ) <= 0 )
    {
        perror("recv() failed or connection closed prematurely");
        exit(1);
    }
    printf( "%s", echoBuffer);
    
    
    while (1) //wu qung hui quan
    {
        bzero( echoString, STRBUFSIZE );
        
        for(int i=0; i<16384; i++) //zhi zao wu yi yi de dong dong
	{
	    echoString[i] = 1;
	}
	usleep(15000);
        send(sock, echoString, 16384, 0);       
    }

    close(sock);
    return 0;
}
