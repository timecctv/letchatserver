#ifndef FRIENDMODEL_HPP
#define FRIENDMODEL_HPP


#include "user.hpp"
#include <vector>
using namespace std;

//维护好友信息的操作接口方法
class friendmodel
{
public:
    //添加好友的关系
    bool insert(int userid, int friendid);

    //返回用户好友列表
    vector<user> query(int userid);
private:

};





#endif