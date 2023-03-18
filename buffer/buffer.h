#ifndef BUFFER_H
#define BUFFER_H
#include <syscall.h>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <atomic>
#include <assert.h>
#include <sys/uio.h>
#include <iostream>


class Buffer{
    public:
    Buffer(int initsize=1024);
    ~Buffer()=default;


    size_t WritableBytes() const;//返回缓冲区可写字节数
    size_t ReadableBytes() const;//返回缓冲区可读字节数
    size_t PrependableBytes() const;//返回缓冲区头部剩余字节

    const char* Peek() const;//返回当前缓冲区头指针
    void EnsurWritable(size_t len);//确保当前缓冲区能写len数据
    void HasWritten(size_t len);//当前已写入长度
    void Retrieve(size_t len);//删除已读取的len数据
    void RetrieveUntil(const char* end);//
    void RetrieveAll();//移除所有数据


    const char* BeginWriteConst() const;//读状态返回尾指针
    char* BeginWrite();//可写状态返回尾指针
    void Append(const std::string& str);
    void Append(const char* str,size_t len);//向缓冲区添加数据，三种数据，void*,string,char*
    void Append(const void* data,size_t len);

    ssize_t ReadFd(int fd,int* Errno);//从服务端接受数据到缓冲区
    ssize_t WriteFd(int fd,int* Errno);//将数据发送到服务端


    private:
    char* BeginPtr_();
    const char* BeginPtr_() const;
    void MakeSpace_(size_t len);//容量不足以写入数据，扩容

    std::vector<char> buffer_;//缓冲区数组
    std::atomic<std::size_t> readPos_;//当前已读入
    std::atomic<size_t> writePos_;//当前已写入
};


#endif