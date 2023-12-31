#include "groupmodel.hpp"
#include "db.h"

//创建群组
bool groupmodel::creategroup(group &group)
{
    //1组装sql语句
    char sql[4096] = {0};
    sprintf(sql, "insert into ALLGroup(groupname, groupdesc) value('%s', '%s')", 
            group.getName().c_str(), group.getDesc().c_str());

    //插入数据库
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            //获取插入成功的用户id
            group.setId(mysql_insert_id(mysql.getconnect()));
            return true;
        }
    }
    return false;
}
//加入群组
void groupmodel::addGroup(int userid, int groupid, string role)
{
    //1组装sql语句
    char sql[4096] = {0};
    sprintf(sql, "insert into GroupUser(groupid, userid, grouprole) value('%d', '%d', '%s')", groupid, userid, role.c_str());

    //插入数据库
    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }

}
//查询用户所在群组信息
vector<group> groupmodel::querygroups(int userid)
{
    //1组装sql语句
    //先根据userid查询该用户的所属群组信息
    //再根据群组信息，查询属于该群组的所有用户的userid,并且和user表联合查询。查出用户的详细信息
    char sql[4096] = {0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from ALLGroup a inner join GroupUser b on a.id = b.groupid where b.userid = %d", userid);

    vector<group> groupvec;

    //插入数据库
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            //查出userid所有的群组信息
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                group group1;
                group1.setId(atoi(row[0]));
                group1.setName(row[1]);
                group1.setDesc(row[2]);
                groupvec.push_back(group1);
            }
            mysql_free_result(res);
        }
    }
    for(group &group2 : groupvec)
    {
        sprintf(sql, "select a.id,a.name,a.state,b.grouprole from user a inner join GroupUser b on b.userid = a.id where b.groupid=%d", group2.getId());

        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                groupuser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group2.getusers().push_back(user);
                
            }
            mysql_free_result(res);
        }
    }
    return groupvec;

}
//根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其他成员群发消息
vector<int> groupmodel::querygroupusers(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from GroupUser where groupid = %d and userid != %d", groupid, userid);

    vector<int> idvec;
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                idvec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return idvec;
}