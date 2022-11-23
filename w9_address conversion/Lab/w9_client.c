#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // usleep()

int main()
{
    int server;
    struct sockaddr_in ServAddr;
    char *servIP;
    int nbytes;

    int port = 10003;

    if ((server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("socket() failed");
        exit(1);
    }

    memset(&ServAddr, 0, sizeof(ServAddr));
    ServAddr.sin_family      = AF_INET;
    ServAddr.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr(servIP);
    ServAddr.sin_port        = htons(port);

    if(connect(server, (struct sockaddr*) &ServAddr, sizeof(ServAddr)) < 0)
    {
        perror("connect() failed");
        exit(1);
    }

    while(1) //wu qung hui quan
    {
      char arr[] = "123456";

      send(server, arr, strlen(arr), 0);

      usleep(1);
    }

    close(server);
    return 0;
}
