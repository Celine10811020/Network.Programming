#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <sys/un.h>
#include <math.h>

int main()
{
	int sock;
	struct sockaddr_un clientSock;
	char buf[8192];
  char data[100];
	int len;

	if((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
  {
		perror("socket");
		return 0;
	}

	memset(&clientSock, 0, sizeof(clientSock));
	clientSock.sun_family = AF_UNIX;
	strcpy(clientSock.sun_path, "./sudoku.sock");

	if(connect(sock, (struct sockaddr *)&clientSock, sizeof(clientSock)) < 0)
  {
		perror("connect");
	  return 0;
	}


	send(sock, "s\n", 3, 0);
	recv(sock, data, 8192, 0);
	//printf("data: %s\n", data);

		char message[8];

		int i, j, temp;
    int index, one, two;
    int mask = 0x000000ff;
    int sudoku[81], row[30], column[30], blank[30];
    int dot = 30;

    //string to number
    for(i=0; i<81; i++)
    {
        sudoku[i] = (int)data[i+4]&mask;
    }

    //convert number n to 2^n
    index = 0;
    for(i=0; i<9; i++)
    {
        for(j=0; j<9; j++)
        {
            temp = (i*9) + j;

            if(sudoku[temp] == 46)
            {
                row[index] = i;
                column[index] = j;
                index++;
                sudoku[temp] = 0;
            }else
            {
                sudoku[temp] = pow(2, sudoku[temp]-49);
            }
        }
    }

    //reset blank
    for(i=0; i<30; i++)
    {
        blank[i] = 511;
    }

    //solve sudoku
    while(dot>0)
    {
        for(i=0; i<30; i++)
        {
            if(blank[i] == -1)
            {
                continue;
            }

            one = row[i];
            two = column[i];
            index = (one*9) + two;

            //in the same row
            for(j=0; j<81; j++)
            {
                if((j/9==one) && (j!=index))
                {
                    blank[i] = blank[i] & (~sudoku[j]);
                }
            }

            //in the same column
            for(j=0; j<81; j++)
            {
                if((j%9==two) && (j!=index))
                {
                    blank[i] = blank[i] & (~sudoku[j]);
                }
            }

            //in the same box
            for(j=0; j<81; j++)
            {
                if(((j%9)/3 == (index%9)/3) && (j/27 == index/27) && (j!=index))
                {
                    blank[i] = blank[i] & (~sudoku[j]);
                }
            }

            //test for the ans
            if((blank[i]&(blank[i]-1)) == 0)
            {
                sudoku[index] = blank[i];
                blank[i] = log2(blank[i]) + 1;

                message[0] = 118;
                message[1] = 32;
                message[2] = one + 48;
                message[3] = 32;
                message[4] = two + 48;
                message[5] = 32;
                message[6] = blank[i] + 48;
                message[7] = 10;

								//sleep(1);

                send(sock, message, strlen(message), 0);
                recv(sock, buf, 8192, 0);

                blank[i] = -1;
                dot--;
            }
        }
    }

		send(sock, "c\n", 2, 0);
		recv(sock, buf, 8192, 0);

	close(sock);

	unlink("/sudoku.sock");

	return 0;
}
