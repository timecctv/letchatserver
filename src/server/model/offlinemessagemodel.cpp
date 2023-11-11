#include "offlinemessagemodel.hpp"
#include "db.h"


//储存用户离线消息
void offlinemessagemodel::inster(int userid, string msg)
{
    //1组装sql语句
    char sql[4096] = {0};
    sprintf(sql, "insert into OfflineMessage value('%d', '%s')", userid, msg.c_str());

    //插入数据库
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            return;
        }
    }

}

//删除用户的离线消息
void offlinemessagemodel::remove(int userid)
{
    //1组装sql语句
    char sql[4096] = {0};
    sprintf(sql, "delete from OfflineMessage where userid=%d", userid);

    //插入数据库
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            return;
        }
    }
}

//查询用户的离线消息
vector<string> offlinemessagemodel::query(int userid)
{
    //1组装sql语句
    char sql[4096] = {0};
    sprintf(sql, "select massage from OfflineMessage where userid = %d", userid);

    //查找数据库
    vector<string> vec;
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            //把用户的所有消息放入vector中返回
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
            return vec;
        }
       
    }
    return vec;

}
