#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"
#include <functional>
#include <string>
using namespace placeholders;
using json = nlohmann::json;


//初始化聊天服务器对象
chatserver::chatserver(EventLoop* loop, const InetAddress& listenaddr, const string& nameArg):
                        m_server(loop, listenaddr, nameArg), m_loop(loop)
                        {
                            //注册连接回调
                            m_server.setConnectionCallback(bind(&chatserver::onconnection, this, _1));
                            //注册通信消息回调函数
                            m_server.setMessageCallback(bind(&chatserver::onmassage, this, _1, _2, _3));
                            //设置线程数量
                            m_server.setThreadNum(4);
                        }

//启动服务
void chatserver::start()
{
    m_server.start();
}

//执行连接相关的回调函数
void chatserver::onconnection(const TcpConnectionPtr& con)
{
    //用户断开连接
    if(!con->connected())
    {
        Chatservice::instance()->clientcloseexpection(con);
        con->shutdown();
    }
}

//执行消息通信相关的回调函数
void chatserver::onmassage(const TcpConnectionPtr& con,
                        Buffer* buffer,
                        Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    //json的反序列化
    json js = json::parse(buf);
    //达到完全解耦网络模块和业务模块的代码
    //通过js["msgid"]获取->业务hander->将js, con, time,传入业务hander，由业务hander去处理
    auto msghandler =  Chatservice::instance()->gethandler(js["msgid"].get<int>());
    //回调，通过msgid绑定的不同的事件处理器，来进行相应的业务处理
    msghandler(con, js, time);
}