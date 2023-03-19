/*
 * @Author: yowayimono 3485000346@qq.com
 * @Date: 2023-03-18 09:29:25
 * @LastEditors: yowayimono 3485000346@qq.com
 * @LastEditTime: 2023-03-19 10:38:35
 * @FilePath: /Myserver/http/httpresponse.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H


#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <spdlog/spdlog.h>


#include "../buffer/buffer.h"


class HttpResponse{
public:
    HttpResponse();//默认构造函数
    ~HttpResponse();//默认析构函数

    //初始化HttpResponse对象路径，文件类型，响应码等信息
    void Init(const std::string& srcDir,std::string& path,std::unordered_map<std::string,int> post_,bool iskeepAlive=false,int code=-1);
    void MakeResponse(Buffer& buff);//构建HTTP响应，并添加至缓冲区
    void UnmapFile();//解除内存映射
    char* File();//获取文件内存映射指针
    size_t FileLen() const;//获取文件长度
    void ErrorContent(Buffer& buff,std::string message);//构建错误响应正文
    int Code() const {return code_;}//获取响应状态吗


    void AddPostContent_(Buffer& buff);//构建post请求表单数据消息体


private:
    void AddStateLine_(Buffer& buff);//添加HTTP响应状态行
    void AddHeader_(Buffer& buff);//添加Http响应头
    void AddContent_(Buffer& buff);//添加HTTP响应消息体


    void ErrorHtml_();//构建错误响应信息的HTML页面
    std::string GetFileType_();//获取文件类型

    int code_;//响应状态吗
    bool isKeepAlive_;//是否保持长链接


    std::string path_;//文件的路径
    std::string srcDir_;//文件的目录

    char* mmFile_;//文件内存映射的指针
    struct stat mmFileStat_;//文件的状态信息

    std::unordered_map<std::string,int> post__;//请求表单数据
    //分别表示文件后缀名，响应状态吗，响应状态码对应的路径的映射表
    static const std::unordered_map<std::string,std::string> SUFFIX_TYPE;
    static const std::unordered_map<int,std::string> CODE_STATUS;
    static const std::unordered_map<int,std::string> CODE_PATH;

};


#endif