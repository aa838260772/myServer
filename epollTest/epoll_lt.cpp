/*
*本测试文件接受一次输入则输出一次hello
*LT模式下缓冲区有内容未读会一直通知
time:2016-04-03
*/

#include <unistd.h>
#include <iostream>
#include <sys/epoll.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

using namespace std;

int main(void)
{
    int epfd,nfds;
    struct epoll_event ev,events[5];//ev用于注册事件，数组用于返回要处理的事件
    epfd=epoll_create(1);//只需要监听一个描述符——标准输入
    ev.data.fd=STDIN_FILENO;
    ev.events=EPOLLIN;//监听读状态同时设置ET模式
    epoll_ctl(epfd,EPOLL_CTL_ADD,STDIN_FILENO,&ev);//注册epoll事件
    char buf[5] = {0};
    for(;;)
   {
     nfds=epoll_wait(epfd,events,5,-1);
     for(int i=0;i<nfds;i++)
     {
        if(events[i].data.fd==STDIN_FILENO)
        {
            cout<<"hello world!";
#if 0
            int n = read(STDIN_FILENO,buf,5);
            if(n)
            cout << buf << endl;
            memset(buf,'\0',5);
#endif
            usleep(100000);
        }
     }
   }
}
