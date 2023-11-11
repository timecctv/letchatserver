#include "usermodel.hpp"
#include "db.h"
#include <iostream>
using namespace std;


//User表的增加方法
bool Usermodel::insert(user &user)
{
    //1组装sql语句
    char sql[4096] = {0};
    sprintf(sql, "insert into user(name, password, state) value('%s', '%s', '%s')", 
            user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());

    //插入数据库
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            //获取插入成功的用户id
            user.setId(mysql_insert_id(mysql.getconnect()));
            return true;
        }
    }
    return false;
}


//根据用户提供的id，查找user表中的数据
user Usermodel::query(int id)
{

    //1组装sql语句
    char sql[4096] = {0};
    sprintf(sql, "select * from user where id = %d", id);

    //查找数据库
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if(row != nullptr)
            {
                user user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);

                mysql_free_result(res);
                return user;
            }
        }
       
    }

    return  user();

}




//更改用户状态
bool Usermodel::updatestste(user user)
{
    //1组装sql语句
    char sql[4096] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", 
            user.getState().c_str(), user.getId());

    //插入数据库
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

//重置用户的状态信息
bool Usermodel::resetstate()
{
        //1组装sql语句
    char sql[1028] = "update user set state = 'offline' where state = 'online'";

    //插入数据库
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}