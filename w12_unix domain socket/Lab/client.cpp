/*
 * Lab problem set for INP course
 * by Chun-Ying Huang <chuang@cs.nctu.edu.tw>
 * License: GPLv2
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <fstream>
#include <vector>

#define err_quit(m) { perror(m); exit(-1); }

#define NIPQUAD(s)	((unsigned char *) &s)[0], \
					((unsigned char *) &s)[1], \
					((unsigned char *) &s)[2], \
					((unsigned char *) &s)[3]

static int s = -1;
static struct sockaddr_in sin;

using namespace std;

int main(int argc, char *argv[]) // /client /files 1000 10.113.113.255
{
	printf("\tdebug:\n\t");
	for(int a=0; a<argc; a++)
	{
		printf("%s ", argv[a]);
	}
	printf("\n");

	int fd, run = 0;
    vector<bool> a(40000000);
    a.clear();
	if(argc < 5) {
		return -fprintf(stderr, "usage: %s ... <port> <ip>\n", argv[0]);
	}

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(strtol(argv[argc-2], NULL, 0));
	if(inet_pton(AF_INET, argv[argc-1], &sin.sin_addr) != 1) {
		return -fprintf(stderr, "** cannot convert IPv4 address for %s\n", argv[1]);
	}

	if((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		err_quit("socket");
    bool b = 1;
	while(1) {
		run %= 1000;
        if(!run) {
            if (b == 0) {
                char t[4];
                int m = 2147483647;
                memcpy(t, &m, 4);
                for (int i = 0; i < 10; i++)
                {
                    if(sendto(s, t, sizeof(t), 0, (struct sockaddr*) &sin, sizeof(sin)) < 0)
			            perror("sendto");
                }
                break;
            }
            b = 0;
        }
		int count = 0, rlen;
		char buf[32768], file_name[15], check = 0, ack[5];
		sprintf(file_name, ".%s/000%03d", argv[argc-4], run);
        //sprintf(file_name, ".%s/a (%d)", argv[argc-4], run);
		ifstream in(file_name, ios::binary);
		in.read(buf, sizeof(buf));
		count = in.gcount();
        int i;
        for (i = 0; i < count/1393; i++) {
            char tem[1400];
            unsigned int mask = 0x000000ff, ii = run*32768+i*1393;
            uint16_t psize = 1393;
            if (a[ii])
                continue;
            b = 1;
            memcpy(tem, &ii, 4);
            memcpy(tem+4, &psize, 2);
            memcpy(tem+6, buf+1393*i, 1393);
            check = 0;
            for (int j = 0; j < 1399; j++) {
                check ^= tem[j];
            }
            memcpy(tem+1399, &check, 1);

            if(sendto(s, tem, sizeof(tem), 0, (struct sockaddr*) &sin, sizeof(sin)) < 0)
			    perror("sendto");
            struct sockaddr_in csin;
		    socklen_t csinlen = sizeof(csin);
            rlen = recvfrom(s, ack, sizeof(ack), MSG_DONTWAIT, (struct sockaddr*) &csin, &csinlen);
            unsigned int data;
            int ch = ack[0]^ack[1]^ack[2]^ack[3], real = ack[4];
            ch = ch&mask;
            real = real&mask;
            if (rlen > 0 && (ch == real)) {

                data = ((int)ack[3]&mask)*16777216 + ((int)ack[2]&mask)*65536 + ((int)ack[1]&mask)*256 + ((int)ack[0]&mask);

                a[data] = 1;
            }
        }
        if (count%1393 != 0) {
            char tem[1400];
            unsigned int mask = 0x000000ff, ii = run*32768+i*1393;
            uint16_t psize = count%1393;

            memcpy(tem, &ii, 4);
            memcpy(tem+4, &psize, 2);
            memcpy(tem+6, buf+1393*i, count%1393);
            check = 0;
            for (int j = 0; j < 1399; j++) {
                check ^= tem[j];
            }
            memcpy(tem+1399, &check, 1);

            if(sendto(s, tem, sizeof(tem), 0, (struct sockaddr*) &sin, sizeof(sin)) < 0)
			    perror("sendto");
        }


				run++;

        in.close();
	}

	close(s);*/
}
