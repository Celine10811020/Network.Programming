#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>

int main()
{
  FILE *input;

  int i, j;
  char forwardIP[20] = {};
  char domain[100][50] = {};
  char filePath[100][100] = {};
  unsigned char tmp[2];
  int temp;
  int mask = 0x000000ff;
  int index, number;
  int goOn;

  input = fopen("config.txt", "rb");

  //forwardIP
  index = 0;
  while(1)
  {
    temp = fread(tmp, sizeof(unsigned char), 1, input);

    //break when {ENTER}
    if(tmp[0] == 13)
    {
      temp = fread(tmp, sizeof(unsigned char), 1, input);
      break;
    }

    forwardIP[index] = tmp[0];
    index++;
  }

  //debug
  for(i=0; i<strlen(forwardIP); i++)
  {
    printf("%c", forwardIP[i]);
  }
  printf("\n");

  //domain and its file
  index = 0;
  number = 0;
  goOn = 0;
  while(1)
  {
    temp = fread(tmp, sizeof(unsigned char), 1, input);

    //break when EOF
    if(temp == 0)
    {
      break;
    }

    //44 == ','
    if(tmp[0] == 44)
    {
      temp = fread(tmp, sizeof(unsigned char), 1, input);
      domain[number][index] = '\0';
      index = 0;
      goOn = 1;
      //printf("\n");
    }

    if(goOn == 0)
    {
      domain[number][index] = tmp[0];
      //printf("%d ", tmp[0]);
      index++;
    }else
    {
      if(tmp[0] == 13)
      {
        temp = fread(tmp, sizeof(unsigned char), 1, input);
        filePath[number][index] = '\0';
        number++;
        index = 0;
        goOn = 0;
        //printf("\n");
      }

      if(goOn == 1)
      {
        filePath[number][index] = tmp[0];
        index++;
        //printf("%d ", tmp[0]);
      }
    }

    //printf("%d ", tmp[0]);
  }
  printf("\n");

  //debug
  for(i=0; i<=number; i++)
  {
    for(j=0; j<strlen(domain[i]); j++)
    {
      printf("%c", domain[i][j]);
    }

    printf("\t");

    for(j=0; j<strlen(filePath[i]); j++)
    {
      printf("%c", filePath[i][j]);
    }

    printf("\n");
  }
  printf("\n");

  fclose(input);



  char buffer[1024] = {};
  struct sockaddr_in client_addr;
  socklen_t addr_len = sizeof(struct sockaddr_in);
  struct sockaddr_in addr;
  int nbytes, rc;
  int sock;
  int port = 9000;

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  sock = socket(AF_INET, SOCK_DGRAM, 0);

  rc = bind(sock, (struct sockaddr *) &addr, addr_len);

  if(rc != 0)
  {
    printf("Could not bind: %s\n", strerror(errno));
    return 1;
  }

  //printf("Use 'dig @ip -p 9000 domain name' command in other terminal \nListening on port %u.\n \n", port);
  printf("Listening on port %u.\n \n", port);

  char id[2] = {};
  char flag[2] = {};
  char questionCount[2] = {};
  char answerRRCount[2] = {};
  char authorityRRCount[2] = {};
  char additionRRCount[2] = {};
  char questionName[50] = {};
  char questionType[2] = {};
  char questionClass[2] = {};
  int inindex;

  //client to server
  nbytes = recvfrom(sock, buffer, 1024, 0, (struct sockaddr *) &client_addr, &addr_len);

  id[0] = buffer[0];
  id[1] = buffer[1];

  //get questionName
  index = 12;
  inindex = 0;
  while(1)
  {
    if(buffer[index] == 0)
    {
      break;
    }

    temp = buffer[index];
    index++;

    for(i=0; i<temp; i++)
    {
      questionName[inindex] = buffer[index];
      inindex++;
      index++;
    }

    questionName[inindex] = 46;
    inindex++;
  }

  //debug
  for(i=0; i<inindex; i++)
  {
    printf("%c", questionName[i]);
  }
  printf("\n");

  char answer[100][200] = {};
  char resource[50] = {};
  char resourceName[50] = {};
  char resourceType[6] = {};
  char resourceClass[3] = {};
  int ttl[4];
  int ttlNum;
  char dataLengh[4] = {};
  char data[100] = {};
  int compare;
  int goToDNS = 1;

  //compare the questionName with the domain in file
  for(i=0; i<=number; i++)
  {
    compare = strcmp(questionName, domain[i]);

    if(compare == 0)
    {
      goToDNS = 0;

      number = i;

      //debug
      printf("got it!  %s\t%s\n\n", domain[number], filePath[number]);

      for(i=0; i<100; i++)
      {
        answer[i][0] = 1;
      }

      input = fopen(filePath[number], "rb");

      //read the file and creat the answer section of packet

      //domain name
      number = 0;
      index = 0;
      while(1)
      {
        temp = fread(tmp, sizeof(unsigned char), 1, input);

        //break when {ENTER}
        if(tmp[0] == 13)
        {
          temp = fread(tmp, sizeof(unsigned char), 1, input);
          number++;
          break;
        }

        resource[index] = tmp[0];
        index++;
        //printf("%d ", tmp[0]);
      }

      //debug
      /*for(i=0; i<strlen(resource); i++)
      {
        printf("%c", resource[i]);
      }
      printf("\n");*/

      //answerSection
      index = 0;
      goOn = 0; //0==resourceName, 1==resourceType, 2==resourceClass, 3==TTL, 4==dataLengh, 5==resourceData
      while(1)
      {
        temp = fread(tmp, sizeof(unsigned char), 1, input);

        //break when EOF
        if(temp == 0)
        {
          temp = answer[number][0];

          answer[number][temp+8] = index / 256;
          answer[number][temp+9] = index % 256;

          answer[number][0] = temp + index + 10;

          break;
        }

        if(tmp[0] == 13)
        {
          temp = fread(tmp, sizeof(unsigned char), 1, input);

          temp = answer[number][0];

          answer[number][temp+8] = index / 256;
          answer[number][temp+9] = index % 256;

          answer[number][0] = temp + index + 10;

          index = 0;
          goOn = 0;
          number++;
          //printf("\n");
          continue;
        }

        if(goOn == 0)
        {
          if(tmp[0] == 44)
          {
            int dot, length;

            temp = answer[number][0];

            if(resourceName[0] == 64)
            {
              dot = temp;
              temp++;

              for(i=0; i<strlen(resource); i++)
              {
                //printf("%c ", resource[i]);

                if(resource[i] == 46)
                {
                  length = temp - dot - 1;
                  answer[number][dot] = length;
                  dot = temp;
                  temp++;
                  continue;
                }

                answer[number][temp] = resource[i];
                temp++;
              }
              //printf("\n");

              answer[number][dot] = 0;
              temp++;

            }else
            {
              answer[number][temp] = strlen(resourceName);
              temp++;

              for(i=0; i<strlen(resourceName); i++)
              {
                answer[number][temp] = resourceName[i];
                temp++;
              }

              answer[number][temp] = 0;
              temp++;
            }

            index = 0;
            goOn = 3;
            answer[number][0] = temp;

            //printf("\t");

            continue;
          }

          resourceName[index] = tmp[0];
          //printf("%d ", tmp[0]);
          index++;

        }else if(goOn == 1)
        {
          if(tmp[0] == 44)
          {
            //printf("\t");

            resourceType[index] = '\0';

            index = 0;
            goOn = 4;

            temp = answer[number][0];
            answer[number][temp] = 0;

            compare = strcmp(resourceType, "A");
            if(compare == 0)
            {
              answer[number][temp+1] = 1;
              continue;
            }

            compare = strcmp(resourceType, "NS");
            if(compare == 0)
            {
              answer[number][temp+1] = 2;
              continue;
            }

            compare = strcmp(resourceType, "MD");
            if(compare == 0)
            {
              answer[number][temp+1] = 3;
              continue;
            }

            compare = strcmp(resourceType, "MF");
            if(compare == 0)
            {
              answer[number][temp+1] = 4;
              continue;
            }

            compare = strcmp(resourceType, "CNAME");
            if(compare == 0)
            {
              answer[number][temp+1] = 5;
              continue;
            }

            compare = strcmp(resourceType, "SOA");
            if(compare == 0)
            {
              answer[number][temp+1] = 6;
              continue;
            }

            compare = strcmp(resourceType, "MB");
            if(compare == 0)
            {
              answer[number][temp+1] = 7;
              continue;
            }

            compare = strcmp(resourceType, "MG");
            if(compare == 0)
            {
              answer[number][temp+1] = 8;
              continue;
            }

            compare = strcmp(resourceType, "MR");
            if(compare == 0)
            {
              answer[number][temp+1] = 9;
              continue;
            }

            compare = strcmp(resourceType, "NULL");
            if(compare == 0)
            {
              answer[number][temp+1] = 10;
              continue;
            }

            compare = strcmp(resourceType, "WKS");
            if(compare == 0)
            {
              answer[number][temp+1] = 11;
              continue;
            }

            compare = strcmp(resourceType, "PTR");
            if(compare == 0)
            {
              answer[number][temp+1] = 12;
              continue;
            }

            compare = strcmp(resourceType, "HINFO");
            if(compare == 0)
            {
              answer[number][temp+1] = 13;
              continue;
            }

            compare = strcmp(resourceType, "MINFO");
            if(compare == 0)
            {
              answer[number][temp+1] = 14;
              continue;
            }

            compare = strcmp(resourceType, "MX");
            if(compare == 0)
            {
              answer[number][temp+1] = 15;
              continue;
            }

            compare = strcmp(resourceType, "TXT");
            if(compare == 0)
            {
              answer[number][temp+1] = 16;
              continue;
            }

            continue;
          }

          resourceType[index] = tmp[0];
          index++;
          //printf("%d ", tmp[0]);

        }else if(goOn == 2)
        {
          if(tmp[0] == 44)
          {
            resourceClass[index] = '\0';

            //printf("\t");
            index = 0;
            goOn = 1;

            temp = answer[number][0];
            answer[number][temp+2] = 0;

            compare = strcmp(resourceClass, "IN");
            if(compare == 0)
            {
              answer[number][temp+3] = 1;
              continue;
            }

            compare = strcmp(resourceClass, "CS");
            if(compare == 0)
            {
              answer[number][temp+3] = 2;
              continue;
            }

            compare = strcmp(resourceClass, "CH");
            if(compare == 0)
            {
              answer[number][temp+3] = 3;
              continue;
            }

            compare = strcmp(resourceClass, "HS");
            if(compare == 0)
            {
              answer[number][temp+3] = 4;
              continue;
            }

            continue;
          }

          resourceClass[index] = tmp[0];
          index++;
          //printf("%d ", tmp[0]);

        }else if(goOn == 3)
        {
          if(tmp[0] == 44)
          {
            if(index == 4)
            {
              ttlNum = ttl[0]*1000 + ttl[1]*100 + ttl[2]*10 + ttl[3];
            }else if(index == 3)
            {
              ttlNum = ttl[0]*100 + ttl[1]*10 + ttl[2];
            }else if(index == 2)
            {
              ttlNum = ttl[0]*10 + ttl[1];
            }else
            {
              ttlNum = ttl[0];
            }


            for(i=3; i>=0; i--)
            {
              ttl[i] = ttlNum % 256;
              ttlNum = ttlNum / 256;
            }

            temp = answer[number][0];
            for(i=0; i<4; i++)
            {
              answer[number][temp+4+i] = ttl[i];
            }

            index = 0;
            goOn = 2;

            //printf("\t");

            continue;
          }

          ttl[index] = tmp[0] - 48;
          index++;
          //printf("%d ", tmp[0]);
        }else
        {
          temp = answer[number][0];

          answer[number][temp+10+index] = tmp[0];
          index++;
          //printf("%d ", tmp[0]);
        }
      }
      //printf("\n");

      fclose(input);

      break;
    }
  }

  //answer count
  //number++;

  //debug
  /*for(i=0; i<number; i++)
  {
    printf("%d\t", answer[i][0]);
    temp = answer[i][0];

    for(j=1; j<temp; j++)
    {
      printf("%c(%02X) ", answer[i][j], answer[i][j]&mask);
    }

    printf("\n");
  }*/




  if(goToDNS == 1)
  {
    //Building the socket for connecting to the DNS server
    long sock_fd;
    struct sockaddr_in servaddr;
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(53);
    inet_pton(AF_INET, "208.67.222.222", &(servaddr.sin_addr));

    // Connecting to the DNS server
    connect(sock_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    // Sending the query packet to the DNS server
    write(sock_fd, buffer, 1024);

    // Receiving the response packet from the DNS server
    if(read(sock_fd, buffer, 1024) <= 0)
    {
      close(sock_fd);
    }


    /*for(int j=0; j<15; j++)
    {
      printf("%03d\t", 10*j);

      for(int i=0; i<10; i++)
      {
        printf("%c(%02X) ", buffer[10*j + i], buffer[10*j + i]);
      }

      printf("\n");
    }*/

    //buffer[0] = id[0];
    //buffer[1] = id[1];

    //server to client
    sendto(sock, buffer, 1024, 0, (struct sockaddr *) &client_addr, addr_len);


    //debug
    /*for(int j=0; j<15; j++)
    {
      printf("%03d\t", 10*j);

      for(int i=0; i<10; i++)
      {
        printf("%c(%02X) ", buffer[10*j + i], buffer[10*j + i]&mask);
      }

      printf("\n");
    }*/
  }else
  {
    buffer[0] = id[0];
    buffer[1] = id[1];
    buffer[2] = 0x81;
    buffer[3] = 0x80;
    buffer[6] = number / 256;
    buffer[7] = number % 256;
    buffer[8] = 0;
    buffer[9] = 0;
    buffer[10] = 0;
    buffer[11] = 0;

    inindex++;
    index = 12 + inindex + 4;


    for(i=0; i<number; i++)
    {
      temp = answer[i][0];

      for(j=1; j<temp; j++)
      {
        buffer[index] = answer[i][j];
        index++;
      }
    }

    /*buffer[index] = 0;
    buffer[index+1] = 0;
    buffer[index+2] = 0x29;
    buffer[index+3] = 0x04;
    buffer[index+4] = 0xd0;
    buffer[index+5] = 0;
    buffer[index+6] = 0;
    buffer[index+7] = 0;
    buffer[index+8] = 0;
    buffer[index+9] = 0;
    buffer[index+10] = 0;*/

    sendto(sock, buffer, index, 0, (struct sockaddr *) &client_addr, addr_len);

    for(i=0; i<20; i++)
    {
      for(j=0; j<16; j++)
      {
        printf("%02X ", buffer[i*16+j]&mask);
      }
      printf("\n");
    }

    //printf("\n\t%d\n", inindex);
  }

  close(sock);

  return 0;
}
