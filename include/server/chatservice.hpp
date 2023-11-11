#ifndef CHATSERVICE_HPP
#define CHATSERVICE_HPP

#include "redis.hpp"
#include "json.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include <muduo/net/TcpServer.h>
#include <mutex>
#include <unordered_map>
#include <functional>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;
//表示处理消息的事件回调方法类型
using msghandler = function<void(const TcpConnectionPtr& con, json &js, Timestamp)>;


//聊天服务器业务类
class Chatservice
{
public:
    //获取单例对象的接口函数
    static Chatservice* instance();

    //处理登录业务
    void login(const TcpConnectionPtr& con, json &js, Timestamp);

    //处理注册业务
    void reg(const TcpConnectionPtr& con, json &js, Timestamp);

    // 处理注销业务
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //获取消息对应的处理器
    msghandler gethandler(int msgid);

    //处理客户端异常退出
    void clientcloseexpection(const TcpConnectionPtr & con);

    //一对一聊天业务
    void onechat(const TcpConnectionPtr& con, json js, Timestamp);

    //服务器异常后，业务重置方法
    void reset();

    //添加好友业务
    void addfriend(const TcpConnectionPtr& con, json js, Timestamp);

    //创建群组业务
    void creategroup(const TcpConnectionPtr &con, json js, Timestamp);

    //加入群组业务
    void addgroup(const TcpConnectionPtr &con, json js, Timestamp);

    //群组聊天业务
    void groupchat(const TcpConnectionPtr &con, json js, Timestamp);

    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int, string);


private:
    Chatservice();

    //存储消息id和其对应的业务处理方法
    unordered_map<int, msghandler> m_msghandlermap;

    //数据操作类的对象
    Usermodel m_usermodel;

    //存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> m_userconnmap;

    //定义互斥锁，保证m__userconnmap的线程安全
    mutex m_conmutex;

    //离线消息类的操作对象
    offlinemessagemodel m_offlinemessagemodel;

    //好友列表类的操作对象
    friendmodel m_friendmodel;

    //群组类的操作对象
    groupmodel m_groupmodel;
    
    // redis操作对象
    Redis m_redis;
    
};



#endif