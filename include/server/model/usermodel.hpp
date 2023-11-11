#ifndef USERMODEL_HPP
#define USERMODEL_HPP
#include "user.hpp"

//User表的数据操作类
class Usermodel
{
public:
    //User表增加的方法
    bool insert(user &user);

    //根据用户提供的id，查找user表中的数据
    user query(int id);

    //更改用户状态
    bool updatestste(user user);

    //重置用户的状态信息
    bool resetstate();
    

private:

};



#endif