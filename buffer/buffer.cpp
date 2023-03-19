#include "buffer.h"
Buffer::Buffer(int initsize):buffer_(initsize),readPos_(0),writePos_(0){}

size_t Buffer::WritableBytes() const{
    return buffer_.size()-writePos_;//返回可写入大小
}

size_t Buffer::ReadableBytes() const{//返回可读入大小
    return writePos_-readPos_;
}

size_t Buffer::PrependableBytes() const{
    return readPos_;
}

const char* Buffer::Peek() const{
    return BeginPtr_()+readPos_;
}

void Buffer::EnsurWritable(size_t len)
{
    if(WritableBytes()<len){
        MakeSpace_(len);
    }
    assert(WritableBytes()>=len);//确保缓冲区能写入这么长的数路
}

void Buffer::HasWritten(size_t len){
    writePos_+=len;//更新缓冲区已写入长度
}

void Buffer::Retrieve(size_t len)
{//读取指针后移
    assert(len<=ReadableBytes());//移除已读取得len数据
    readPos_+=len;//更新已读取长度
}

void Buffer::RetrieveUntil(const char* end)
{
    assert(Peek()<=end);//移除缓冲区头指针到end间的数据
    Retrieve(end-Peek());
}


void Buffer::RetrieveAll(){//标记全部已读
    bzero(&buffer_[0],buffer_.size());
    readPos_=0;
    writePos_=0;
}

const char* Buffer::BeginWriteConst() const{
    return BeginPtr_()+writePos_;//返回可写位置的指针
}

char* Buffer::BeginWrite(){//返回可写位置指针并开始写入
    return BeginPtr_()+writePos_;
}

void Buffer::Append(const std::string& str)
{
    Append(str.data(),str.length());
}

void Buffer::Append(const char* str,size_t len){
    assert(str);
    EnsurWritable(len);
    std::copy(str,str+len,BeginWrite());
    HasWritten(len);
}

ssize_t Buffer::ReadFd(int fd,int* saveErrno)
{//往缓冲区写入从客户端读取的数据，成功返回读取字节数，失败返回错误吗
    char buff[65535];
    struct iovec iov[2];
    const size_t writable=WritableBytes();
    iov[0].iov_base=BeginPtr_()+writePos_;
    iov[0].iov_len=writable;
    iov[1].iov_base=buff;
    iov[1].iov_len=sizeof(buff);

    const ssize_t len=readv(fd,iov,2);
    if(len<0){
        *saveErrno=errno;

    }
    else if(static_cast<size_t>(len)<=writable){
        writePos_+=len;//更新已读数据长度
    }
    else{
        writePos_=buffer_.size();
        Append(buff,len-writable);
    }

    return len;//返回读入数据长度
}

char* Buffer::BeginPtr_(){
    return &*buffer_.begin();
}

const char* Buffer::BeginPtr_() const{
    return &*buffer_.begin();
}

void Buffer::MakeSpace_(size_t len)
{
    if(WritableBytes()+PrependableBytes()<len)
    {//剩余可写大小加前面可用空间小于临时数组大小，扩容
        buffer_.resize(writePos_+len+1);
    }
    else{//可以装len大小,则直接将后面的数据拷贝到前面
        size_t readable=ReadableBytes();
        std::copy(BeginPtr_()+readPos_,BeginPtr_()+writePos_,BeginPtr_());
        readPos_=0;
        writePos_=readPos_+readable;
        assert(readable==ReadableBytes());
    }
}