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

  int rv;
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

  int temp, compare;
  int i, j, k, m;

  int clientNum = 0;
  char clientName[20][20] = {};
  char ipArr[20][16] = {};

  int channelNum = 0;
  int channelUser[20] = {};
  char channelName[20][20] = {};
  char topicName[20][20] = {};

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

            char *ipArrTemp = inet_ntoa(client.sin_addr);
            for(int k=0; k<strlen(ipArrTemp); k++)
            {
                ipArr[newfd][k] = ipArrTemp[k];
            }

	          clientNum++;
          }

          break;
        }else //yi you lian xian de client chuan song xuen xi
        {
            char buf[1024] = {};
            nbytes = recv(i, buf, sizeof buf, 0);
            if(nbytes > 1)
            {
                printf("receive message: %s", buf);

                char commandFour[5] = {};
                char commandFive[6] = {};
                char commandSeven[8] = {};
                for(k=0; k<4; k++)
                {
                    commandFour[k] = buf[k];
                    commandFive[k] = buf[k];
                    commandSeven[k] = buf[k];
                }
                commandFour[4] = '\0';
                commandFive[4] = buf[4];
                commandFive[5] = '\0';
                for(k=4; k<7; k++)
                {
                    commandSeven[k] = buf[k];
                }
                commandSeven[7] = '\0';

                compare = strcmp("NICK", commandFour); //shou dao "NICK", jian li yong hu ming
                if(compare == 0)
                {
                  if(buf[5] == 0)
                  {
                    char errorOne[] = ":mid 431 :No nickname given\n\0";

                    send(i, errorOne, strlen(errorOne), 0);
                  }else
                  {
                    char tempName[20] = {};
                    for(k=0; k<20; k++)
                    {
                        tempName[k] = buf[5+k];
                    }
                    temp = strlen(tempName)-1;
                    tempName[temp] = '\0';

                    for(j=0; j<=fdmax; j++)
                    {
                      if (FD_ISSET(j, &master))
                      {
                         if (j!=listener) //bu shi zi ji
                         {
                            temp = strcmp(tempName, clientName[j]);
                            if(temp == 0)
                            {
                              char combind[100] = {};
                              char errorOne[] = ":mid 436 \0";
                              char errorTwo[] = " :Nickname collision KILL\n\0";

                              strcat(combind, errorOne);
                              strcat(combind, tempName);
                              strcat(combind, errorTwo);

                              send(i, combind, strlen(combind), 0);
                            }
                         }
                       }
                     }

                     for(k=0; k<20; k++)
                     {
                         clientName[i][k] = tempName[k];
                     }
                  }
                  break;
                }

                compare = strcmp("USERS", commandFive); //shou dao "USERS"
                if(compare == 0)
                {
                    char usersOneOne[] = ":mid 392 \0";
                    char usersOneTwo[] = " :UserID                   Terminal  Host\n\0";
                    char combindOne[100] = {};
                    strcat(combindOne, usersOneOne);
                    strcat(combindOne, clientName[i]);
                    strcat(combindOne, usersOneTwo);
                    send(i, combindOne, strlen(combindOne), 0);

                    for(j=0; j<=fdmax; j++)
            	      {
          		      	if (FD_ISSET(j, &master))
          		      	{
          			      if (j != listener) //bu shi zi ji
          			      {
          			        char combindTwo [100] = {};
          			        char usersTwoOne[] = ":mid 393 \0";
          			        char usersTwoTwo[] = " :\0";
          			        strcat(combindTwo, usersTwoOne);
          			        strcat(combindTwo, clientName[i]);
          			        strcat(combindTwo, usersTwoTwo);
          			        strcat(combindTwo, clientName[j]);
          			        char usersTwoThree[] = " \0";
          			    	  int remain = 25 - strlen(clientName[j]);
          			      	for(k=0; k<remain; k++)
          			      	{
          			      	    strcat(combindTwo, usersTwoThree);
          			      	}
          			      	char usersTwoFour[] = "-         \0";
          			      	strcat(combindTwo, usersTwoFour);
          			      	strcat(combindTwo, ipArr[j]);
          			      	char usersTwoFive[] = "\n";
          			      	strcat(combindTwo, usersTwoFive);
          			      	send(i, combindTwo, strlen(combindTwo), 0);
          			      }
			              }
		              }

          		    char usersThreeOne[] = ":mid 394 \0";
          		    char usersThreeTwo[] = " :End of users\n\0";
          		    char combindThree[100] = {};
          		    strcat(combindThree, usersThreeOne);
          		    strcat(combindThree, clientName[i]);
                  strcat(combindThree, usersThreeTwo);
                  send(i, combindThree, strlen(combindThree), 0);

                  break;
                }

                compare = strcmp("USER", commandFour); //shou dao "USER", chuan song huan ying xuen xi
                if(compare == 0)
                {
                    if(buf[4] != ' ')
                    {
                      buf[nbytes-1] = '\0';

                      char combind[100] = {};
                      char errorOne[] = ":mid 461 \0";
                      char errorTwo[] = " \0";
                      char errorThree[] = " :Not enought parameters\n\0";

                      strcat(combind, errorOne);
                      strcat(combind, clientName[i]);
                      strcat(combind, errorTwo);
                      strcat(combind, buf);
                      strcat(combind, errorThree);

                      send(i, combind, strlen(combind), 0);
                    }else
                    {
                      char welcomeOneOne[] = ":mid 001 \0";
                      char welcomeOneTwo[] = " :Welcome to the mid project chat room\n\0";
                      char combindOne[100] = {};
                      strcat(combindOne, welcomeOneOne);
                      strcat(combindOne, clientName[i]);
                      strcat(combindOne, welcomeOneTwo);
                      send(i, combindOne, strlen(combindOne), 0);

                      char welcomeTwoOne[] = ":mid 251 \0";
                      char welcomeTwoTwo[] = " :There are \0";
                      char welcomeTwoThree[2] = {};
              	      sprintf(welcomeTwoThree, "%d", clientNum);
              	      char welcomeTwoFour[] = " users online now\n\0";
                      char combindTwo[100] = {};
                      strcat(combindTwo, welcomeTwoOne);
                      strcat(combindTwo, clientName[i]);
                      strcat(combindTwo, welcomeTwoTwo);
                      strcat(combindTwo, welcomeTwoThree);
                      strcat(combindTwo, welcomeTwoFour);
                      send(i, combindTwo, strlen(combindTwo), 0);

                      char welcomeThreeOne[] = ":mid 375 \0";
                      char welcomeThreeTwo[] = " :- mid Message of the day -\n\0";
                      char combindThree[100] = {};
                      strcat(combindThree, welcomeThreeOne);
                      strcat(combindThree, clientName[i]);
                      strcat(combindThree, welcomeThreeTwo);
                      send(i, combindThree, strlen(combindThree), 0);

                      char welcomeFourOne[] = ":mid 372 \0";
                      char welcomeFourTwo[] = " :-  Hello, World!\n\0";
                      char combindFour[100] = {};
                      strcat(combindFour, welcomeFourOne);
                      strcat(combindFour, clientName[i]);
                      strcat(combindFour, welcomeFourTwo);
                      send(i, combindFour, strlen(combindFour), 0);

                      char welcomeFiveOne[] = ":mid 376 \0";
                      char welcomeFiveTwo[] = " :End of message of the day\n\0";
                      char combindFive[100] = {};
                      strcat(combindFive, welcomeFiveOne);
                      strcat(combindFive, clientName[i]);
                      strcat(combindFive, welcomeFiveTwo);
                      send(i, combindFive, strlen(combindFive), 0);
                    }
                    break;
                }

                compare = strcmp("QUIT", commandFour); //shou dao "QUIT"
                if(compare == 0)
                {
                    close(i);
                    clientNum--;
                    FD_CLR(i, &master);

                    break;
                }

                compare = strcmp("LIST", commandFour); //shou dao "LIST", lie chu channel lie biao
                if(compare == 0)
                {
                    char listOneOne[] = ":mid 321 \0";
                    char listOneTwo[] = " :Channel  Users  Name\n\0";
                    char combindOne[100] = {};
                    strcat(combindOne, listOneOne);
                    strcat(combindOne, clientName[i]);
                    strcat(combindOne, listOneTwo);
                    send(i, combindOne, strlen(combindOne), 0);

                    for(k=0; k<channelNum; k++)
                    {
                        char combindTwo[100] = {};
                        char listTwoOne[] = ":mid 322 \0";
                        char listTwoTwo[] = " #";
                        char listTwoThree[] = " \0";
                        char listTwoFour[2] = {};
                        sprintf(listTwoFour, "%d", channelUser[k]);
                        char listTwoFive[] = " :\0";

                        strcat(combindTwo, listTwoOne);
                        strcat(combindTwo, clientName[i]);
                        strcat(combindTwo, listTwoTwo);
                        strcat(combindTwo, channelName[k]);
                        strcat(combindTwo, listTwoThree);
                        strcat(combindTwo, listTwoFour);
                        strcat(combindTwo, listTwoFive);

                        if(topicName[k][0] != 0)
                        {
                            strcat(combindTwo, topicName[k]);
                        }

                        char listTwoSix[] = "\n\0";
                        strcat(combindTwo, listTwoSix);

                        send(i, combindTwo, strlen(combindTwo), 0);
                    }

                    char listThreeOne[] = ":mid 323 \0";
                    char listThreeTwo[] = " :End of /LIST\n\0";
                    char combindThree[100] = {};
                    strcat(combindThree, listThreeOne);
                    strcat(combindThree, clientName[i]);
                    strcat(combindThree, listThreeTwo);
                    send(i, combindThree, strlen(combindThree), 0);

                    break;
                }

                compare = strcmp("JOIN", commandFour); //shou dao "JOIN", jia ru channel, gou channel bu cuen zai, jiu chuang li yi ge xin de
                if(compare == 0)
                {
                    char channel[20] = {};
                    for(k=0; k<20; k++)
                    {
                        channel[k] = buf[6+k];
                    }
                    temp = strlen(channel) - 1;
                    channel[temp] = '\0';

                    char combindOne[100] = {};
                    char joinOneOne[] = ":\0";
                    char joinOneTwo[] = " JOIN #\0";
                    char joinOneThree[] = "\n\0";

                    strcat(combindOne, joinOneOne);
                    strcat(combindOne, clientName[i]);
                    strcat(combindOne, joinOneTwo);
                    strcat(combindOne, channel);
                    strcat(combindOne, joinOneThree);

                    send(i, combindOne, strlen(combindOne), 0);

                    for(k=0; k<channelNum; k++)
                    {
                        compare = strcmp(channel, channelName[k]);
                        if(compare == 0)
                        {
                            break;
                        }
                    }
                    if(k == channelNum) //chuang li xin de channel
                    {
                        for(m=0; m<20; m++)
                        {
                            channelName[k][m] = channel[m];
                        }
                        channelNum++;
                    }
                    channelUser[k]++;

                    break;
                }

                compare = strcmp("PART", commandFour); //shou dao "PART", li kai channel
                if(compare == 0)
                {
                    char channel[20] = {};
                    for(k=0; k<20; k++)
                    {
                        if(buf[6+k] == ':')
                        {
                           break;
                        }
                        channel[k] = buf[6+k];
                    }
                    temp = strlen(channel) - 1;
                    channel[temp] = '\0';

                    for(k=0; k<channelNum; k++)
                    {
                        compare = strcmp(channel, channelName[k]);
                        if(compare == 0)
                        {
                            break;
                        }
                    }
                    if(k == channelNum) //chuang li xin de channel
                    {
                       char combind[100] = {};
                       char errorOne[] = ":mid 403 \0";
                       char errorTwo[] = " #\0";
                       char errorThree[] = " :No such channel\n\0";

                       strcat(combind, errorOne);
                       strcat(combind, clientName[i]);
                       strcat(combind, errorTwo);
                       strcat(combind, channel);
                       strcat(combind, errorThree);

                       send(i, combind, strlen(combind), 0);
                    }else
                    {
                      char combind[100] = {};
                      char partOne[] = ":\0";
                      char partTwo[] = " PART :#\0";
                      char partThree[] = "\n\0";

                      strcat(combind, partOne);
                      strcat(combind, clientName[i]);
                      strcat(combind, partTwo);
                      strcat(combind, channel);
                      strcat(combind, partThree);

                      send(i, combind, strlen(combind), 0);

                      for(k=0; k<channelNum; k++) //ji suan zai channel li de ren shu
                      {
                          compare = strcmp(channel, channelName[k]);
                          if(compare == 0)
                          {
                              break;
                          }
                      }
                      channelUser[k]--;
                    }
                    break;
                }

                compare = strcmp("TOPIC", commandFive); //shou dao "TOPIC", wei channel she topic
                if(compare == 0)
                {
                    char channel[20] = {};
                    int setTopic = 0;
                    for(k=0; k<20; k++) //kan zai na yi ge channel li
                    {
                        if(buf[7+k] == ':')
                        {
                    	     setTopic = 1;
                           break;
                        }
                        channel[k] = buf[7+k];
                    }
                    if(setTopic == 1)
                    {
                        temp = strlen(channel) - 1;
                    	  channel[temp] = '\0';

                        temp = 8 + k;

                        for(k=0; k<20; k++) //zhao dao dai ying de channel
                        {
                            compare = strcmp(channel, channelName[k]);
                            if(compare == 0)
                            {
                                break;
                            }
                        }

                        for(m=0; m<20; m++)
                        {
                            topicName[k][m] = buf[temp + m];
                        }
                        temp = strlen(topicName[k]) - 1;
                        topicName[k][temp] = '\0';

                        char combind[100] = {};
                        char topicOne[] = ":mid 332 \0";
                        char topicTwo[] = " #\0";
                        char topicThree[] = " :\0";
                        char topicFour[] = "\n\0";

                        strcat(combind, topicOne);
                        strcat(combind, clientName[i]);
                        strcat(combind, topicTwo);
                        strcat(combind, channel);
                        strcat(combind, topicThree);
                        strcat(combind, topicName[k]);
                        strcat(combind, topicFour);

                        send(i, combind, strlen(combind), 0);
                    }else
                    {
                        temp = strlen(channel) - 1;
                        channel[temp] = '\0';

                        for(k=0; k<20; k++) //zhao dao dai ying de channel
                        {
                            compare = strcmp(channel, channelName[k]);
                            if(compare == 0)
                            {
                                break;
                            }
                        }

                        char combind[100] = {};
                        if(topicName[k][0] == 0)
                        {
                            char topicOne[] = ":mid 331 \0";
                            char topicTwo[] = " #\0";
                            char topicThree[] = " :No topic is set\n\0";

                            strcat(combind, topicOne);
                            strcat(combind, clientName[i]);
                            strcat(combind, topicTwo);
                            strcat(combind, channelName[k]);
                            strcat(combind, topicThree);
                        }else
                        {
                            char topicOne[] = ":mid 332 \0";
                            char topicTwo[] = " #\0";
                            char topicThree[] = " :\0";
                            char topicFour[] = "\n\0";

                            strcat(combind, topicOne);
                            strcat(combind, clientName[i]);
                            strcat(combind, topicTwo);
                            strcat(combind, channelName[k]);
                            strcat(combind, topicThree);
                            strcat(combind, topicName[k]);
                            strcat(combind, topicFour);
                        }

                        send(i, combind, strlen(combind), 0);
                    }
                    break;
                }

                compare = strcmp("PRIVMSG", commandSeven); //shou dao "TOPIC", wei channel she topic
                if(compare == 0)
                {
                    if(buf[9] == 0)
                    {
                      char combind[100] = {};
                      char errorOne[] = ":mid 411 \0";
                      char errorTwo[] = " :No recipient given (PRIVMSG)\n\0";

                      strcat(combind, errorOne);
                      strcat(combind, clientName[i]);
                      strcat(combind, errorTwo);

                      send(i, combind, strlen(combind), 0);
                    }else
                    {
                      char channel[20] = {};
                      for(k=0; k<20; k++) //kan zai na yi ge channel li
                      {
                          if(buf[9+k] == ':')
                          {
                              break;
                          }
                          channel[k] = buf[9+k];
                      }
                      channel[k-1] = '\0';
                      temp = 10 + k;

                      if(k == 20)
                      {
                         channel[nbytes-10] = '\0';
                      }

                      //printf("debug, channel (%ld): %s\n", strlen(channel), channel);
                      for(k=0; k<channelNum; k++)
                      {
                          //printf("debug, channelName: %s\n", channelName[k]);
                          compare = strcmp(channel, channelName[k]);
                          if(compare == 0)
                          {
                              break;
                          }
                      }
                      //printf("debug, k: %d, channelNum: %d\n", k, channelNum);
                      if(k > channelNum) //chuang li xin de channel
                      {
                         char combind[100] = {};
                         char errorOne[] = ":mid 401 \0";
                         char errorTwo[] = " #\0";
                         char errorThree[] = " :No such nick/channel\n\0";

                         strcat(combind, errorOne);
                         strcat(combind, clientName[i]);
                         strcat(combind, errorTwo);
                         strcat(combind, channel);
                         strcat(combind, errorThree);

                         send(i, combind, strlen(combind), 0);
                      }else
                      {
                        for(k=0; k<nbytes; k++)
                        {
                          if(buf[k] == ':')
                          {
                            break;
                          }
                        }

                        if(k == nbytes)
                        {
                          char combind[100] = {};
                          char errorOne[] = ":mid 412 \0";
                          char errorTwo[] = " :No text to send\n\0";

                          strcat(combind, errorOne);
                          strcat(combind, clientName[i]);
                          strcat(combind, errorTwo);

                          send(i, combind, strlen(combind), 0);
                        }else
                        {
                          char combind[100] = {};
                          char messageOne[] = ":\0";
                          char messageTwo[] = " PRIVMSG #\0";
                          char messageThree[] = " :\0";
                          char messageFour[100]  = {};
                          char messageFive[] = "\n\0";

                          for(k=0; k<100; k++)
                          {
                              messageFour[k] = buf[temp+k];
                          }

                          strcat(combind, messageOne);
                          strcat(combind, clientName[i]);
                          strcat(combind, messageTwo);
                          strcat(combind, channel);
                          strcat(combind, messageThree);
                          strcat(combind, messageFour);
                          strcat(combind, messageFive);

                          for(j = 0; j <= fdmax; j++)
                          {
                             if (FD_ISSET(j, &master))
                             {
                                 if (j != listener && j != i)
                                 {
                                      send(j, combind, strlen(combind), 0);
                                 }
                             }
                          }
                        }
                      }
                    }
                    break;
                }

                if(compare != 0)
                {
                  buf[nbytes-1] = '\0';

                  char combind[100] = {};
                  char errorOne[] = ":mid 421 \0";
                  char errorTwo[] = " \0";
                  char errorThree[] = " :Unknown command\n\0";

                  strcat(combind, errorOne);
                  strcat(combind, clientName[i]);
                  strcat(combind, errorTwo);
                  strcat(combind, buf);
                  strcat(combind, errorThree);

                  send(i, combind, strlen(combind), 0);
                }
            }
         }
      }
    }
  }

  return 0;
}
