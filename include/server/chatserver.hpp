#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
using namespace std;
using namespace muduo;
using namespace muduo::net;

//聊天服务器主类
class chatserver
{
public:
    //初始化聊天服务器对象
    chatserver(EventLoop* loop, const InetAddress& listenaddr, const string& nameArg);
    //启动服务
    void start();

private:
    //执行连接相关的回调函数
    void onconnection(const TcpConnectionPtr&);

    //执行消息通信相关的回调函数
    void onmassage(const TcpConnectionPtr&,
                            Buffer*,
                            Timestamp);


    TcpServer m_server;//结合muduo库实现的服务器的类对象
    EventLoop* m_loop;//指向事件循环对象的指针

};


#endif