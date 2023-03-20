/*
 * @Author: yowayimono 3485000346@qq.com
 * @Date: 2023-03-19 21:03:50
 * @LastEditors: yowayimono 3485000346@qq.com
 * @LastEditTime: 2023-03-20 09:33:22
 * @FilePath: /Myserver/server/webserver.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <string>
#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



#include "spdlog//spdlog.h"
#include "epoller.h"
#include "Threadpool.h"
#include "../http/httpconn.h"


class WebServer{
    
public:
    WebServer(
        int port,int trigMode,
        int connPoolNum,int threadNum);
    

    ~WebServer();


    void _Start();


private:

    bool InitSocket_();
    void InitEventMode_(int trigMode);
    void AddClient_(int fd,sockaddr_in addr);


    void DealListen_();
    void DealWrite_(HttpConn* client);
    void DealRead_(HttpConn* client);


    void SendError_(int fd,const char* info);//向服务端发送错误
    void CloseConn_(HttpConn* client);//关闭链接


    void onRead_(HttpConn* client);//处理读写，业务逻辑
    void onWrite_(HttpConn* client);
    void onProcess(HttpConn* client);

    static const int MAX_FD=65536;

    static int SetFdNonblock(int fd);//设置非阻塞

    int port_;//端口号
    bool isClose_;//是否关闭
    int listenFd_;//监听描述符
    char* srcDir_;//资源目录


    uint32_t listenEvent_;
    uint32_t connEvent_;

    std::unique_ptr<ThreadPool> threadpool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int,HttpConn> users_;//

};
#endif