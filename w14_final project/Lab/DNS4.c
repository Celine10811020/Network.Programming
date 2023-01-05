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

  //while(1)
  {
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
    int ininindex;
    int ip[5];
    int ipv6Num = 0;

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

        int dot, length;

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

            if(answer[number][temp+1] == 1) //'A'
            {
              if(ininindex == 1)
              {
                answer[number][temp+10+index] = ip[0];
              }else if(ininindex == 2)
              {
                answer[number][temp+10+index] = ip[0]*10 + ip[1];
              }else if(ininindex == 3)
              {
                answer[number][temp+10+index] = ip[0]*100 + ip[1]*10 + ip[2];
              }

              index++;
              ininindex = 0;
            }else if(answer[number][temp+1] == 2) //'NS'
            {
              char tempArr[100];

              temp = answer[number][0];

              int indexx = 0;
              length = 0;
              dot = 0;
              ininindex = 1;
              for(j=temp+10; j<temp+10+index; j++)
              {
                if(answer[number][j] == 46)
                {
                  tempArr[dot] = length;
                  dot = ininindex;
                  ininindex++;
                  length = 0;
                }else
                {
                  tempArr[ininindex] = answer[number][j];
                  length++;
                  ininindex++;
                }
              }

              ininindex--;
              tempArr[ininindex] = 0;
              ininindex++;

              index = 0;
              for(j=0; j<ininindex; j++)
              {
                answer[number][temp+10+index] = tempArr[j];
                index++;
              }

            }else if(answer[number][temp+1] == 3) //'MD'
            {

            }else if(answer[number][temp+1] == 4) //'MF'
            {

            }else if(answer[number][temp+1] == 5) //'CNAME'
            {
              char tempArr[100];

              temp = answer[number][0];

              int indexx = 0;
              length = 0;
              dot = 0;
              ininindex = 1;
              for(j=temp+10; j<temp+10+index; j++)
              {
                if(answer[number][j] == 46)
                {
                  tempArr[dot] = length;
                  dot = ininindex;
                  ininindex++;
                  length = 0;
                }else
                {
                  tempArr[ininindex] = answer[number][j];
                  length++;
                  ininindex++;
                }
              }

              ininindex--;
              tempArr[ininindex] = 0;
              ininindex++;

              index = 0;
              for(j=0; j<ininindex; j++)
              {
                answer[number][temp+10+index] = tempArr[j];
                index++;
              }

              ininindex = 0;

            }else if(answer[number][temp+1] == 6) //'SOA'
            {
              char tempArr[100];
              int tempArrNum[10];

              temp = answer[number][0];

              int gogoOn = 1;
              int indexx = 0;
              length = 0;
              dot = 0;
              ininindex = 1;
              for(j=temp+10; j<temp+10+index; j++)
              {
                if(gogoOn == 1)
                {
                  if(answer[number][j] == 46)
                  {
                    tempArr[dot] = length;
                    dot = ininindex;
                    ininindex++;
                    length = 0;
                  }else if(answer[number][j] == 32)
                  {
                    tempArr[dot] = 0;
                    dot++;
                    ininindex++;
                    gogoOn = 2;
                  }else
                  {
                    tempArr[ininindex] = answer[number][j];
                    length++;
                    ininindex++;
                  }
                }else if(gogoOn == 2)
                {
                  if(answer[number][j] == 46)
                  {
                    tempArr[dot] = length;
                    dot = ininindex;
                    ininindex++;
                    length = 0;
                  }else if(answer[number][j] == 32)
                  {
                    tempArr[dot] = 0;
                    //ininindex++;
                    gogoOn = 3;
                  }else
                  {
                    tempArr[ininindex] = answer[number][j];
                    length++;
                    ininindex++;
                  }
                }else
                {
                  if(answer[number][j] == 32)
                  {
                    long numnum;

                    if(indexx == 10)
                    {
                      numnum = tempArrNum[0]*1000000000 + tempArrNum[1]*100000000 + tempArrNum[2]*10000000 + tempArrNum[3]*1000000 + tempArrNum[4]*100000 + tempArrNum[5]*10000 + tempArrNum[6]*1000 + tempArrNum[7]*100 + tempArrNum[8]*10 + tempArrNum[9];
                    }else if(indexx == 9)
                    {
                      numnum = tempArrNum[0]*100000000 + tempArrNum[1]*10000000 + tempArrNum[2]*1000000 + tempArrNum[3]*100000 + tempArrNum[4]*10000 + tempArrNum[5]*1000 + tempArrNum[6]*100 + tempArrNum[7]*10 + tempArrNum[8];
                    }else if(indexx == 8)
                    {
                      numnum = tempArrNum[0]*10000000 + tempArrNum[1]*1000000 + tempArrNum[2]*100000 + tempArrNum[3]*10000 + tempArrNum[4]*1000 + tempArrNum[5]*100 + tempArrNum[6]*10 + tempArrNum[7];
                    }else if(indexx == 7)
                    {
                      numnum = tempArrNum[0]*1000000 + tempArrNum[1]*100000 + tempArrNum[2]*10000 + tempArrNum[3]*1000 + tempArrNum[4]*100 + tempArrNum[5]*10 + tempArrNum[6];
                    }else if(indexx == 6)
                    {
                      numnum = tempArrNum[0]*100000 + tempArrNum[1]*10000 + tempArrNum[2]*1000 + tempArrNum[3]*100 + tempArrNum[4]*10 + tempArrNum[5];
                    }else if(indexx == 5)
                    {
                      numnum = tempArrNum[0]*10000 + tempArrNum[1]*1000 + tempArrNum[2]*100 + tempArrNum[3]*10 + tempArrNum[4];
                    }else if(indexx == 4)
                    {
                      numnum = tempArrNum[0]*1000 + tempArrNum[1]*100 + tempArrNum[2]*10 + tempArrNum[3];
                    }else if(indexx == 3)
                    {
                      numnum = tempArrNum[0]*100 + tempArrNum[1]*10 + tempArrNum[2];
                    }else if(indexx == 2)
                    {
                      numnum = tempArrNum[0]*10 + tempArrNum[1];
                    }else if(indexx == 1)
                    {
                      numnum = tempArrNum[0];
                    }

                    tempArr[ininindex+3] = numnum % 256;
                    numnum = numnum / 256;
                    tempArr[ininindex+2] = numnum % 256;
                    numnum = numnum / 256;
                    tempArr[ininindex+1] = numnum % 256;
                    numnum = numnum / 256;
                    tempArr[ininindex] = numnum % 256;

                    ininindex = ininindex + 4;

                    indexx = 0;
                  }else
                  {
                    tempArrNum[indexx] = answer[number][j] - 48;
                    indexx++;
                  }
                }
              }
              long numnum;
              if(indexx == 10)
              {
                numnum = tempArrNum[0]*1000000000 + tempArrNum[1]*100000000 + tempArrNum[2]*10000000 + tempArrNum[3]*1000000 + tempArrNum[4]*100000 + tempArrNum[5]*10000 + tempArrNum[6]*1000 + tempArrNum[7]*100 + tempArrNum[8]*10 + tempArrNum[9];
              }else if(indexx == 9)
              {
                numnum = tempArrNum[0]*100000000 + tempArrNum[1]*10000000 + tempArrNum[2]*1000000 + tempArrNum[3]*100000 + tempArrNum[4]*10000 + tempArrNum[5]*1000 + tempArrNum[6]*100 + tempArrNum[7]*10 + tempArrNum[8];
              }else if(indexx == 8)
              {
                numnum = tempArrNum[0]*10000000 + tempArrNum[1]*1000000 + tempArrNum[2]*100000 + tempArrNum[3]*10000 + tempArrNum[4]*1000 + tempArrNum[5]*100 + tempArrNum[6]*10 + tempArrNum[7];
              }else if(indexx == 7)
              {
                numnum = tempArrNum[0]*1000000 + tempArrNum[1]*100000 + tempArrNum[2]*10000 + tempArrNum[3]*1000 + tempArrNum[4]*100 + tempArrNum[5]*10 + tempArrNum[6];
              }else if(indexx == 6)
              {
                numnum = tempArrNum[0]*100000 + tempArrNum[1]*10000 + tempArrNum[2]*1000 + tempArrNum[3]*100 + tempArrNum[4]*10 + tempArrNum[5];
              }else if(indexx == 5)
              {
                numnum = tempArrNum[0]*10000 + tempArrNum[1]*1000 + tempArrNum[2]*100 + tempArrNum[3]*10 + tempArrNum[4];
              }else if(indexx == 4)
              {
                numnum = tempArrNum[0]*1000 + tempArrNum[1]*100 + tempArrNum[2]*10 + tempArrNum[3];
              }else if(indexx == 3)
              {
                numnum = tempArrNum[0]*100 + tempArrNum[1]*10 + tempArrNum[2];
              }else if(indexx == 2)
              {
                numnum = tempArrNum[0]*10 + tempArrNum[1];
              }else if(indexx == 1)
              {
                numnum = tempArrNum[0];
              }

              tempArr[ininindex+3] = numnum % 256;
              numnum = numnum / 256;
              tempArr[ininindex+2] = numnum % 256;
              numnum = numnum / 256;
              tempArr[ininindex+1] = numnum % 256;
              numnum = numnum / 256;
              tempArr[ininindex] = numnum % 256;

              ininindex = ininindex + 4;

              //indexx = 0;

              index = 0;
              for(j=0; j<ininindex; j++)
              {
                answer[number][temp+10+index] = tempArr[j];
                index++;
              }
            }else if(answer[number][temp+1] == 7) //'MB'
            {

            }else if(answer[number][temp+1] == 8) //'MG'
            {

            }else if(answer[number][temp+1] == 9) //'MR'
            {

            }else if(answer[number][temp+1] == 10) //'NULL'
            {

            }else if(answer[number][temp+1] == 11) //'WKS'
            {

            }else if(answer[number][temp+1] == 12) //'PTR'
            {

            }else if(answer[number][temp+1] == 13) //'HINFO'
            {

            }else if(answer[number][temp+1] == 14) //'MINFO'
            {

            }else if(answer[number][temp+1] == 15) //'MX'
            {
              char tempArr[100];
              int tempArrNum[5];

              temp = answer[number][0];

              int indexx = 0;
              int gogoOn = 1;
              length = 0;
              dot = 0;
              for(j=temp+10; j<temp+10+index; j++)
              {
                if(gogoOn == 1)
                {
                  if(answer[number][j] == 32)
                  {
                    int numnum;

                    if(indexx == 5)
                    {
                      numnum = tempArrNum[0]*10000 + tempArrNum[1]*1000 + tempArrNum[2]*100 + tempArrNum[3]*10 + tempArrNum[4];
                    }else if(indexx == 4)
                    {
                      numnum = tempArrNum[0]*1000 + tempArrNum[1]*100 + tempArrNum[2]*10 + tempArrNum[3];
                    }else if(indexx == 3)
                    {
                      numnum = tempArrNum[0]*100 + tempArrNum[1]*10 + tempArrNum[2];
                    }else if(indexx == 2)
                    {
                      numnum = tempArrNum[0]*10 + tempArrNum[1];
                    }else if(indexx == 1)
                    {
                      numnum = tempArrNum[0];
                    }

                    tempArr[1] = numnum % 256;
                    tempArr[0] = numnum / 256;

                    dot = 2;
                    ininindex = 3;

                    gogoOn = 2;
                    indexx = 0;
                  }else
                  {
                    tempArrNum[indexx] = answer[number][j] - 48;
                    indexx++;
                  }
                }else
                {
                  if(answer[number][j] == 46)
                  {
                    tempArr[dot] = length;
                    dot = ininindex;
                    ininindex++;
                    length = 0;
                  }else
                  {
                    tempArr[ininindex] = answer[number][j];
                    length++;
                    ininindex++;
                  }
                }
              }

              ininindex--;
              tempArr[ininindex] = 0;
              ininindex++;

              index = 0;
              for(j=0; j<ininindex; j++)
              {
                answer[number][temp+10+index] = tempArr[j];
                index++;
              }

            }else if(answer[number][temp+1] == 16) //'TXT'
            {
              temp = answer[number][0];
              answer[number][temp+10] = index - 1;
            }else if(answer[number][temp+1] == 28) //'AAAA'
            {
              ipv6Num++;

              if(ininindex == 4)
              {
                answer[number][temp+10+index] = ip[0]*16 + ip[1];
                index++;
                answer[number][temp+10+index] = ip[2]*16 + ip[3];
                index++;
              }else if(ininindex == 3)
              {
                answer[number][temp+10+index] = ip[0];
                index++;
                answer[number][temp+10+index] = ip[1]*16 + ip[2];
                index++;
              }else if(ininindex == 2)
              {
                answer[number][temp+10+index] = 0;
                index++;
                answer[number][temp+10+index] = ip[0]*16 + ip[1];
                index++;
              }else if(ininindex == 1)
              {
                answer[number][temp+10+index] = 0;
                index++;
                answer[number][temp+10+index] = ip[0];
                index++;
              }else if(ininindex == 0)
              {
                answer[number][temp+10+index] = 0;
                index++;
                answer[number][temp+10+index] = 0;
                index++;
              }

              if(ipv6Num < 8)
              {
                if(answer[number][temp+10+0]==0 && answer[number][temp+10+1]==0)
                {
                  if(ipv6Num == 0)
                  {
                    for(j=2; j<=15; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 1)
                  {
                    answer[number][temp+10+15] = answer[number][temp+10+3];
                    answer[number][temp+10+14] = answer[number][temp+10+2];

                    for(j=2; j<=13; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 2)
                  {
                    int k = 15;
                    for(j=5; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=2; j<=11; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 3)
                  {
                    int k = 15;
                    for(j=7; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=2; j<=9; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 4)
                  {
                    int k = 15;
                    for(j=9; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=2; j<=7; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 5)
                  {
                    int k = 15;
                    for(j=11; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=2; j<=5; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 6)
                  {
                    int k = 15;
                    for(j=13; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=2; j<=3; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }
                }else if(answer[number][temp+10+2]==0 && answer[number][temp+10+3]==0)
                {
                  if(ipv6Num == 1)
                  {
                    for(j=4; j<=15; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 2)
                  {
                    answer[number][temp+10+15] = answer[number][temp+10+5];
                    answer[number][temp+10+14] = answer[number][temp+10+4];

                    for(j=4; j<=13; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 3)
                  {
                    int k = 15;
                    for(j=7; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=4; j<=11; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 4)
                  {
                    int k = 15;
                    for(j=9; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=4; j<=9; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 5)
                  {
                    int k = 15;
                    for(j=11; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=4; j<=7; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 6)
                  {
                    int k = 15;
                    for(j=13; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=4; j<=5; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }
                }else if(answer[number][temp+10+4]==0 && answer[number][temp+10+5]==0)
                {
                  if(ipv6Num == 2)
                  {
                    for(j=6; j<=15; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 3)
                  {
                    answer[number][temp+10+15] = answer[number][temp+10+7];
                    answer[number][temp+10+14] = answer[number][temp+10+6];

                    for(j=6; j<=13; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 4)
                  {
                    int k = 15;
                    for(j=9; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=6; j<=11; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 5)
                  {
                    int k = 15;
                    for(j=11; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=6; j<=9; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 6)
                  {
                    int k = 15;
                    for(j=13; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=6; j<=7; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }
                }else if(answer[number][temp+10+6]==0 && answer[number][temp+10+7]==0)
                {
                  if(ipv6Num == 3)
                  {
                    for(j=8; j<=15; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 4)
                  {
                    answer[number][temp+10+15] = answer[number][temp+10+9];
                    answer[number][temp+10+14] = answer[number][temp+10+8];

                    for(j=8; j<=13; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 5)
                  {
                    int k = 15;
                    for(j=11; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=8; j<=11; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 6)
                  {
                    int k = 15;
                    for(j=13; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=8; j<=9; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }
                }else if(answer[number][temp+10+8]==0 && answer[number][temp+10+9]==0)
                {
                  if(ipv6Num == 4)
                  {
                    for(j=10; j<=15; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 5)
                  {
                    answer[number][temp+10+15] = answer[number][temp+10+11];
                    answer[number][temp+10+14] = answer[number][temp+10+9];

                    for(j=10; j<=13; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 6)
                  {
                    int k = 15;
                    for(j=13; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=10; j<=11; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }
                }else if(answer[number][temp+10+10]==0 && answer[number][temp+10+11]==0)
                {
                  if(ipv6Num == 5)
                  {
                    for(j=12; j<=15; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 6)
                  {
                    answer[number][temp+10+15] = answer[number][temp+10+13];
                    answer[number][temp+10+14] = answer[number][temp+10+12];

                    for(j=12; j<=13; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }
                }else if(answer[number][temp+10+12]==0 && answer[number][temp+10+13]==0)
                {
                  if(ipv6Num == 6)
                  {
                    for(j=14; j<=15; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }
                }
              }
              index = 16;
            }

            temp = answer[number][0];

            answer[number][temp+8] = index / 256;
            answer[number][temp+9] = index % 256;

            answer[number][0] = temp + index + 10;

            break;
          }

          //{ENTER}
          if(tmp[0] == 13)
          {
            temp = answer[number][0];

            if(answer[number][temp+1] == 1) //'A'
            {
              if(ininindex == 1)
              {
                answer[number][temp+10+index] = ip[0];
              }else if(ininindex == 2)
              {
                answer[number][temp+10+index] = ip[0]*10 + ip[1];
              }else if(ininindex == 3)
              {
                answer[number][temp+10+index] = ip[0]*100 + ip[1]*10 + ip[2];
              }

              index++;
              ininindex = 0;
            }else if(answer[number][temp+1] == 2) //'NS'
            {
              char tempArr[100];

              temp = answer[number][0];

              int indexx = 0;
              length = 0;
              dot = 0;
              ininindex = 1;
              for(j=temp+10; j<temp+10+index; j++)
              {
                if(answer[number][j] == 46)
                {
                  tempArr[dot] = length;
                  dot = ininindex;
                  ininindex++;
                  length = 0;
                }else
                {
                  tempArr[ininindex] = answer[number][j];
                  length++;
                  ininindex++;
                }
              }

              ininindex--;
              tempArr[ininindex] = 0;
              ininindex++;

              index = 0;
              for(j=0; j<ininindex; j++)
              {
                answer[number][temp+10+index] = tempArr[j];
                index++;
              }

              ininindex = 0;

            }else if(answer[number][temp+1] == 3) //'MD'
            {

            }else if(answer[number][temp+1] == 4) //'MF'
            {

            }else if(answer[number][temp+1] == 5) //'CNAME'
            {
              char tempArr[100];

              temp = answer[number][0];

              int indexx = 0;
              length = 0;
              dot = 0;
              ininindex = 1;
              for(j=temp+10; j<temp+10+index; j++)
              {
                if(answer[number][j] == 46)
                {
                  tempArr[dot] = length;
                  dot = ininindex;
                  ininindex++;
                  length = 0;
                }else
                {
                  tempArr[ininindex] = answer[number][j];
                  length++;
                  ininindex++;
                }
              }

              ininindex--;
              tempArr[ininindex] = 0;
              ininindex++;

              index = 0;
              for(j=0; j<ininindex; j++)
              {
                answer[number][temp+10+index] = tempArr[j];
                index++;
              }

              ininindex = 0;

            }else if(answer[number][temp+1] == 6) //'SOA'
            {
              char tempArr[100];
              int tempArrNum[10];

              temp = answer[number][0];

              int gogoOn = 1;
              int indexx = 0;
              length = 0;
              dot = 0;
              ininindex = 1;
              for(j=temp+10; j<temp+10+index; j++)
              {
                if(gogoOn == 1)
                {
                  if(answer[number][j] == 46)
                  {
                    tempArr[dot] = length;
                    dot = ininindex;
                    ininindex++;
                    length = 0;
                  }else if(answer[number][j] == 32)
                  {
                    tempArr[dot] = 0;
                    dot++;
                    ininindex++;
                    gogoOn = 2;
                  }else
                  {
                    tempArr[ininindex] = answer[number][j];
                    length++;
                    ininindex++;
                  }
                }else if(gogoOn == 2)
                {
                  if(answer[number][j] == 46)
                  {
                    tempArr[dot] = length;
                    dot = ininindex;
                    ininindex++;
                    length = 0;
                  }else if(answer[number][j] == 32)
                  {
                    tempArr[dot] = 0;
                    //ininindex++;
                    gogoOn = 3;
                  }else
                  {
                    tempArr[ininindex] = answer[number][j];
                    length++;
                    ininindex++;
                  }
                }else
                {
                  if(answer[number][j] == 32)
                  {
                    long numnum;

                    if(indexx == 10)
                    {
                      numnum = tempArrNum[0]*1000000000 + tempArrNum[1]*100000000 + tempArrNum[2]*10000000 + tempArrNum[3]*1000000 + tempArrNum[4]*100000 + tempArrNum[5]*10000 + tempArrNum[6]*1000 + tempArrNum[7]*100 + tempArrNum[8]*10 + tempArrNum[9];
                    }else if(indexx == 9)
                    {
                      numnum = tempArrNum[0]*100000000 + tempArrNum[1]*10000000 + tempArrNum[2]*1000000 + tempArrNum[3]*100000 + tempArrNum[4]*10000 + tempArrNum[5]*1000 + tempArrNum[6]*100 + tempArrNum[7]*10 + tempArrNum[8];
                    }else if(indexx == 8)
                    {
                      numnum = tempArrNum[0]*10000000 + tempArrNum[1]*1000000 + tempArrNum[2]*100000 + tempArrNum[3]*10000 + tempArrNum[4]*1000 + tempArrNum[5]*100 + tempArrNum[6]*10 + tempArrNum[7];
                    }else if(indexx == 7)
                    {
                      numnum = tempArrNum[0]*1000000 + tempArrNum[1]*100000 + tempArrNum[2]*10000 + tempArrNum[3]*1000 + tempArrNum[4]*100 + tempArrNum[5]*10 + tempArrNum[6];
                    }else if(indexx == 6)
                    {
                      numnum = tempArrNum[0]*100000 + tempArrNum[1]*10000 + tempArrNum[2]*1000 + tempArrNum[3]*100 + tempArrNum[4]*10 + tempArrNum[5];
                    }else if(indexx == 5)
                    {
                      numnum = tempArrNum[0]*10000 + tempArrNum[1]*1000 + tempArrNum[2]*100 + tempArrNum[3]*10 + tempArrNum[4];
                    }else if(indexx == 4)
                    {
                      numnum = tempArrNum[0]*1000 + tempArrNum[1]*100 + tempArrNum[2]*10 + tempArrNum[3];
                    }else if(indexx == 3)
                    {
                      numnum = tempArrNum[0]*100 + tempArrNum[1]*10 + tempArrNum[2];
                    }else if(indexx == 2)
                    {
                      numnum = tempArrNum[0]*10 + tempArrNum[1];
                    }else if(indexx == 1)
                    {
                      numnum = tempArrNum[0];
                    }

                    tempArr[ininindex+3] = numnum % 256;
                    numnum = numnum / 256;
                    tempArr[ininindex+2] = numnum % 256;
                    numnum = numnum / 256;
                    tempArr[ininindex+1] = numnum % 256;
                    numnum = numnum / 256;
                    tempArr[ininindex] = numnum % 256;

                    ininindex = ininindex + 4;

                    indexx = 0;
                  }else
                  {
                    tempArrNum[indexx] = answer[number][j] - 48;
                    indexx++;
                  }
                }
              }
              long numnum;
              if(indexx == 10)
              {
                numnum = tempArrNum[0]*1000000000 + tempArrNum[1]*100000000 + tempArrNum[2]*10000000 + tempArrNum[3]*1000000 + tempArrNum[4]*100000 + tempArrNum[5]*10000 + tempArrNum[6]*1000 + tempArrNum[7]*100 + tempArrNum[8]*10 + tempArrNum[9];
              }else if(indexx == 9)
              {
                numnum = tempArrNum[0]*100000000 + tempArrNum[1]*10000000 + tempArrNum[2]*1000000 + tempArrNum[3]*100000 + tempArrNum[4]*10000 + tempArrNum[5]*1000 + tempArrNum[6]*100 + tempArrNum[7]*10 + tempArrNum[8];
              }else if(indexx == 8)
              {
                numnum = tempArrNum[0]*10000000 + tempArrNum[1]*1000000 + tempArrNum[2]*100000 + tempArrNum[3]*10000 + tempArrNum[4]*1000 + tempArrNum[5]*100 + tempArrNum[6]*10 + tempArrNum[7];
              }else if(indexx == 7)
              {
                numnum = tempArrNum[0]*1000000 + tempArrNum[1]*100000 + tempArrNum[2]*10000 + tempArrNum[3]*1000 + tempArrNum[4]*100 + tempArrNum[5]*10 + tempArrNum[6];
              }else if(indexx == 6)
              {
                numnum = tempArrNum[0]*100000 + tempArrNum[1]*10000 + tempArrNum[2]*1000 + tempArrNum[3]*100 + tempArrNum[4]*10 + tempArrNum[5];
              }else if(indexx == 5)
              {
                numnum = tempArrNum[0]*10000 + tempArrNum[1]*1000 + tempArrNum[2]*100 + tempArrNum[3]*10 + tempArrNum[4];
              }else if(indexx == 4)
              {
                numnum = tempArrNum[0]*1000 + tempArrNum[1]*100 + tempArrNum[2]*10 + tempArrNum[3];
              }else if(indexx == 3)
              {
                numnum = tempArrNum[0]*100 + tempArrNum[1]*10 + tempArrNum[2];
              }else if(indexx == 2)
              {
                numnum = tempArrNum[0]*10 + tempArrNum[1];
              }else if(indexx == 1)
              {
                numnum = tempArrNum[0];
              }

              tempArr[ininindex+3] = numnum % 256;
              numnum = numnum / 256;
              tempArr[ininindex+2] = numnum % 256;
              numnum = numnum / 256;
              tempArr[ininindex+1] = numnum % 256;
              numnum = numnum / 256;
              tempArr[ininindex] = numnum % 256;

              ininindex = ininindex + 4;

              //indexx = 0;

              index = 0;
              for(j=0; j<ininindex; j++)
              {
                answer[number][temp+10+index] = tempArr[j];
                index++;
              }
            }else if(answer[number][temp+1] == 7) //'MB'
            {

            }else if(answer[number][temp+1] == 8) //'MG'
            {

            }else if(answer[number][temp+1] == 9) //'MR'
            {

            }else if(answer[number][temp+1] == 10) //'NULL'
            {

            }else if(answer[number][temp+1] == 11) //'WKS'
            {

            }else if(answer[number][temp+1] == 12) //'PTR'
            {

            }else if(answer[number][temp+1] == 13) //'HINFO'
            {

            }else if(answer[number][temp+1] == 14) //'MINFO'
            {

            }else if(answer[number][temp+1] == 15) //'MX'
            {
              char tempArr[100];
              int tempArrNum[5];

              temp = answer[number][0];

              int indexx = 0;
              int gogoOn = 1;
              length = 0;
              dot = 0;
              for(j=temp+10; j<temp+10+index; j++)
              {
                if(gogoOn == 1)
                {
                  if(answer[number][j] == 32)
                  {
                    int numnum;

                    if(indexx == 5)
                    {
                      numnum = tempArrNum[0]*10000 + tempArrNum[1]*1000 + tempArrNum[2]*100 + tempArrNum[3]*10 + tempArrNum[4];
                    }else if(indexx == 4)
                    {
                      numnum = tempArrNum[0]*1000 + tempArrNum[1]*100 + tempArrNum[2]*10 + tempArrNum[3];
                    }else if(indexx == 3)
                    {
                      numnum = tempArrNum[0]*100 + tempArrNum[1]*10 + tempArrNum[2];
                    }else if(indexx == 2)
                    {
                      numnum = tempArrNum[0]*10 + tempArrNum[1];
                    }else if(indexx == 1)
                    {
                      numnum = tempArrNum[0];
                    }

                    tempArr[1] = numnum % 256;
                    tempArr[0] = numnum / 256;

                    dot = 2;
                    ininindex = 3;

                    gogoOn = 2;
                    indexx = 0;
                  }else
                  {
                    tempArrNum[indexx] = answer[number][j] - 48;
                    indexx++;
                  }
                }else
                {
                  if(answer[number][j] == 46)
                  {
                    tempArr[dot] = length;
                    dot = ininindex;
                    ininindex++;
                    length = 0;
                  }else
                  {
                    tempArr[ininindex] = answer[number][j];
                    length++;
                    ininindex++;
                  }
                }
              }

              ininindex--;
              tempArr[ininindex] = 0;
              ininindex++;

              index = 0;
              for(j=0; j<ininindex; j++)
              {
                answer[number][temp+10+index] = tempArr[j];
                index++;
              }
            }else if(answer[number][temp+1] == 16) //'TXT'
            {
              temp = answer[number][0];
              answer[number][temp+10] = index - 1;
            }else if(answer[number][temp+1] == 28) //'AAAA'
            {
              ipv6Num++;

              if(ininindex == 4)
              {
                answer[number][temp+10+index] = ip[0]*16 + ip[1];
                index++;
                answer[number][temp+10+index] = ip[2]*16 + ip[3];
                index++;
              }else if(ininindex == 3)
              {
                answer[number][temp+10+index] = ip[0];
                index++;
                answer[number][temp+10+index] = ip[1]*16 + ip[2];
                index++;
              }else if(ininindex == 2)
              {
                answer[number][temp+10+index] = 0;
                index++;
                answer[number][temp+10+index] = ip[0]*16 + ip[1];
                index++;
              }else if(ininindex == 1)
              {
                answer[number][temp+10+index] = 0;
                index++;
                answer[number][temp+10+index] = ip[0];
                index++;
              }else if(ininindex == 0)
              {
                answer[number][temp+10+index] = 0;
                index++;
                answer[number][temp+10+index] = 0;
                index++;
              }

              if(ipv6Num < 8)
              {
                if(answer[number][temp+10+0]==0 && answer[number][temp+10+1]==0)
                {
                  if(ipv6Num == 0)
                  {
                    for(j=2; j<=15; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 1)
                  {
                    answer[number][temp+10+15] = answer[number][temp+10+3];
                    answer[number][temp+10+14] = answer[number][temp+10+2];

                    for(j=2; j<=13; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 2)
                  {
                    int k = 15;
                    for(j=5; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=2; j<=11; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 3)
                  {
                    int k = 15;
                    for(j=7; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=2; j<=9; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 4)
                  {
                    int k = 15;
                    for(j=9; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=2; j<=7; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 5)
                  {
                    int k = 15;
                    for(j=11; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=2; j<=5; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 6)
                  {
                    int k = 15;
                    for(j=13; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=2; j<=3; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }
                }else if(answer[number][temp+10+2]==0 && answer[number][temp+10+3]==0)
                {
                  if(ipv6Num == 1)
                  {
                    for(j=4; j<=15; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 2)
                  {
                    answer[number][temp+10+15] = answer[number][temp+10+5];
                    answer[number][temp+10+14] = answer[number][temp+10+4];

                    for(j=4; j<=13; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 3)
                  {
                    int k = 15;
                    for(j=7; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=4; j<=11; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 4)
                  {
                    int k = 15;
                    for(j=9; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=4; j<=9; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 5)
                  {
                    int k = 15;
                    for(j=11; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=4; j<=7; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 6)
                  {
                    int k = 15;
                    for(j=13; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=4; j<=5; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }
                }else if(answer[number][temp+10+4]==0 && answer[number][temp+10+5]==0)
                {
                  if(ipv6Num == 2)
                  {
                    for(j=6; j<=15; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 3)
                  {
                    answer[number][temp+10+15] = answer[number][temp+10+7];
                    answer[number][temp+10+14] = answer[number][temp+10+6];

                    for(j=6; j<=13; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 4)
                  {
                    int k = 15;
                    for(j=9; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=6; j<=11; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 5)
                  {
                    int k = 15;
                    for(j=11; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=6; j<=9; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 6)
                  {
                    int k = 15;
                    for(j=13; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=6; j<=7; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }
                }else if(answer[number][temp+10+6]==0 && answer[number][temp+10+7]==0)
                {
                  if(ipv6Num == 3)
                  {
                    for(j=8; j<=15; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 4)
                  {
                    answer[number][temp+10+15] = answer[number][temp+10+9];
                    answer[number][temp+10+14] = answer[number][temp+10+8];

                    for(j=8; j<=13; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 5)
                  {
                    int k = 15;
                    for(j=11; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=8; j<=11; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 6)
                  {
                    int k = 15;
                    for(j=13; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=8; j<=9; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }
                }else if(answer[number][temp+10+8]==0 && answer[number][temp+10+9]==0)
                {
                  if(ipv6Num == 4)
                  {
                    for(j=10; j<=15; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 5)
                  {
                    answer[number][temp+10+15] = answer[number][temp+10+11];
                    answer[number][temp+10+14] = answer[number][temp+10+9];

                    for(j=10; j<=13; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 6)
                  {
                    int k = 15;
                    for(j=13; j>=2; j--)
                    {
                      answer[number][temp+10+k] = answer[number][temp+10+j];
                      k--;
                    }
                    for(j=10; j<=11; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }
                }else if(answer[number][temp+10+10]==0 && answer[number][temp+10+11]==0)
                {
                  if(ipv6Num == 5)
                  {
                    for(j=12; j<=15; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }else if(ipv6Num == 6)
                  {
                    answer[number][temp+10+15] = answer[number][temp+10+13];
                    answer[number][temp+10+14] = answer[number][temp+10+12];

                    for(j=12; j<=13; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }
                }else if(answer[number][temp+10+12]==0 && answer[number][temp+10+13]==0)
                {
                  if(ipv6Num == 6)
                  {
                    for(j=14; j<=15; j++)
                    {
                      answer[number][temp+10+j] = 0;
                    }
                  }
                }
              }
              index = 16;
            }

            temp = fread(tmp, sizeof(unsigned char), 1, input);

            temp = answer[number][0];

            answer[number][temp+8] = index / 256;
            answer[number][temp+9] = index % 256;

            answer[number][0] = temp + index + 10;

            dot = 0;
            index = 0;
            goOn = 0;
            ininindex = 0;
            ipv6Num = 0;
            number++;
            //printf("\n");
            continue;
          }

          if(goOn == 0)
          {
            if(tmp[0] == 44)
            {
              dot = 0;
              length = 0;

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
                index++;
                continue;
              }

              compare = strcmp(resourceType, "AAAA");
              if(compare == 0)
              {
                answer[number][temp+1] = 28;
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

            if(answer[number][temp+1] == 1) //'A'
            {
              if(tmp[0] == 46)
              {
                if(ininindex == 1)
                {
                  answer[number][temp+10+index] = ip[0];
                }else if(ininindex == 2)
                {
                  answer[number][temp+10+index] = ip[0]*10 + ip[1];
                }else if(ininindex == 3)
                {
                  answer[number][temp+10+index] = ip[0]*100 + ip[1]*10 + ip[2];
                }

                index++;
                ininindex = 0;
                continue;
              }

              ip[ininindex] = tmp[0] - 48;
              ininindex++;

            }else if(answer[number][temp+1] == 2) //'NS'
            {
              temp = answer[number][0];

              answer[number][temp+10+index] = tmp[0];
              index++;

            }else if(answer[number][temp+1] == 3) //'MD'
            {

            }else if(answer[number][temp+1] == 4) //'MF'
            {

            }else if(answer[number][temp+1] == 5) //'CNAME'
            {
              temp = answer[number][0];

              answer[number][temp+10+index] = tmp[0];
              index++;

            }else if(answer[number][temp+1] == 6) //'SOA'
            {
              temp = answer[number][0];

              answer[number][temp+10+index] = tmp[0];
              index++;

            }else if(answer[number][temp+1] == 7) //'MB'
            {

            }else if(answer[number][temp+1] == 8) //'MG'
            {

            }else if(answer[number][temp+1] == 9) //'MR'
            {

            }else if(answer[number][temp+1] == 10) //'NULL'
            {

            }else if(answer[number][temp+1] == 11) //'WKS'
            {

            }else if(answer[number][temp+1] == 12) //'PTR'
            {

            }else if(answer[number][temp+1] == 13) //'HINFO'
            {

            }else if(answer[number][temp+1] == 14) //'MINFO'
            {

            }else if(answer[number][temp+1] == 15) //'MX'
            {
              temp = answer[number][0];

              answer[number][temp+10+index] = tmp[0];
              index++;

            }else if(answer[number][temp+1] == 16) //'TXT'
            {
              temp = answer[number][0];

              answer[number][temp+10+index] = tmp[0];
              index++;
            }else if(answer[number][temp+1] == 28) //'AAAA'
            {
              temp = answer[number][0];

              if(tmp[0] == 58)
              {
                ipv6Num++;

                if(ininindex == 4)
                {
                  answer[number][temp+10+index] = ip[0]*16 + ip[1];
                  index++;
                  answer[number][temp+10+index] = ip[2]*16 + ip[3];
                  index++;
                }else if(ininindex == 3)
                {
                  answer[number][temp+10+index] = ip[0];
                  index++;
                  answer[number][temp+10+index] = ip[1]*16 + ip[2];
                  index++;
                }else if(ininindex == 2)
                {
                  answer[number][temp+10+index] = 0;
                  index++;
                  answer[number][temp+10+index] = ip[0]*16 + ip[1];
                  index++;
                }else if(ininindex == 1)
                {
                  answer[number][temp+10+index] = 0;
                  index++;
                  answer[number][temp+10+index] = ip[0];
                  index++;
                }else if(ininindex == 0)
                {
                  answer[number][temp+10+index] = 0;
                  index++;
                  answer[number][temp+10+index] = 0;
                  index++;
                }

                ininindex = 0;
                continue;
              }

              ip[ininindex] = tmp[0] - 48;
              ininindex++;
            }
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
    printf("AnswerSection\n");
    for(i=0; i<=number; i++)
    {
      printf("%d\t", answer[i][0]);
      temp = answer[i][0];

      for(j=1; j<temp; j++)
      {
        printf("%c(%02X) ", answer[i][j], answer[i][j]&mask);
      }

      printf("\n");
    }


    int temtemp;

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
      //buffer[6] = number / 256;
      //buffer[7] = number % 256;
      buffer[8] = 0;
      buffer[9] = 0;
      buffer[10] = 0;
      buffer[11] = 0;

      inindex++;
      index = 12 + inindex + 4;
      ininindex = 0;
      int gotAnswer = 0;
      for(i=0; i<=number; i++)
      {
        temp = answer[i][0];

        temtemp = 0;
        while(answer[i][temtemp] != 0)
        {
          temtemp++;
        }

        if((buffer[12+inindex+1]==answer[i][temtemp+2]) || (buffer[12+inindex+1]==answer[i][temtemp+3]))
        {
          for(j=1; j<temp; j++)
          {
            buffer[index] = answer[i][j];
            index++;
          }

          ininindex++;
          gotAnswer = 1;
        }
      }

      buffer[6] = ininindex / 256;
      buffer[7] = ininindex % 256;
      buffer[8] = 0;
      buffer[9] = 2;

      if(gotAnswer == 1)
      {
        for(i=0; i<=number; i++)
        {
          temp = answer[i][0];

          temtemp = 0;
          while(answer[i][temtemp] != 0)
          {
            temtemp++;
          }

          if((2==answer[i][temtemp+2]) || (2==answer[i][temtemp+3]))
          {
            for(j=1; j<temp; j++)
            {
              buffer[index] = answer[i][j];
              index++;
            }

            ininindex++;
            gotAnswer = 1;
          }
        }
      }else
      {
        for(i=0; i<=number; i++)
        {
          temp = answer[i][0];

          temtemp = 0;
          while(answer[i][temtemp] != 0)
          {
            temtemp++;
          }

          if((6==answer[i][temtemp+2]) || (6==answer[i][temtemp+3]))
          {
            for(j=1; j<temp; j++)
            {
              buffer[index] = answer[i][j];
              index++;
            }

            ininindex++;
            gotAnswer = 1;
          }
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

      //debug
      /*for(i=0; i<20; i++)
      {
        for(j=0; j<16; j++)
        {
          printf("%02X ", buffer[i*16+j]&mask);
        }
        printf("\n");
      }*/

      //printf("\n\t%d\n", inindex);
    }
  }
  close(sock);

  return 0;
}
