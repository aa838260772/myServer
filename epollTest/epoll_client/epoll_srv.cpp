#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/epoll.h>
#include <fcntl.h>
#define MAXLEN 4

#define ERR_EXIT(m) \
    do { \
        perror(m);\
        exit(EXIT_FAILURE);\
    }while(0)

int listenf()
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);//准备一个socketfd
    if(listenfd == -1 )
        ERR_EXIT("listen");

    fcntl(listenfd, F_SETFL, O_NONBLOCK );
    int on = 1;
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)//setsockopt设置端口复用
    {
        close(listenfd);
        ERR_EXIT("setsockopt");
    }

    struct sockaddr_in seraddr;
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(8888);
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    socklen_t len = sizeof(seraddr);
    if(bind(listenfd, (struct sockaddr*)&seraddr, len) == -1)//监听socket端口，
    {
        close(listenfd);
        ERR_EXIT("bind");
    }

    if(listen(listenfd, 6) == -1)
    {
        close(listenfd);
        ERR_EXIT("listen");
    }
    return listenfd;
}

void do_epoll(int fd)
{
    char recvbuf[MAXLEN] = {0};
    int epollfd = epoll_create(2048);//设置的最大连接数
    if(epollfd == -1)
        ERR_EXIT("epoll_create");

    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN|EPOLLET;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1)//加入轮询
                 ERR_EXIT("epoll_ctl_add");

    struct epoll_event events[2048];//数组在epoll_wait返回结果的时候使用
    int ret;
    int i;//在下面while的for循环中遍历使用
    int rfd;
    int clientfd;
    int nread;
    while(1)
    {
        ret = epoll_wait(epollfd, events, 2048, -1);
        if(ret == -1)
            ERR_EXIT("epoll_wait");

        for(i = 0; i < ret; ++i )
        {
            rfd = events[i].data.fd;
            printf("listenfd:%d\n", fd);
            if(rfd == fd)
            {
                if((clientfd = accept(fd, NULL, NULL)) == -1)
                    ERR_EXIT("accept");
                printf("accept_fd:%d", clientfd);
//                fcntl(clientfd, F_SETFL, O_NONBLOCK );
                ev.data.fd = clientfd;
                ev.events = EPOLLIN;
                if(epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev) == -1)
                    ERR_EXIT("epoll_ctl");
            }else
            {
#if 1
            printf ("handle!fd:%d\n", rfd);
            sleep(1);
            continue;
#endif
                int nread = read(rfd, recvbuf, MAXLEN);
                printf("nread:%d\n", nread);
                if(nread == -1)
                {
                    if(errno == EINTR)
                        continue;
                    ERR_EXIT("read");
                }else if( nread == 0)//客户端退出，从epoll轮询中删除
                {
                    printf("%d fd close\n", rfd);
                    ev.data.fd = rfd;
                    ev.events = EPOLLIN;
                    if(epoll_ctl(epollfd, EPOLL_CTL_DEL, rfd, &ev) == -1)
                        ERR_EXIT("epoll_ctl");
                }else
                {
                    printf("fd:%d,size:%d,recvmsg:%s\n", fd, strlen(recvbuf), recvbuf);
                    if(write(rfd, recvbuf, strlen(recvbuf)) == -1)
                        ERR_EXIT("write");
                    memset(recvbuf, 0, MAXLEN);
                }
            }
        }
    }
    close(epollfd);
}


int main(int argc, const char *argv[])
{
    int listenfd = listenf();
    do_epoll(listenfd);
    close(listenfd);
    return 0;
}
