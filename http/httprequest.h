/*
 * @Author: yowayimono 3485000346@qq.com
 * @Date: 2023-03-18 09:28:52
 * @LastEditors: yowayimono 3485000346@qq.com
 * @LastEditTime: 2023-03-18 17:12:10
 * @FilePath: /Myserver/http/httprequest.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H
//封装用于解析http请求的类

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <regex>
#include <spdlog/spdlog.h>
#include <errno.h>
#include "../buffer/buffer.h"

class HttpRequest{
public:
    HttpRequest(){Init();}
    ~HttpRequest()=default;
    enum PARSE_STATE{
        REQUEST_LINE,//正在解析请求行
        HEADERS,//正在解析请求头
        BODY,//正在解析请求体
        FINISH,//解析完成
    };

    enum HTTP_CODE{//表示处理结果
        NO_REQUEST=0,//无http请求
        GET_REQUEST,//GET请求
        BAD_REQUEST,//HTTP请求语法错误
        NO_RESOURSE,//请求资源未找到
        FORBIDDENT_REQUEST,//访问被拒绝
        FILE_REQUEST,//文件请求
        INTERNAL_ERROR,//服务器内部错误
        CLOSE_CONNECTION,//链接被关闭
    };

    void Init();//初始化对象状态和相关变量
    bool parse(Buffer& buff);//开始解析请求


    std::string method() const;//返回请求方法
    std::string path() const;//返回请求路径
    std::string& path();//返回请求路径引用
    std::string version() const;//返回http版本

    bool IsKeepAlive() const;//是否保持长链接

    std::unordered_map<std::string,int> Post_();//返回表单中的数据

private:
    bool ParseRequestLine_(const std::string& line);//解析请求行
    void ParseHeader_(const std::string& line);//解析请求头
    void ParseBody_(const std::string& line);//解析请求体

    void ParsePath_();//解析路径
    void ParsePost_();//解析post请求表单中的数据
    void ParseFromUrlencoded_();//使用url编码形式解析post请求表单数据

    PARSE_STATE state_;

    std::string method_,path_,version_,body_;//保存结果
    std::unordered_map<std::string,std::string> header_;
    std::unordered_map<std::string,int> post_;


    static const std::unordered_set<std::string> DEFAULT_HTML;

};

#endif // HTTP_REQUEST_H