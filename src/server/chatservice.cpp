#include "chatservice.hpp"
#include "pubilc.hpp"
#include <string.h>
#include <map>
#include <vector>
#include <muduo/base/Logging.h>
using namespace muduo;
using namespace std;

//获取单例对象的接口函数
Chatservice* Chatservice::instance()
{
    static Chatservice service;
    return &service;
} 

//注册消息以及对应的回调方法
Chatservice::Chatservice()
{
    m_msghandlermap.insert({LOGIN_MSG, std::bind(&Chatservice::login, this, _1, _2, _3)});
    m_msghandlermap.insert({LOGINOUT_MSG, std::bind(&Chatservice::loginout, this, _1, _2, _3)});
    m_msghandlermap.insert({REG_MSG, std::bind(&Chatservice::reg, this, _1, _2, _3)});
    m_msghandlermap.insert({ONE_CHAT_MSG, std::bind(&Chatservice::onechat, this, _1, _2, _3)});
    m_msghandlermap.insert({ADD_FRIEND_MSG, std::bind(&Chatservice::addfriend, this, _1, _2, _3)});
    m_msghandlermap.insert({CREATE_GROUP_MSG, std::bind(&Chatservice::creategroup, this, _1, _2, _3)});
    m_msghandlermap.insert({ADD_GROUP_MSG, std::bind(&Chatservice::addgroup, this, _1, _2, _3)});
    m_msghandlermap.insert({GROUP_CHAT_MSG, std::bind(&Chatservice::groupchat, this, _1, _2, _3)});

    // 连接redis服务器
    if (m_redis.connect())
    {
        // 设置上报消息的回调
        m_redis.init_notify_handler(std::bind(&Chatservice::handleRedisSubscribeMessage, this, _1, _2));
    }
}



//获取消息对应的处理器
msghandler Chatservice::gethandler(int msgid)
{
    //记录错误日志，msgid没有对应的事件回调处理
    auto it = m_msghandlermap.find(msgid);
    if(it == m_msghandlermap.end())
    {
        return [=](const TcpConnectionPtr& con, json &js, Timestamp)
        {
            LOG_ERROR << "msgid:" << msgid <<"can not find handler!";
        };
        
    }
    else
    return m_msghandlermap[msgid];
}

