#ifndef OFFLINEMESSAGEMODEL_HPP
#define OFFLINEMESSAGEMODEL_HPP
#include <string>
#include <vector>
using namespace std;

//提供理想消息表的操作接口方法
class offlinemessagemodel
{
public:
    //储存用户离线消息
    void inster(int userid, string msg);

    //删除用户的离线消息
    void remove(int userid);

    //查询用户的离线消息
    vector<string> query(int userid);
private:

};


#endif