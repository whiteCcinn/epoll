#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

// 服务端口
#define SERVER_PORT 9601

// 服务IP
#define SERVER_IP "127.0.0.1"

// 已完成三次握手的队列长度
#define BACKLOG 128

// 可以在epoll时间机制中注册多少个fd
#define EPOLL_SIZE 5000

// 客户端连接上了之后服务端默认发一条消息给客户端
#define SERVER_WELCOME "Hi,I am server, Welcome\n"

#define BUF_SIZE 0xFFFF

/**
  * @param sockfd: socket descriptor
  **/
void setnonblocking(int sockfd)
{
  fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK);
}

/**
  * @param epollfd: epoll handle
  * @param fd: socket descriptor
  * @param enable_et : enable_et = true, epoll use ET; otherwise LT
  **/
void addfd(int epollfd, int fd, int enable_et)
{
  struct epoll_event ev;
  ev.data.fd = fd;
  ev.events = EPOLLIN;
  if (enable_et)
    ev.events = EPOLLIN | EPOLLET;
  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
  setnonblocking(fd);
  printf("fd added to epoll!\n\n");
}

/**
 *  
 **/
void handler(struct epoll_event event, int epollfd, int fd)
{

  if (event.events & (EPOLLERR | EPOLLHUP))
  {
    printf("触发错误，对端已经关闭，关闭本端fd\n");
    close(fd);
    return;
  }

  if (event.events & EPOLLIN)
  {
    printf("触发EPOLLIN\n");
    /* 接入的socket有数据可读 */
    while (1)
    {
      ssize_t count;
      char buf[512];
      count = read(fd, buf, sizeof buf);
      if (count == -1)
      {
        if (errno != EAGAIN)
        {
          printf("read\n");
          close(fd);
        }
        break;
      }
      else if (count == 0)
      {
        /* 数据读取完毕，结束 */
        close(fd);
        printf("Closed connection on descriptor %d\n", fd);
        break;
      }
      /* 输出到stdout */
      int s = write(1, buf, count);
      if (s == -1)
      {
        printf("write\n");
        abort();
      }

      // char message[BUF_SIZE] = "process ";
      // strcat(message, buf);
      // int ret = send(fd, message, BUF_SIZE, 0);

      event.events = EPOLLET | EPOLLOUT;
      epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
    }
  }

  if (event.events & EPOLLOUT)
  {
    printf("触发EPOLLOUT\n");
    write(fd, "it's echo man\n", 14);
    event.events = EPOLLET | EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
  }
  // else
  // {
  //   printf("触发未知错误：%d\n", event.events);
  // }
}