#include <stdio.h>      // for printf() and fprintf() 
#include <sys/socket.h> // for socket(), connect(), send(), and recv() 
#include <arpa/inet.h>  // for sockaddr_in and inet_addr() 
#include <stdlib.h>     // for atoi() and exit() 
#include <string.h>     // for memset() 
#include <unistd.h>     // for close() 

#define STRBUFSIZE 100 // Size of receive buffer 

void DieWithError(char *errorMessage);  // Error handling function 

int main(int argc, char *argv[])
{
    int sock;                        // Socket descriptor 
    struct sockaddr_in echoServAddr; // Echo server address 
    unsigned short echoServPort;     // Echo server port 
    char *servIP;                    // Server IP address (dotted quad)
    char echoString[STRBUFSIZE];     // Buffer for echo string  
    char echoBuffer[STRBUFSIZE];     // Buffer for echo string 
    unsigned int echoStringLen;      // Length of string to echo 
    int bytesRcvd;   // Bytes read in single recv() and total bytes read 
    int num, end;

    if ( ( argc < 2 ) || (argc > 3 ) )    // Test for correct number of arguments 
    {
       fprintf( stderr, "Usage: %s <Server IP> [<Echo Port>]\n", argv[0] );
       exit(1);
    }

    servIP = argv[1];             // First arg: server IP address (dotted quad) 

    if ( argc == 3 )
    {
        // Use given port, if any 
        echoServPort = atoi(argv[2]); 
    }
    else
    {
        // 7 is the well-known port for the echo service
        echoServPort = 7;  
    } 

    // Create a reliable, stream socket using TCP 
    if ( ( sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP ) ) < 0 )
    {
        perror("socket() failed");
        exit(1);
    }
    
    // Construct the server address structure 
    memset(&echoServAddr, 0, sizeof(echoServAddr));     // Zero out structure 
    echoServAddr.sin_family      = AF_INET;             // Internet address family 
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);   // Server IP address 
    echoServAddr.sin_port        = htons(echoServPort); // Server port 

    // Establish the connection to the echo server 
    if ( connect( sock, (struct sockaddr *) &echoServAddr, sizeof( echoServAddr ) ) < 0 )
    {
        perror( "connect() failed" );
        exit(1);
    }
    
    //shu chu
    if ( ( bytesRcvd = recv( sock, echoBuffer, STRBUFSIZE - 1, 0) ) <= 0 )
    {
            perror("recv() failed or connection closed prematurely");
            exit(1);
    }
    printf( "%s", echoBuffer);
    
    //shu ru: GO
    fgets( echoString, STRBUFSIZE, stdin ); 
    echoStringLen = strlen( echoString );
    send(sock, echoString, echoStringLen, 0);
    
    //ji suan you ji ge byte
    num=0;
    end=0;
    while ( end<1 )
    {
        bzero( echoString, STRBUFSIZE );
        bzero( echoBuffer, STRBUFSIZE );
        
        if ( ( bytesRcvd = recv( sock, echoBuffer, STRBUFSIZE - 1, 0) ) <= 0 )
        {
            perror("recv() failed or connection closed prematurely");
            exit(1);
        }
        printf( "%s", echoBuffer); 
        
        for(int i=0; i<100; i++)
        {
        	if(echoBuffer[i] == '?') //du dao wen hao shi jie shu
        	{
        		end=1;
        	}
        }
        num = num + bytesRcvd;
    }
    num = num - 86;
    
    //do an
    printf("ENDENDENDEND: %d\n", num);
    
    //shu ru
    fgets( echoString, STRBUFSIZE, stdin ); 
    echoStringLen = strlen(echoString);
    send(sock, echoString, echoStringLen, 0);
    
    //shu chu gui ling
    for(int i=0; i<100; i++)
    {
    	echoBuffer[i] = '\0';
    }
    
    //shu chu
    if ( ( bytesRcvd = recv( sock, echoBuffer, STRBUFSIZE - 1, 0) ) <= 0 )
    {
    	    perror("recv() failed or connection closed prematurely");
            exit(1);
    }
    printf( "%s", echoBuffer);
    
    printf("\n");    // Print a final linefeed 

    close(sock);
    return 0;
}
