#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h> //sockaddr_in and inet_ntoa()
#include <fcntl.h>     //access()

#define BUFFER_SIZE 1024
#define on_error(...) { fprintf(stderr, __VA_ARGS__); fflush(stderr); exit(1); }

int     AcceptTCPConnection( int );

int main (int argc, char *argv[])
{
  if (argc < 2) on_error("Usage: %s [port]\n", argv[0]);

  int port = atoi(argv[1]);

  int server_fd, client_fd, err;
  struct sockaddr_in server, client;
  char buf[BUFFER_SIZE];

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) on_error("Could not create socket\n");

  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

  err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
  if (err < 0) on_error("Could not bind socket\n");

  err = listen(server_fd, 128);
  if (err < 0) on_error("Could not listen on socket\n");
  
  socklen_t client_len = sizeof(client);
  client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);
  
  
  
  int compare = 1;
  compare = strcmp("/usr/bin/date", argv[2]);
  if(compare == 0)
  {
	printf("New connection from %s:%d\n", inet_ntoa( client.sin_addr ), port);
	return 0;
  }
  compare = strcmp("date", argv[2]);
  if(compare == 0)
  {
	time_t now; 
	time(&now); //qu de xian zai shi jian
	send(client_fd, ctime(&now), 24, 0);
	return 0;
  }
  compare = strcmp("ls", argv[2]);
  if(compare == 0)
  {
  	int i=2;
  	char *arr[100];
  	while(argv[i] != NULL)
  	{
  		arr[i-2] = argv[i];
  		i++;
  	}
  	char *envp[] = {"PATH=/bin", NULL};
  	execve("/bin/ls", arr, envp);
  	//printf(" ");
  	return 0;
  }
  if((access(argv[2],F_OK))==-1 && compare!=0)   
  {   
        printf("%s is not exist!\n", argv[2]);
        return 0;
  }
}
