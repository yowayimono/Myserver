/*
 * @Author: yowayimono 3485000346@qq.com
 * @Date: 2023-03-18 09:28:31
 * @LastEditors: yowayimono 3485000346@qq.com
 * @LastEditTime: 2023-03-19 10:38:40
 * @FilePath: /Myserver/http/httpconn.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef HTTPCONN_H
#define HTTPCONN_H
//处理请求

#include <sys/types.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#include "../buffer/buffer.h"
#include "spdlog/spdlog.h"
#include "httprequest.h"
#include "httpresponse.h"


class HttpConn{
public:
    HttpConn();
    ~HttpConn();

    void Init(int sockFd,const sockaddr_in& addr);

    ssize_t read(int* saveErrno);

    ssize_t write(int* saveErrno);
    

    void Close();

    int GetFd() const;

    const char* GetIP() const;

    int GetPort() const;

    sockaddr_in GetAddr() const;

    bool process();

    int ToWriteBytes(){
        return iov_[0].iov_len+iov_[1].iov_len;
    }

    bool IsKeepAlive() const{
        return request_.IsKeepAlive();
    }

    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;
private:

    int fd_;

    struct sockaddr_in addr_;

    bool isClose_;

    int iovCnt_;

    struct iovec iov_[2];

    Buffer readBuff_;
    Buffer writeBuff_;

    HttpRequest request_;
    HttpResponse response_;


};
#endif