#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

// 服务端口
#define SERVER_PORT 9601

// 服务IP
#define SERVER_IP "127.0.0.1"

// 已完成三次握手的队列长度
#define BACKLOG 128

// 可以在epoll时间机制中注册多少个fd
#define EPOLL_SIZE 5000

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

int main(int argc, char *argv[])
{
  // 服务端 IP + PORT
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = PF_INET;
  serverAddr.sin_port = htons(SERVER_PORT);
  serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

  // 创建监听socket
  int listenfd = socket(PF_INET, SOCK_STREAM, 0);
  if (listenfd < 0)
  {
    printf("创建服务端文件描述符失败\n");
    exit(-1);
  }

  // 绑定地址
  if (bind(listenfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
  {
    printf("文件描述符和服务端IP端口绑定失败\n");
    exit(-1);
  }

  // 监听
  int ret = listen(listenfd, BACKLOG);
  if (ret < 0)
  {
    printf("服务端文件描述符状态修改为listen状态失败，backlog手动指定队列大小为：%d\n", BACKLOG);
    exit(-1);
  }

  // 在内核中创建epoll事件表

  int epfd = epoll_create(EPOLL_SIZE);
  if (epfd < 0)
  {
    printf("epoll 文件描述符创建失败\n");
    exit(-1);
  }

  printf("epoll created, epollfd = %d\n", epfd);

  // 定义需要注册到epoll事件
  static struct epoll_event events[EPOLL_SIZE];

  // 注册epoll事件，往内核事件表里添加事件
  addfd(epfd, listenfd, true);

  while (1)
  {
    //epoll_events_count表示就绪事件的数目，一般为0或者大于0
    int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
    if (epoll_events_count < 0)
    {
      perror("epoll就绪事件小于0");
      break;
    }

  }
}