# letchatserver
一个工作在nginx_tcp中的集群聊天服务器，数据库用mysql部署，网络通信采用muduo和json消息作为通信协议


删除cd build文件夹中的文件\
然后
cmake ..
make

需要nginx的tcp负载均衡，redis中间件, mysql数据库，muduo网络库

