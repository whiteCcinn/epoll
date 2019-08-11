#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// 服务端口
#define SERVER_PORT 9601

// 服务IP
#define SERVER_IP "127.0.0.1"

int main(int argc, char *argv[])
{
  // 服务端 IP + PORT
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = PF_INET;
  serverAddr.sin_port = htons(SERVER_PORT);
  serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

  // 创建监听socket
  int listener = socket(PF_INET, SOCK_STREAM, 0);
  if (listener < 0)
  {
    printf("listener error\n");
    exit(-1);
  }

  // 绑定地址
  if (bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
  {
  }
}