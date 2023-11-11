#include "friendmodel.hpp"
#include "db.h"
#include <vector>
using namespace std;



//添加好友的关系
bool friendmodel::insert(int userid, int friendid)
{
    //1组装sql语句
    char sql[4096] = {0};
    sprintf(sql, "insert into Friend value(%d, %d)", userid, friendid);

    //插入数据库
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            //获取插入成功的用户id
            return true;
        }
    }
    return false;
}

//返回用户好友列表
vector<user> friendmodel::query(int userid)
{
    //1组装sql语句
    char sql[4096] = {0};
    sprintf(sql, "select a.id,a.name,a.state from user a inner join Friend b on b.friendid = a.id where b.userid = %d", userid);

    //查找数据库
    vector<user> vec;
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while(row = mysql_fetch_row(res))
            {
                user user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;

        }
       
    }
    return  vec;
}