//处理登录业务
void Chatservice::login(const TcpConnectionPtr& con, json &js, Timestamp)
{
    int id = js["id"];
    string password = js["password"];

    user user1 = m_usermodel.query(id);
    if(user1.getId() == id && user1.getPassword() == password)
    {
        if(user1.getState() == "online")
        {
            //该用户已经登陆，不允许重复登陆
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "this client is uing, input another client!";
            con->send(response.dump());
        }
        else
        {
            //登录成功，记录用户连接信息
            {
                lock_guard<mutex> lock(m_conmutex);
                m_userconnmap.insert({id, con});            
            }

            // id用户登录成功后，向redis订阅channel(id)
            m_redis.subscribe(id); 

            //登录成功，更新用户状态
            user1.setState("online");
            m_usermodel.updatestste(user1);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user1.getId();
            response["name"] = user1.getName();

            // 查询该用户是否有离线消息
            vector<string> vec = m_offlinemessagemodel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 读取该用户的离线消息后，把该用户的所有离线消息删除掉
                m_offlinemessagemodel.remove(id);
            }


            //查询该用户的好友信息，并返回
            vector<user> uservec = m_friendmodel.query(id);
            if(!uservec.empty())
            {
                vector<string> vec2;
                for(user &user : uservec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }
        

            //查询用户的群组信息
           vector<group> groupuserVec = m_groupmodel.querygroups(id);
            if (!groupuserVec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for (group &group : groupuserVec)
                {
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (groupuser &user : group.getusers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }

                response["groups"] = groupV;
            }

            con->send(response.dump());
        }

    }
    else
    {
        //该用户不存在，或者密码
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "name or password is invalid!";
        con->send(response.dump());
    }
}
//处理注册业务
void Chatservice::reg(const TcpConnectionPtr& con, json &js, Timestamp)
{
    string name = js["name"];
    string password = js["password"];

    user user;
    user.setName(name);
    user.setPassword(password);
    bool state = m_usermodel.insert(user);
    if(state)
    {
        //注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        con->send(response.dump());
    }
    else
    {
        //注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        //报错显示
        
        con->send(response.dump());
    }
}

//处理客户端异常退出
void Chatservice::clientcloseexpection(const TcpConnectionPtr & con)
{
    user user;
    {
        lock_guard<mutex> lock(m_conmutex);
        for(auto it = m_userconnmap.begin(); it != m_userconnmap.end(); ++it)
        {
            if(it->second == con)  
            {
                //从map表中删除连接信息
                user.setId(it->first);
                m_userconnmap.erase(it);
                break;
            }
        }
    }

    // 用户注销，相当于就是下线，在redis中取消订阅通道
    m_redis.unsubscribe(user.getId()); 
    
    //更新用户的状态信息
    if(user.getId() != -1)
    {
        user.setState("offline");
        m_usermodel.updatestste(user);
    }

}



//一对一聊天业务
void Chatservice::onechat(const TcpConnectionPtr& con, json js, Timestamp)
{
    int toid = js["toid"].get<int>();

    {
        lock_guard<mutex> lock(m_conmutex);
        auto it = m_userconnmap.find(toid);
        if(it != m_userconnmap.end())
        {
            //对方用户在线,转发消息
            it->second->send(js.dump());
            return;
        }
    }

    // 查询toid是否在线 
    user user = m_usermodel.query(toid);
    if (user.getState() == "online")
    {
        m_redis.publish(toid, js.dump());
        return;
    }

    //对方用户不在线,存储离线消息
    m_offlinemessagemodel.inster(toid, js.dump());
}

//服务器异常后，业务重置方法
void Chatservice::reset()
{
    //把online状态的用户，设置成offline
    m_usermodel.resetstate();
}


//添加好友业务
void Chatservice::addfriend(const TcpConnectionPtr& con, json js, Timestamp)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    //存储好友信息
    m_friendmodel.insert(userid, friendid);
}

//创建群组业务
void Chatservice::creategroup(const TcpConnectionPtr &con, json js, Timestamp)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    //存储新创建的群组信息
    group group(-1, name, desc);
    if(m_groupmodel.creategroup(group))
    {
        //存储群创建人的信息
        m_groupmodel.addGroup(userid, group.getId(), "creator");
    }
}

//加入群组业务
void Chatservice::addgroup(const TcpConnectionPtr &con, json js, Timestamp)
{
    int userid = js["id"].get<int>();
    int groupid= js["groupid"].get<int>();
    m_groupmodel.addGroup(userid, groupid, "normal");
}

//群组聊天业务
void Chatservice::groupchat(const TcpConnectionPtr &con, json js, Timestamp)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridvec = m_groupmodel.querygroupusers(userid, groupid);


   lock_guard<mutex> lock(m_conmutex);
    for (int id : useridvec)
    {
        auto it = m_userconnmap.find(id);
        if (it != m_userconnmap.end())
        {
            // 转发群消息
            it->second->send(js.dump());
        }
        else
        {
            // 查询toid是否在线 
            user user = m_usermodel.query(id);
            if (user.getState() == "online")
            {
                m_redis.publish(id, js.dump());
            }
            else
            {
                // 存储离线群消息
                m_offlinemessagemodel.inster(id, js.dump());
            }
        }
    }
}


// 处理注销业务
void Chatservice::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(m_conmutex);
        auto it = m_userconnmap.find(userid);
        if (it != m_userconnmap.end())
        {
            m_userconnmap.erase(it);
        }
    }

    //用户注销，相当于就是下线，在redis中取消订阅通道
    m_redis.unsubscribe(userid); 

    // 更新用户的状态信息
    user user(userid, "", "", "offline");
    m_usermodel.updatestste(user);
}


// 从redis消息队列中获取订阅的消息
void Chatservice::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(m_conmutex);
    auto it = m_userconnmap.find(userid);
    if (it != m_userconnmap.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    m_offlinemessagemodel.inster(userid, msg);
}