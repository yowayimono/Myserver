/*
 * @Author: yowayimono 3485000346@qq.com
 * @Date: 2023-03-19 14:40:24
 * @LastEditors: yowayimono 3485000346@qq.com
 * @LastEditTime: 2023-03-19 14:51:55
 * @FilePath: /Myserver/server/epoller.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <errno.h>

class Epoller{
public:
    explicit Epoller(int maxEvent=1024);

    ~Epoller();

    bool AddFd(int fd,uint32_t events);
    bool ModFd(int fd,uint32_t events);

    bool DelFd(int fd);

    int Wait(int timeoutMs=-1);

    int GetEventFd(size_t i) const;

    uint32_t GetEvents(size_t i) const;


private:
    int epollFd_;
    std::vector<struct epoll_event> events_;

};

#endif //EPOLLER_H