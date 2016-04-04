#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <iostream>

#define MAX_EVENTS 1024
#define PORT 8999
#define BUFSIZE 10

using namespace std;
//设置socket连接为非阻塞模式
void setnonblocking(int sockfd) {
    int opts;

    opts = fcntl(sockfd, F_GETFL);
    if(opts < 0) {
        perror("fcntl(F_GETFL)\n");
        exit(1);
    }
    opts = (opts | O_NONBLOCK);
    if(fcntl(sockfd, F_SETFL, opts) < 0) {
        perror("fcntl(F_SETFL)\n");
        exit(1);
    }
}

static void PrintClient(int sockfd)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    ::getpeername(sockfd, (struct sockaddr*)&addr, &len);
    cout << "ip:" << inet_ntoa(addr.sin_addr)
       << " port:" << ntohs(addr.sin_port)
       << " fd:" << sockfd
       << endl;
}


int main(){
    struct epoll_event ev, events[MAX_EVENTS];
    int addrlen, listenfd, conn_sock, nfds, epfd, fd, i, nread, n;
    struct sockaddr_in local, remote;
    char buf[BUFSIZE];

    //创建listen socket
    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("sockfd\n");
        exit(1);
    }

    int reuse = 1;
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)//setsockopt设置端口复用
    {
        close(listenfd);
        cout << "setsockopt fail!" << endl;
    }

    //监听套接字设置为非阻塞
    setnonblocking(listenfd);
    bzero(&local, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);;
    local.sin_port = htons(PORT);
    if( bind(listenfd, (struct sockaddr *) &local, sizeof(local)) < 0) {
        perror("bind\n");
        exit(1);
    }

    listen(listenfd, 20);

    epfd = epoll_create(MAX_EVENTS);
    if (epfd == -1) {
        perror("epoll_create");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        //阻塞在epoll_wait
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_pwait");
            exit(EXIT_FAILURE);
        }

        for (i = 0; i < nfds; ++i) {
            cout << nfds << "events" << endl;
            fd = events[i].data.fd;
            if (fd == listenfd) {
                while ((conn_sock = accept(listenfd,(struct sockaddr *) &remote,
                                (socklen_t*)&addrlen)) > 0)
                {
                    setnonblocking(conn_sock);
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = conn_sock;
                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, conn_sock,
                                &ev) == -1) {
                        perror("epoll_ctl: add");
                        exit(EXIT_FAILURE);
                    }
                    PrintClient(conn_sock);
                }

                if (conn_sock == -1) {
                    if (errno != EAGAIN &&
                        errno != ECONNABORTED &&
                        errno != EPROTO &&
                        errno != EINTR)
                        perror("accept fail");
                }
                continue;
            }
            else if (events[i].events & EPOLLIN) {
                n = 0;
                PrintClient(fd);
                memset(buf,0,BUFSIZE);
                while ((nread = read(fd, buf + n, BUFSIZE-1)) > 0) {
                    n += nread;
                }

                if (nread == -1 && errno != EAGAIN) {
                    perror("read error");
                }

                if(nread == 0) //客户端退出
                {
                     ev.data.fd = fd;
                     ev.events = EPOLLIN;
                     if(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev) == -1)
                        cout << "error del" << endl;
                    close(fd);
                    cout << "退出" << endl;
                    continue;
                }
#if 0
                ev.data.fd = fd;
                ev.events = events[i].events | EPOLLOUT;
                if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
                    perror("epoll_ctl: mod");
                }
            }
#endif
#if 1
                cout << "recv:" << buf << endl;

                sprintf(buf, "mess:%s\n", "hello");
                int nwrite, data_size = strlen(buf);
                n = data_size;
                while (n > 0) {
                    nwrite = write(fd, buf + data_size - n, n);
                    if (nwrite < n) {
                        if (nwrite == -1 && errno != EAGAIN) {
                            perror("write error");
                        }
                        break;
                    }
                    n -= nwrite;
                }
            }
#endif

#if 0
            if (events[i].events & EPOLLOUT) {
                sprintf(buf, "get:%lu\n", time(NULL));
                int nwrite, data_size = strlen(buf);
                n = data_size;
                while (n > 0) {
                    nwrite = write(fd, buf + data_size - n, n);
                    if (nwrite < n) {
                        if (nwrite == -1 && errno != EAGAIN) {
                            perror("write error");
                        }
                        break;
                    }
                    n -= nwrite;
                }
                sleep(1);

                ev.data.fd = fd;
                ev.events = events[i].events & (~EPOLLOUT);
                if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
                    perror("epoll_ctl: mod");
                }

//                close(fd);
            }

#endif

        }
    }
    return 0;
}