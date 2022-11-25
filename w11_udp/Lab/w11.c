#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main()
{
    int len;
    char buf[64];

    srand(0x7791dd89);

    for(int i=0; i<5; i++)
    {
      rand();
    }

    len  = snprintf(buf,     10, "%x", rand());
	  len += snprintf(buf+len, 10, "%x", rand());
	  len += snprintf(buf+len, 10, "%x", rand());
    len += snprintf(buf+len, 10, "%x", rand());
    buf[len] = '\0';

    printf("%s\n", buf);
}
