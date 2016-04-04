#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <signal.h>

#define ERR_EXIT(m) \
    do { \
        perror(m);\
        exit(EXIT_FAILURE);\
    }while(0)
#define MAXLINE 1024
static void do_client(int fd)
{
    char recvbuf[MAXLINE + 1] = {0};
    char sendbuf[MAXLINE + 1] = {0};

    fd_set reade, ready;
    FD_ZERO(&reade);
    int fd_stdin = fileno(stdin);
    FD_SET(fd_stdin, &reade);
    FD_SET(fd, &reade);
    int fd_max = (fd_stdin > fd) ? fd_stdin : fd;

    int ret;
    while(1)
    {
        ready = reade;
        ret = select( fd_max+1, &ready, NULL, NULL, NULL);//轮询
        if(ret == -1)
        {
            if(errno == EINTR)
                continue;
            ERR_EXIT("select");
        }else if(ret ==  0)
        {
            continue;
        }

        if(FD_ISSET(fd_stdin, &ready))
        {
            if(fgets(sendbuf, sizeof(sendbuf), stdin) == NULL)
            {
                close(fd);
               break;
            }else
            {
                if( -1 == write(fd, sendbuf, strlen(sendbuf)))
                    printf("write\n");
            }
        }

        if(FD_ISSET(fd, &ready))
        {
            int nread = read(fd, recvbuf, MAXLINE);
            if(nread < 0)
                ERR_EXIT("read");
            if(nread == 0)//如果没接收到消息，打印关闭描述符，退出循环
            {
                fprintf(stdout, "fd close\n");
                break;
            }
            fprintf(stdout, "receive:%s", recvbuf);
            sleep(1);
        }
        memset(recvbuf, 0, sizeof recvbuf);
        memset(sendbuf, 0, sizeof sendbuf);
    }
}
void handle(int signum)
{
printf("sigpipe\n");
}

int main(int argc, const char *argv[])
{
    signal(SIGPIPE, SIG_IGN);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0)
        ERR_EXIT("socket");

    struct sockaddr_in cliaddr;
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(8999);
    cliaddr.sin_addr.s_addr = inet_addr("192.168.1.52");
    socklen_t len = sizeof cliaddr;

    int ret ;
    if((ret = connect(fd, (struct sockaddr*)&cliaddr, len)) == -1)
    {
        close(fd);
        ERR_EXIT("connect");
    }
    do_client(fd);
    close(fd);
    return 0;
}
