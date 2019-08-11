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

// 客户端连接上了之后服务端默认发一条消息给客户端
#define SERVER_WELCOME "welcome message"

#define BUF_SIZE 0xFFFF

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
  // setnonblocking(fd);
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

  // 注册epoll事件，往内核事件表里添加事件
  addfd(epfd, listenfd, true);

  // 定义需要注册到epoll事件
  static struct epoll_event events[EPOLL_SIZE];

  while (1)
  {
    //epoll_events_count表示就绪事件的数目，一般为0或者大于0
    int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
    if (epoll_events_count < 0)
    {
      printf("epoll就绪事件小于0");
      break;
    }

    // 处理这epoll_events_count个就绪事件
    for (int i = 0; i < epoll_events_count; ++i)
    {
      int sockfd = events[i].data.fd;
      //新用户连接
      if (sockfd == listenfd)
      {
        struct sockaddr_in client_address;
        socklen_t client_addrLength = sizeof(struct sockaddr_in);
        int clientfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrLength);
        printf("client connection from: %s : % d(IP : port), clientfd = %d \n",
               inet_ntoa(client_address.sin_addr),
               ntohs(client_address.sin_port),
               clientfd);

        addfd(epfd, clientfd, true); ////把这个新的客户端添加到内核事件列表

        // 服务端用list保存用户连接
        // clients_list.push_back(clientfd);
        // printf("Add new clientfd = %d to epoll\n", clientfd);
        // printf("Now there are %d clients int the chat room\n", (int)clients_list.size());

        // 服务端发送欢迎信息
        printf("welcome message\n");
        char message[BUF_SIZE];
        bzero(message, BUF_SIZE);
        sprintf(message, SERVER_WELCOME, clientfd);
        int ret = send(clientfd, message, BUF_SIZE, 0);
        if (ret < 0)
        {
          printf("send welcome error\n");
          exit(-1);
        }
      }
      else
      {
      }
    }

    close(listenfd); //关闭socket
    close(epfd);     //关闭内核   不在监控这些注册事件是否发生

    return 0;
  }