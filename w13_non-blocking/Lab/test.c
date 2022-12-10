#include <stdio.h>
#include <math.h>

int main()
{
    int i, j, temp;
    int index, one, two;
    int mask = 0x000000ff;
    int sudoku[81], row[30], column[30], blank[30];
    int dot = 30;

    char data[] = "75.13486.1.3..9.75469.78.2.3.5861..22..3457.6..8....31537..62988.49.3.17.1.782..4";

    //string to number
    for(i=0; i<81; i++)
    {
        sudoku[i] = (int)data[i]&mask;
    }

    //debug
    for(i=0; i<9; i++)
    {
        for(j=0; j<9; j++)
        {
            temp = (i*9) + j;

            printf("%d ", sudoku[temp]);
        }
        printf("\n");
    }
    printf("\n");

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

    //debug
    for(i=0; i<9; i++)
    {
        for(j=0; j<9; j++)
        {
            temp = (i*9) + j;

            printf("%d ", sudoku[temp]);
        }
        printf("\n");
    }
    printf("\n");

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
                printf("%d. (%d, %d) = %d\n", dot, one, two, blank[i]);
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

                send(sock, message, strlen(message), 0);
                recv(sock, buf, 8192, 0);

                blank[i] = -1;
                dot--;
            }
        }
    }

    //debug
    for(i=0; i<9; i++)
    {
        for(j=0; j<9; j++)
        {
            temp = (i*9) + j;

            printf("%d ", sudoku[temp]);
        }
        printf("\n");
    }
    printf("\n");

    //convert number 2^n back to n
    for(i=0; i<81; i++)
    {
        sudoku[i] = log2(sudoku[i]) + 1;
    }

    //debug
    for(i=0; i<9; i++)
    {
        for(j=0; j<9; j++)
        {
            temp = (i*9) + j;

            printf("%d ", sudoku[temp]);
        }
        printf("\n");
    }
    printf("\n");



    return 0;
}
