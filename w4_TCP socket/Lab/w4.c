#include <stdio.h>      //printf(), fprintf(), sprintf()
#include <sys/socket.h> //socket(), connect(), send(), recv() 
#include <arpa/inet.h>  //sockaddr_in, inet_addr() 
#include <stdlib.h>     //atoi(), exit() 
#include <string.h>     //memset() 
#include <unistd.h>     //close() 

#define STRBUFSIZE 100 //yi chi du ji ge byte 

int main(int argc, char *argv[])
{
    int sock;                        //socket de ming zi(jian li de na ge lian xian) 
    struct sockaddr_in echoServAddr; //server zhi liao de zhi liao jie gou 
    unsigned short echoServPort;     //port number 
    char *servIP;                    //IP wei zhi 
    char echoBuffer[STRBUFSIZE];     //du dao de zi chuan zhan shi de cuen fang di
    int bytesRcvd;   //recv() du dao ji ge byte 
    int num, end;

    if ( ( argc < 2 ) || (argc > 3 ) )    //jian cha shu ru shi fou zheng que
    {
       fprintf( stderr, "Usage: %s <Server IP> [<Echo Port>]\n", argv[0] );
       exit(1);
    }

    servIP = argv[1];             //di yi ge: IP wei zhi
    if ( argc == 3 )
    {
        //di er ge: port number
        echoServPort = atoi(argv[2]); //zi chuan zhuan shu zi
    }
    else
    {
        //port number 7: ju shuo hen you ming de the echo service port (?)
        echoServPort = 7;  
    } 

    //jian li xin de lian xian: reliable, stream socket, TCP 
    if ( ( sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP ) ) < 0 )
    {
        perror("socket() failed");
        exit(1);
    }
    
    //shu ru server de zi liao
    memset(&echoServAddr, 0, sizeof(echoServAddr));     //gui ling 
    echoServAddr.sin_family      = AF_INET;             //IPv4
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);   //IP wei zhi: shou dong shu ru di yi ge
    echoServAddr.sin_port        = htons(echoServPort); //port number: shou dong shu ru di er ge

    //lian jie dao server
    if ( connect( sock, (struct sockaddr *) &echoServAddr, sizeof( echoServAddr ) ) < 0 )
    {
        perror( "connect() failed" );
        exit(1);
    }
    
    //shu chu: zui zui kai shi
    if ( ( bytesRcvd = recv( sock, echoBuffer, STRBUFSIZE - 1, 0) ) <= 0 )
    {
            perror("recv() failed or connection closed prematurely");
            exit(1);
    }
    printf( "%s", echoBuffer);
    
    //shu ru: GO
    char go[4];
    go[0]='G';
    go[1]='O';
    go[2]='\n';
    send(sock, go, 3, 0);
    
    //ji suan you ji ge byte
    num=0;
    end=0;
    while ( end<1 )
    {
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
    num = num - 86; //kou diao kai tou han jie wei
    
    //da an
    printf("%d\n", num);
    
    //shu ru: da an
    char answer[10];
    int length;
    sprintf(answer, "%d", num);
    length = strlen(answer);
    send(sock, answer, length, 0);
    //shu ru: enter
    char enter[1];
    enter[0]='\n';
    send(sock, enter, 1, 0);
    
    //shu chu gui ling
    for(int i=0; i<100; i++)
    {
    	echoBuffer[i] = '\0';
    }
    
    //shu chu: jie guo
    if ( ( bytesRcvd = recv( sock, echoBuffer, STRBUFSIZE - 1, 0) ) <= 0 )
    {
    	    perror("recv() failed or connection closed prematurely");
            exit(1);
    }
    printf( "%s", echoBuffer);    

    //jie shu lian xian
    close(sock);
    return 0;
}
