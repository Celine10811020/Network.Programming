#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>


int main()
{

	FILE *input;
	FILE *output;
	char file[16];
	unsigned char change[4], fileName[16];
	unsigned char tmp[2];
	unsigned char fileContent[16];
	int i, j, k=0;
	int offset=0;
	unsigned int offsetString, offsetContent, fileNum, offsetFileName, fileSize, offsetFileContent=0;
	int temp, Temp;

//printf("123");

	//開檔
	//input = fopen("example.pak", "rb");
	input = fopen("testcase0.pak", "rb");
	//input = fopen("testcase.pak", "rb");

	//讀有幾個byte
	while(!feof(input))
    {
        temp = fread(tmp, sizeof(unsigned char), 1, input);
        if(temp == 0)
        {
            break; //EOF 時 break
        }

        offset++;
    }

    //重新開檔，因為剛才的已經讀完了 QAQ
    fclose(input);
    //input = fopen("example.pak", "rb");
    input = fopen("testcase0.pak", "rb");
    //input = fopen("testcase.pak", "rb");

    unsigned char *hexdump;
    hexdump = (unsigned char*)malloc(sizeof(unsigned char)*offset);

	//讀檔
	offset = 0;
	while(!feof(input))
	{
		temp = fread(tmp, sizeof(unsigned char), 1, input);
		if(temp == 0)
        {
            break;
        }

        hexdump[offset] = tmp[0];

        offset++;
	}

	for(i=0; i<offset; i++)
    {
        //printf("%02X ", hexdump[i]);
        hexdump[i] = (int)(hexdump[i] & 0x00FF);
    }

	printf("%d total bytes\n", offset);

	//關檔
	fclose(input);

	//magic number
	printf("magic number: ");
    for(i=0; i<4; i++)
    {
        printf("%02X", hexdump[i]);
    }
    printf("\n");

    //offset of string section
    offsetString = (int)hexdump[7]*16777216 + (int)hexdump[6]*65536 + (int)hexdump[5]*256 + (int)hexdump[4];
    printf("offset of string section: %d\n", offsetString);

    //offset of content section
    offsetContent = (int)hexdump[11]*16777216 + (int)hexdump[10]*65536 + (int)hexdump[9]*256 + (int)hexdump[8];

	//the number of files
	for(i=0; i<4; i++)
    {
        hexdump[i+12] = (int) hexdump[i+12];
        //printf("%d ", hexdump[i+12]);
    }
	fileNum = (int)hexdump[15]*16777216 + (int)hexdump[14]*65536 + (int)hexdump[13]*256 + (int)hexdump[12];
	printf("the number of files packed: %d\n", fileNum);

    //write file
    for(i=0; i<fileNum; i++)
    {
        //file name
        fileName[0] = '\0';
        offsetFileName = (int)hexdump[19+20*i]*16777216 + (int)hexdump[18+20*i]*65536 + (int)hexdump[17+20*i]*256 + (int)hexdump[16+20*i];
        //printf("%x + %x = %x", offsetString, offsetFileName, offsetString + offsetFileName);
        temp = offsetString + offsetFileName;
        j=0;
        //printf("%X ", temp);
        while(hexdump[temp+j] != 00)
        {
            tmp[0] = hexdump[temp+j];
			tmp[1] = '\0';
			strcat(fileName, tmp);
            j++;
            //printf("%x ", temp+j);
        }
        Temp = j;
        printf("\nthe %d file name: ", i+1);
        printf("%s\n", fileName);
        output=fopen(fileName, "ab+");
        //fileName[0] = '\0';
        //size of file
        fileSize = (int)hexdump[20+20*i]*16777216 + (int)hexdump[21+20*i]*65536 + (int)hexdump[22+20*i]*256 + (int)hexdump[23+20*i];
        printf("size of %d file: %d\n", i+1, fileSize);
        //file content
        offsetFileContent = (int)hexdump[27+20*i]*16777216 + (int)hexdump[26+20*i]*65536 + (int)hexdump[25+20*i]*256 + (int)hexdump[24+20*i];
        temp = offsetContent + offsetFileContent;
        //printf("the %d file content:\n", i+1);
        fileContent[0] = '\0';
        for(j=0; j<fileSize; j++)
        {
            if(j%16 == 0 && j>0)
            {
                //printf("%s\n", fileContent);
                fwrite(fileContent, 16, 1, output);
                fileContent[0] = '\0';
            }
			tmp[0] = hexdump[temp+j];
			tmp[1] = '\0';
			strcat(fileContent, tmp);
        }
        fileSize = fileSize % 16;
        if(fileSize > 0)
        {
            //printf("%s", fileContent);
            fwrite(fileContent, fileSize, 1, output);
            fileContent[0] = '\0';
        }
        fclose(output);





        //釋放存file name的memory

    }








    free(hexdump);

	return(0);
}

