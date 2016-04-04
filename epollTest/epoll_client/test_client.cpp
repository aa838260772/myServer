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
#include <unistd.h>
#include <iostream>

using namespace std;

void do_client(void *fd)
{
#if 0
    for (int i = 0; i < 2; i++) {
        close(i);
    }
#endif
    int client_fd = ::socket(AF_INET,SOCK_STREAM,0);
    if(client_fd == -1)
        cout << "error socket" << endl;
    struct sockaddr_in srv_add;
    srv_add.sin_family = AF_INET;
    srv_add.sin_addr.s_addr = inet_addr("192.168.1.52");
    srv_add.sin_port = htons(8999);
    socklen_t sock_length = sizeof(struct sockaddr_in);

//    int client_fd = *(int*)fd;
    int ret = connect(client_fd, (struct sockaddr*)&srv_add, sock_length);
    if(-1 == ret)
        cout << "error connect" << endl;

    char buf[1024];
    while(1)
    {
        memset(buf,0,1024);
        sprintf(buf, "%lu:%lu", getpid(), time(NULL));
        ::write(client_fd, buf, strlen(buf));
//        cout << "send-->" << buf << endl;
        ::read(client_fd,buf,1024);
//        cout << getpid() << "recv-->" << buf << endl;

#if 0
        int on = 1;
        if(0>=send(client_fd, buf, strlen(buf), on))
            cout << "error send" << endl;
        if(0<=recv(client_fd, buf, 1024, on))
            cout << buf << endl;
#endif
        sleep(1);
    }
}

int main(int argc, const char *argv[])
{
#if 1
    for (int i = 0; i < 100; i++) {
        pid_t pid;
        if((pid=fork()) == 0)
        {
            do_client(NULL);
        }else if(pid > 0)
        {
            cout << "fork success " << pid
                 << " " << i
                 << endl;
        }
    }
#endif
#if 0
    int client_fd = ::socket(AF_INET,SOCK_STREAM,0);
    if(client_fd == -1)
        cout << "error socket" << endl;
    struct sockaddr_in srv_add;
    srv_add.sin_family = AF_INET;
    srv_add.sin_addr.s_addr = inet_addr("192.168.1.52");
    srv_add.sin_port = htons(8999);
    socklen_t sock_length = sizeof(struct sockaddr_in);

    int ret = connect(client_fd, (struct sockaddr*)&srv_add, sock_length);
    if(-1 == ret)
        cout << "error connect" << endl;

    char buf[1024];
    while(1)
    {
        memset(buf,0,1024);
        cout << "input "<< endl;
        cin >> buf;
        int on = 1;

        if(0>=send(client_fd, buf, strlen(buf), on))
            cout << "error send" << endl;

        if(0>=recv(client_fd, buf, 1024, on))
            cout << buf << endl;
    }
#endif
    waitpid(-1, NULL, 0);


    return 0;
}
