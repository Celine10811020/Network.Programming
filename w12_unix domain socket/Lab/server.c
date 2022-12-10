/*
 * Lab problem set for INP course
 * by Chun-Ying Huang <chuang@cs.nctu.edu.tw>
 * License: GPLv2
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>


#define err_quit(m) { perror(m); exit(-1); }

int main(int argc, char *argv[])
{
	int s;
	struct sockaddr_in sin;

	if(argc < 2) {
		return -fprintf(stderr, "usage: %s ... <port>\n", argv[0]);
	}

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(strtol(argv[argc-1], NULL, 0));

	if((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		err_quit("socket");

	if(bind(s, (struct sockaddr*) &sin, sizeof(sin)) < 0)
		err_quit("bind");

	int status;
	//status = mkdir("/home/ruby/virtual/file", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	status = mkdir("/home/files", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	//status = mkdir("/files", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	FILE *output;
	//char path[] = "./file/000\0";
	char path[] = "./files/000\0";
	int i, temp;

	for(i=0; i<1000; i++)
	{
		char fileName[4] = {};
		char filePath[20] = {};
		sprintf(fileName, "%03d", i);

		strcat(filePath, path);
		strcat(filePath, fileName);

		output = fopen(filePath, "w");
		fclose(output);
	}

	int data, name, location, fileSize;
	char checksum;
	unsigned int mask = 0x000000ff;
	int one, two;

	i = 1;
	while(1)
	{
		struct sockaddr_in csin;
		socklen_t csinlen = sizeof(csin);
		unsigned char buf[512] = {};
		int rlen;

		if((rlen = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*) &csin, &csinlen)) < 0)
		{
			perror("recvfrom");
			break;
		}

		if(rlen > 0)
		{
			data = ((int)buf[3]&mask)*16777216 + ((int)buf[2]&mask)*65536 + ((int)buf[1]&mask)*256 + ((int)buf[0]&mask);

			if(data == 2147483647)
			{
				break;
			}
			/*fileSize = ((int)buf[5]&mask)*256 + ((int)buf[4]&mask);

			name = data / 32768;
			location = data % 32768;

			printf("%d(%d): location(%d): %x\t", name, fileSize, location, buf[6]);*/

			checksum = 0;
			for(i=0; i<511; i++)
			{
				checksum = checksum ^ buf[i];
			}

			one = (int)checksum&mask;
			two = (int)buf[511]&mask;

			//printf("checksum: %x, buf[511]: %x\n", one, two);

			if(one == two)
			{
				//printf("in\n");
				//data = ((int)buf[3]&mask)*16777216 + ((int)buf[2]&mask)*65536 + ((int)buf[1]&mask)*256 + ((int)buf[0]&mask);
				fileSize = ((int)buf[5]&mask)*256 + ((int)buf[4]&mask);

				name = data / 32768;
				location = data % 32768;

				char fileName[4] = {};
				char filePath[20] = {};
				sprintf(fileName, "%03d", name);

				strcat(filePath, path);
				strcat(filePath, fileName);

				char *p = buf;
				char *pp = p + 6;

				output = fopen(filePath, "rb+");
				fseek(output, location, SEEK_SET);
				fwrite(pp, fileSize, 1, output);
				fclose(output);

				/*struct stat sb;

				if(stat(filePath, &sb) == -1)
				{
	        perror("stat");
	        exit(EXIT_FAILURE);
	    	}
				printf("%s size: %lu bytes\n", filePath, sb.st_size);*/

				buf[4] = buf[0] ^ buf[1] ^ buf[2] ^ buf[3];
				buf[5] = '\0';
				sendto(s, buf, 5, 0, (struct sockaddr*) &csin, sizeof(csin));

				/*for(i=0; i<5; i++)
				{
					printf("%x ", (int)buf[i]&mask);
				}
				printf("\n");*/
			}
		}
	}

	close(s);
}
