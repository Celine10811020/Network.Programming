#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h> //sockaddr_in and inet_ntoa()
#include <fcntl.h>     //access()

#define on_error(...) { fprintf(stderr, __VA_ARGS__); fflush(stderr); exit(1); }

/*int gettimeofday (struct timeval * tv, struct timezone * tz);
    //成功return 1，失敗return 0
    struct timeval{
    long tv_sec;  //秒
    long tv_usec;  //微秒
};*/

int main (int argc, char *argv[])
{
  fd_set master;
  fd_set read_fds;
  int nbytes;
  int server_fd, client_fd, err, server_sink, client_sink;
  struct sockaddr_in server, client, serverSink, clientSink;

  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  //chuang li liang ge socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) on_error("Could not create server_fd socket\n");
  server_sink = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sink < 0) on_error("Could not create server_sink socket\n");

  //di yi ge socket de zi liao
  server.sin_family = AF_INET;
  server.sin_port = htons(10002);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

  err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
  if (err < 0) on_error("Could not bind server_fd socket\n");

  //di er ge socket de zi liao
  server.sin_port = htons(10003);

  setsockopt(server_sink, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

  err = bind(server_sink, (struct sockaddr *) &server, sizeof(server));
  if (err < 0) on_error("Could not bind server_sink socket\n");

  //listen
  err = listen(server_fd, 128);
  if (err < 0) on_error("Could not listen on server_fd socket\n");

  err = listen(server_sink, 128);
  if (err < 0) on_error("Could not listen on server_sink socket\n");

  int fd[2];
  pipe(fd);

  fcntl(fd[1], F_SETPIPE_SZ, 1048576);

  pid_t pid;
  pid = fork();

  //accept
  if(pid == 0)
  {
    socklen_t client_len = sizeof(client);
    client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);

    struct timeval tv;
    struct timezone tz;
    long timeSec, timeUsec;

    int compare;
    while(1) //wu qung hui quan
    {
        gettimeofday (&tv, &tz);
        timeSec = tv.tv_sec;
        timeUsec = tv.tv_usec;

        char buf[100] = {};

        nbytes = recv(client_fd, buf, 100, 0);

        printf("%s", buf);
        if(nbytes>1)
        {
          compare = strcmp("/ping\n", buf);
          if(compare == 0)
          {
            gettimeofday (&tv, &tz);
            //printf("%ld.%ld\n", tv.tv_sec, tv.tv_usec);

            char combind[100] = {};
            char pingOne[20] = {};
            sprintf(pingOne, "%ld", tv.tv_sec);
            char pingTwo[] = ".\0";
            char pingThree[10] = {};
            sprintf(pingThree, "%ld", tv.tv_usec);
            char pingFour[] = " PONG\n\0";

            strcat(combind, pingOne);
            strcat(combind, pingTwo);
            strcat(combind, pingThree);
            strcat(combind, pingFour);

            printf("debug: %s\n", combind);

            send(client_fd, combind, strlen(combind), 0);
          }

          compare = strcmp("/reset\n", buf);
          if(compare == 0)
          {
            char reset[1048576] = {};
            read(fd[0], reset, 1048576);

            gettimeofday (&tv, &tz);
            //printf("%ld.%ld\n", tv.tv_sec, tv.tv_usec);

            char combind[100] = {};
            char resetOne[20] = {};
            sprintf(resetOne, "%ld", tv.tv_sec);
            char resetTwo[] = ".\0";
            char resetThree[10] = {};
            sprintf(resetThree, "%ld", tv.tv_usec);
            char resetFour[] = " RESET \0";
            char resetFive[20] = {};
            sprintf(resetFive, "%ld", strlen(reset));
            char resetSix[] = "\n\0";

            strcat(combind, resetOne);
            strcat(combind, resetTwo);
            strcat(combind, resetThree);
            strcat(combind, resetFour);
            strcat(combind, resetFive);
            strcat(combind, resetSix);

            printf("debug: %s\n", combind);

            send(client_fd, combind, strlen(combind), 0);

            timeSec = tv.tv_sec;
            timeUsec = tv.tv_usec;
          }

          compare = strcmp("/report\n", buf);
          if(compare == 0)
          {
            gettimeofday (&tv, &tz);
            timeSec = tv.tv_sec - timeSec;
            timeUsec = abs(tv.tv_usec - timeUsec);

            char report[1048576] = {};
            read(fd[0], report, 1048576);

            int sinkData = strlen(report);

            float measured = 8 * sinkData / 1000000 / timeSec;

            char combind[100] = {};
            char reportOne[20] = {};
            sprintf(reportOne, "%ld", tv.tv_sec);
            char reportTwo[] = ".\0";
            char reportThree[10] = {};
            sprintf(reportThree, "%ld", tv.tv_usec);
            char reportFour[] = " REPORT \0";
            char reportFive[20] = {};
            sprintf(reportFive, "%ld", timeSec);
            char reportSix[] = ".\0";
            char reportSeven[20] = {};
            sprintf(reportSeven, "%ld", timeUsec);
            char reportEight[] = "s \0";
            char reportNine[20] = {};
            sprintf(reportNine, "%f", measured);
            char reportTen[] = "Mbps\n\0";

            strcat(combind, reportOne);
            strcat(combind, reportTwo);
            strcat(combind, reportThree);
            strcat(combind, reportFour);
            strcat(combind, reportFive);
            strcat(combind, reportSix);
            strcat(combind, reportSeven);
            strcat(combind, reportEight);
            strcat(combind, reportNine);
            strcat(combind, reportTen);

            printf("debug: %s\n", combind);

            send(client_fd, combind, strlen(combind), 0);
          }
      }
    }
  }else if(pid > 0)
  {
    socklen_t clientSink_len = sizeof(clientSink);
    client_sink = accept(server_sink, (struct sockaddr *) &clientSink, &clientSink_len);

    while(1)
    {
        char buf[1048576] = {};
        nbytes = recv(client_sink, buf, 1048576, 0);
        write(fd[1], buf, strlen(buf));
    }
  }else
  {
    printf("forkError\n");
  }

  return 0;
}
