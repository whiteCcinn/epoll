#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>

// 服务端口
#define SERVER_PORT 9601

// 服务IP
#define SERVER_IP "127.0.0.1"

// 已完成三次握手的队列长度
#define BACKLOG 128

// 可以在epoll时间机制中注册多少个fd
#define EPOLL_SIZE 5000

// 客户端连接上了之后服务端默认发一条消息给客户端
#define SERVER_WELCOME "welcome message"

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
void addfd(int epollfd, int fd, bool enable_et)
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
  if (event.events & EPOLLIN)
  {
    printf("触发EPOLLIN\n");
    if (fd < 0)
    {
      return;
    }
    int size_t n;
    if ()
    {
    }
    event.events = EPOLL | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
  }
  else if (event.events & EPOLLOUT)
  {
    printf("触发EPOLLOUT\n");
    /* 接入的socket有数据可读 */
    while (1)
    {
      ssize_t count;
      char buf[512];
      count = read(fd, buf, sizeof(buf));
      if (count == -1)
      {
        if (errno != EAGAIN)
        {
          printf("读取错误，并且errno不等于EAGAIN：没提示需要再次读取\n");
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
      s = write(1, buf, count);
      if (s == -1)
      {
        printf("写入错误\n");
        abort();
      }
      event.events = EPOLLIN | EPOLLET;
      epoll_ctl(epollfd, EPOLL_CTL_MOD, efd, &event);
    }
  }
  else if (event.events & (EPOLLERR | EPOLLHUP))
  {
    printf("触发错误，对端已经关闭，关闭本端fd\n");
    close(fd);
    continue;
  }
  else
  {
    printf("触发未知错误：%d\n", event.events);
  }
}