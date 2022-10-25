#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h> //access()

int main (int argc, char *argv[])
{
  fd_set master; //master file descriptor qing dan
  fd_set read_fds; //gei select() yong de zhan shi de file descriptor qing dan
  int fdmax; //zuei da de file descriptor shu mu
  int listener; //listening socket descriptor
  int newfd; //xin jie shou de accept() socket descriptor
  struct sockaddr_in client;
  socklen_t addrlen;
  int nbytes;
  int yes=1; //di xia de setsockopt() she ding SO_REUSEADDR
  
  int i, j, rv;
  int port = atoi(argv[1]);

  struct addrinfo hints, *ai, *p;

  FD_ZERO(&master); //qing chu master and temp sets
  FD_ZERO(&read_fds);

  //bind a socker
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(NULL, argv[1], &hints, &ai)) != 0) 
  {
    fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
    exit(1);
  }

  for(p = ai; p != NULL; p = p->ai_next) 
  {
    listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener < 0) 
    {
      continue;
    }

    //bi kai cuo wu xuen xi: "address already in use"
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) 
    {
      close(listener);
      continue;
    }

    break;
  }

  //bind() shi bai
  if (p == NULL) 
  {
    fprintf(stderr, "selectserver: failed to bind\n");
    exit(2);
  }
  freeaddrinfo(ai); // all done with this

  //listen
  if (listen(listener, 10) == -1) 
  {
    perror("listen");
    exit(3);
  }

  //jiang listener xin zueng dao master set
  FD_SET(listener, &master);

  //chi xu zhuei zong zuei da de file descriptor
  fdmax = listener;

  int temp;
  char *output;
  int clientNum = 0;
  int clientName[20] = {};
  int clientRename[100][20] = {};
  char ipArr[100][16] = {};
  unsigned portArr[100][1] = {};
  
  while(1)
  {
    read_fds = master; //fu zhi master

    if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) 
    {
      perror("select");
      exit(4);
    }

    //zai xian cuen de lian xian zhong xuen zhao xu yao du qu de zi liao
    for(i=0; i<=fdmax; i++) 
    {
      if(FD_ISSET(i, &read_fds)) //ru guo you zhao dao
      {
        if(i == listener) //ru guo shi xin de lian xian
        {
          addrlen = sizeof client;
          newfd = accept(listener, (struct sockaddr *)&client, &addrlen);

          if (newfd == -1) 
          {
            perror("accept");
          }else 
          {
            FD_SET(newfd, &master); //xin zeng dao master set
            if (newfd > fdmax) 
            {
              fdmax = newfd; //chi xu zhuei zong zuei da de
            }
            
            //song chu huan ying xuen xi
            char welOne[] = "Welcome to the simple CHAT server\n";
            send(newfd, welOne, strlen(welOne), 0);
            char welTwo[10] = {};
            sprintf(welTwo, "%d", clientNum); //shu zi zhuan zi yuan
            send(newfd, welTwo, strlen(welTwo), 0);
            char welThree[] = " users online now. Your name is USER";
            send(newfd, welThree, strlen(welThree), 0);
            srand( time(NULL) ); //she ding luan shu zhong zi
            int randNum = rand(); //chan sheng luan shu
	    char welFour[10] = {};
            sprintf(welFour, "%d", randNum); //shu zi zhuan zi yuan
            send(newfd, welFour, strlen(welFour), 0);
            send(newfd, "\n", 1, 0);
  	    
            temp=newfd;
            clientNum++; //mu qian you ji ren zai xian shang
            
            //chu cuen ming zi
            clientRename[temp][0] = 'U';
            clientRename[temp][1] = 'S';
            clientRename[temp][2] = 'E';
            clientRename[temp][3] = 'R';
            clientName[temp] = randNum;
            
            //chu cuen ip wei zhi and port number
            char *ipArrTemp = inet_ntoa(client.sin_addr);
            for(int k=0; k<strlen(ipArrTemp); k++)
            {
                ipArr[temp][k] = ipArrTemp[k];
            }
            portArr[temp][0] = ntohs(client.sin_port);
            
            //server duan xian shi lian xian
            printf("client connected from %s:%u\n", ipArr[temp], portArr[temp][0]);
            
            //song chu xuen xi gei qi ta ren
            for(j = 0; j <= fdmax; j++)
            {
              if (FD_ISSET(j, &master)) //zhao dao yi you de lian xian
              {
                if (j != listener && j != newfd) //gai lian xian bu shi listener(server), ye bu shi zi ji
               	{
               	  send(j, "USER", 4, 0);
                  send(j, welFour, strlen(welFour), 0);
               	  char connect[] = " has just landed on the server\n"; 
                  send(j, connect, strlen(connect), 0);
                }
              }
            }
          }
        }else //yi you lian xian de client chuan song xuen xi
        {
          char buf[1024] = {};
          nbytes = recv(i, buf, sizeof buf, 0);
          if(nbytes <= 0) //jie shou xuen xi
          {
            if(nbytes == 0) //"^c", jie shu lian xian
            {
                //gei qi ta ren song xuen xi (shei shei shei li kai lian xian
                for(j = 0; j <= fdmax; j++)
		{
              	    if (FD_ISSET(j, &master))
              	    {
		        if (j != listener && j != i) 
		       	{
		       	    if(clientName[i] != 0) //li kai de ren mei you gai ming
            	    	    {
            	    	    	send(j, "USER", 4, 0);
            	    	    	char leaveOne[10] = {};
            	    	    	sprintf(leaveOne, "%d", clientName[i]);
            	    	    	send(j, "USER", 4, 0);
            	    	    	send(j, leaveOne, strlen(leaveOne), 0);
            	    	    }else //li kai de ren you gai ming
            	    	    {
            	    	        char leaveTwo[20] = {};
            	    	        for(int k=0; k<20; k++)
            	    	        {
            	    	             leaveTwo[k] = clientRename[i][k];
            	    	        }
            	    	    	send(j, leaveTwo, strlen(leaveTwo), 0);
            	    	    }
		       	    char leaveThree[] = " has left the server";
		            send(j, leaveThree, strlen(leaveThree), 0);
		            send(j, "\n", 1, 0);
		        }
		     }
		 }
		 //server duan xian shi li kai lian xian
		 printf("client %s:%u disconnected\n", ipArr[i], portArr[i][0]);
            }else
            {
              perror("recv");
            }
            close(i); //jie shu gai client
            clientNum--; //xian shang client shu jian shao
            FD_CLR(i, &master); //cong  master set zhong yi chu
          }else //bytes > 0, you song shu xuen xi
          {
            char commandOne = buf[0];
            char commandTwo = '/';
            if(commandOne == commandTwo) //ruo di yi ge zi yuan shi '/', ze wei command
            {
            	int compareOne = 1;
            	compareOne = strcmp("/name\n", buf); // '/name', gai ming zi
            	if(compareOne == 0)
            	{
            	    //jiang jiu de ming zi cuen qi lai
            	    char oldNameOne[20] = {};
            	    if(clientName[i] == 0)
            	    {
            	    	for(int k=0; k<20; k++)
            	    	{
            	    	    oldNameOne[k] = clientRename[i][k];
            	    	}
            	    }
            	    
            	    //song chu gai ming guei ze
            	    char renameOne[] = "Please enter a new nickname (less than 20 characters):\n";
            	    send(i, renameOne, strlen(renameOne), 0);
            	    
            	    //jie shou xin de ming zi
            	    char renameTwo[20] = {};
            	    recv(i, renameTwo, sizeof renameTwo, 0);
            	    
            	    //chu cuen xin de ming zi
            	    for(int k=0; k<strlen(renameTwo)-1; k++) //jian yi shi yin wei zuei hou yi ge shi '\n'
            	    {
            	    	clientRename[i][k] = renameTwo[k];
            	    }
            	    
            	    //song chu gai ming cheng gong
            	    char renameThree[] = "Nickname changed to ";
            	    send(i, renameThree, strlen(renameThree), 0);
            	    send(i, renameTwo, strlen(renameTwo), 0);
            	    
            	    //gei qi ta ren song chu you ren gai ming de xuen xi
            	    for(j=0; j<=fdmax; j++)
            	    {
            	    	if(FD_ISSET(j, &master))
            	    	{
            	    	    if(j!=listener && j!=i)
            	    	    {
            	    	    	if(clientName[i] != 0) //yuan ben de ming zi shi luan ma
            	    	    	{
            	    	    	    send(j, "USER", 4, 0);
            	    	    	    char oldNameTwo[10] = {};
            	    	    	    sprintf(oldNameTwo, "%d", clientName[i]);
            	    	    	    send(j, oldNameTwo, strlen(oldNameTwo), 0);
            	    	    	    send(j, " rename to ", 11, 0);
            	    	    	    send(j, renameTwo, strlen(renameTwo), 0);
            	    	    	}else //yuan ben de ming zi bu shi luan ma
            	    	    	{
            	    	    	    send(j, oldNameOne, strlen(oldNameOne), 0);
            	    	    	    send(j, " rename to ", 11, 0);
            	    	    	    send(j, renameTwo, strlen(renameTwo), 0);
            	    	    	    send(j, "\n", 1, 0);
            	    	    	}
            	    	    }
            	    	}
            	    }
            	    //jiang luan ma qu xiao
            	    clientName[i] = 0;
            	}
            	
            	int compareTwo = 1;
            	compareTwo = strcmp("/who\n", buf); // '/who', show chu xian shang suo you de ming zi
            	if(compareTwo == 0)
            	{
            	    send(i, "-----------------------------------------\n", 43, 0);
            	    //son chu xuen xi, you shei zai xian shang
            	    for(j=0; j<=fdmax; j++)
            	    {
		      	if (FD_ISSET(j, &master))
		      	{
			    if (j != listener && j != i) //bu shi zi ji
			    {
			    	  send(i, "   ", 3, 0);
			    	  //song chu chu ming zi
			    	  char whoName[20] = {};
			    	  for(int k=0; k<20; k++)
			    	  {
			    	  	whoName[k] = clientRename[j][k];
			    	  }
			    	  send(i, whoName, strlen(whoName), 0);
			    	  
			    	  //ru guo mei gai ming de hua, hou mian huai shi luan ma
			    	  char who[10] = {};
			    	  if(clientName[j] != 0)
			    	  {
			       	 	sprintf(who, "%d", clientName[j]); //shu zi zhuan zi yuan
				  	send(i, who, strlen(who), 0);
			    	  }
			    	  
			    	  //song chu kong ge (pai ban yong)
			    	  int remain = 25-3-strlen(whoName)-strlen(who);
			    	  for(int k=0; k<remain; k++)
			    	  {
			    	        send(i, " ", 1, 0);
			    	  }
			    	  
			    	  //song chu gai yong hu de ip wei zhi and port number
			    	  send(i, ipArr[j], strlen(ipArr[j]), 0);
			    	  send(i, ":", 1, 0);
			    	  char portArrChar[16] = {};
			    	  sprintf(portArrChar, "%u", portArr[j][0]);
			    	  send(i, portArrChar, strlen(portArrChar), 0);
				  send(i, "\n", 1, 0);
				  
			     }else if(j == i) //shi zi ji de hua
			     {
			     	  send(i, " * ", 3, 0); //qian mian jia xing hao
			     	  
			    	  char whoName[20] = {};
			    	  for(int k=0; k<20; k++)
			    	  {
			    	  	whoName[k] = clientRename[j][k];
			    	  }
			    	  send(i, whoName, strlen(whoName), 0);
			    	  
			    	  char who[10] = {};
			    	  if(clientName[j] != 0)
			    	  {
			       	 	sprintf(who, "%d", clientName[j]);
				  	send(i, who, strlen(who), 0);
			    	  }
			    	  
			    	  int remain = 25-3-strlen(whoName)-strlen(who);
			    	  for(int k=0; k<remain; k++)
			    	  {
			    	        send(i, " ", 1, 0);
			    	  }
			    	  
			    	  send(i, ipArr[j], strlen(ipArr[j]), 0);
			    	  send(i, ":", 1, 0);
			    	  char portArrChar[16] = {};
			    	  sprintf(portArrChar, "%u", portArr[j][0]);
			    	  send(i, portArrChar, strlen(portArrChar), 0);
				  send(i, "\n", 1, 0);
			     }
			 }
		     }
		     send(i, "-----------------------------------------\n", 43, 0);
            	}
            	if(compareOne!=0 && compareTwo!=0) //cha wu ci command
            	{
            	    char noCommand[] = "Unknown or incomplete command ";
            	    send(i, noCommand, strlen(noCommand), 0);
            	    send(i, buf, strlen(buf), 0);
            	}
            	
            }else //ru gou bu wei command, (chuan xuen xi)
            {
                //dan chuen an enter jian, bu song chu xuen xi
            	int compare = strcmp("\n", buf);
            	if(compare == 0)
            	{
            	    break;
            	}
            	
            	//jiang xuen xi song gei qi ta client
		for(j = 0; j <= fdmax; j++)
		{
		    if (FD_ISSET(j, &master))
		    {
		        if (j != listener && j != i) 
		       	{
		       	    //song chu de xuen xi qian mian jia yong hu ming
		       	    if(clientName[i] != 0) //ru guo mei gai guo ming zi
            	    	    {
            	    	    	send(j, "USER", 4, 0);
            	    	    	char messageNameOne[10] = {};
            	    	    	sprintf(messageNameOne, "%d", clientName[i]);
            	    	    	send(j, messageNameOne, strlen(messageNameOne), 0);
            	    	    	send(j, ": ", 2, 0);
            	    	    }else //you gai guo ming zi
            	    	    {
            	    	        char messageNameTwo[20] = {};
            	    	        for(int k=0; k<20; k++)
            	    	        {
            	    	             messageNameTwo[k] = clientRename[i][k];
            	    	        }
            	    	    	send(j, messageNameTwo, strlen(messageNameTwo), 0);
            	    	    	send(j, ": ", 2, 0);
            	    	    }
            	    	    
            	    	    //song chu xuen xi
		            send(j, buf, nbytes, 0);
		        }
		    }
		}
	    }
          }
        }
      }
    }
  }

  return 0;
}
