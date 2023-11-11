#include "chatserver.hpp"
#include "chatservice.hpp"
#include <signal.h>

//处理服务器ctrl+c结束,重置user的状态
void resetHandler(int)
{
    Chatservice::instance()->reset();
    exit(0);
}
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatServer 192.168.29.141 9999" << endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress addr("192.168.29.141", 9999);
    chatserver server(&loop, addr, "聊天服务器");

    server.start();
    loop.loop();

    return 0;
}