/*
 * @Author: yowayimono 3485000346@qq.com
 * @Date: 2023-03-18 09:29:35
 * @LastEditors: yowayimono 3485000346@qq.com
 * @LastEditTime: 2023-03-19 10:38:30
 * @FilePath: /Myserver/http/httpresponse.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "httpresponse.h"

using namespace std;


const unordered_map<string, string> HttpResponse::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".mp3",   "audio/mp3" },
    { ".mp4",   "audio/mp4" },
    { ".avi",   "video/x-msvideo" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "}
};

const unordered_map<int,std::string> HttpResponse::CODE_STATUS={
    {200,"OK"},
    {400,"Bad Request"},
    {403,"Forbidden"},
    {404,"Not Found"},
};


const unordered_map<int,std::string> HttpResponse::CODE_PATH={
    {400,"/error.html"},
    {403,"/error.html"},
    {404,"error.html"}
};


HttpResponse::HttpResponse(){
    code_=-1;
    path_=srcDir_="";
    isKeepAlive_=false;
    mmFile_=nullptr;
    mmFileStat_={0};
}

HttpResponse::~HttpResponse(){
    UnmapFile();//直接调用解除内存映射函数
}

void HttpResponse::Init(const std::string& srcDir,std::string& path,std::unordered_map<std::string,int> post_,bool iskeepAlive,int code)
{
    assert(srcDir!="");
    if(mmFile_){UnmapFile();}//如果已有映射则解除

    code_=code;
    isKeepAlive_=iskeepAlive;
    path_=path;
    srcDir_=srcDir;

    mmFile_=nullptr;
    mmFileStat_={0};
    post__=post_;
}

void HttpResponse::MakeResponse(Buffer& buff){
    /*判断请求的文件，拼接路径*/
    if(stat((srcDir_+path_).data(),&mmFileStat_)<0||S_ISDIR(mmFileStat_.st_mode)){//先判断文件是否存在
        code_=404;
    }
    else if(!(mmFileStat_.st_mode&S_IROTH)){//判断是否有权限
        code_=403;
    }
    else if(code_=-1){
        code_=200;
    }
    ErrorHtml_();
    AddStateLine_(buff);
    AddHeader_(buff);
    AddContent_(buff);
    cout<<"封装响应完成"<<endl;
}

void HttpResponse::UnmapFile(){
    if(mmFile_)
    {
        munmap(mmFile_,mmFileStat_.st_size);
        mmFile_=nullptr;
    }
}

char* HttpResponse::File(){
    return mmFile_;
}

size_t HttpResponse::FileLen() const{
    return mmFileStat_.st_size;
}
void HttpResponse::ErrorContent(Buffer& buff,string message)
{//构建错误响应消息正文
    string body;
    string status;
    body+="<html><titlt>Error</title>";
    body+="<body bgcolor=\"ffffff\">";
    if(CODE_STATUS.count(code_)==1){
        status=CODE_STATUS.find(code_)->second;
    }else{
        status="Bad Request";
    }
    body+=to_string(code_)+":"+status+"\n";
    body+="<p>"+message+"<p>";
    body+="<hr><em>TinyWebServer</em></body></html>";

    buff.Append("Content-length:"+to_string(body.size())+"\r\n\r\n");
    buff.Append(body);

}

void HttpResponse::AddStateLine_(Buffer& buff)//封装响应首行
{
    string status;
    if(CODE_STATUS.count(code_)==1)//存在
    {
        status=CODE_STATUS.find(code_)->second;
    }
    else{
        code_=400;
        status=CODE_STATUS.find(400)->second;
    }
    buff.Append("HTTP/1.1"+to_string(code_)+" "+status+"\r\n");
}


void HttpResponse::AddHeader_(Buffer& buff){
    buff.Append("Connection: ");
    if(isKeepAlive_)
    {
        buff.Append("keep-alive\r\n");
        buff.Append("keep-alive:max=6,timeout=120\r\n");
    }else{
        buff.Append("close\r\n");
    }
    buff.Append("Connect-type: "+GetFileType_()+"\r\n");
    buff.Append("charset: utf-8\r\n");
}

/*


该段代码是HTTP响应报文中添加响应内容的函数，对应于HttpResponse类的AddContent_函数。该函数通过打开本地文件、读取文件内容并将其存储到HTTP响应报文中的方式，将响应正文添加到HTTP响应报文中。

具体来说，函数首先打开请求的文件，如果文件不存在则返回错误信息，将错误信息存储到HTTP响应报文中。如果请求的是CGI脚本，执行AddPostContent_函数，该函数将POST请求的响应内容写入HTTP响应报文中。

接着，使用mmap函数将请求的文件映射到内存中，从而提高文件访问速度。如果映射失败，则返回错误信息并将其存储到HTTP响应报文中。

最后，关闭文件，将响应的数据大小添加到HTTP响应报文中。

下面是对代码中各句的解释和建议：

1. 打开请求的文件

使用open函数打开请求的文件，返回文件描述符srcFd。需要注意的是，open函数的路径参数必须是字符指针类型，因此需要使用data函数转换成char*指针类型。

建议：需要检查open函数的返回值，若小于0则说明打开文件失败，需要返回错误信息给客户端。

2. 判断是否为CGI脚本

如果请求的文件为CGI脚本，调用AddPostContent_函数添加POST请求的响应内容到HTTP响应报文中，并返回。

建议：需要对请求的文件类型进行更准确的判断，避免误将普通文件和CGI脚本文件混淆。

3. 将文件映射到内存中

使用mmap函数将请求的文件映射到内存中，这样就可以使用指针直接读取文件内容，从而提高文件访问速度。需要注意的是，mmap函数映射完成后需要手动关闭文件描述符。

建议：需要检查mmap函数的返回值，若等于-1则说明映射失败，需要返回错误信息给客户端。

4. 获得文件大小并添加到HTTP响应报文

使用stat函数获得请求的文件的大小，然后将其添加到HTTP响应报文的报文头中，表示响应的数据大小。

建议：需要检查stat函数的返回值，若小于0则说明获取文件信息失败，需要返回错误信息给客户端。同时建议添加缓存功能，以避免每次请求都需要重新计算文件大小。

综上所述，建议对代码进行改进以提高代码的可读性和稳定性。*/
void HttpResponse::AddContent_(Buffer& buff)
{
    int srcFd=open((srcDir_+path_).data(),O_RDONLY);
    if(srcFd<0)
    {
        
        ErrorContent(buff,"File NotFound");
        return;
    }
    if(path_=="/CGI/compute_.html"){
        AddPostContent_(buff);
        return;
    }
    cout<<"file path "<<(srcDir_+path_).data()<<endl;
    int* mmRet=(int*)mmap(0,mmFileStat_.st_size,PROT_READ,MAP_PRIVATE,srcFd,0);
    if(*mmRet==-1)
    {
        ErrorContent(buff,"File Notfound!");
        return;
    }
    mmFile_=(char*)mmRet;

    close(srcFd);
    buff.Append("Content-length: "+to_string(mmFileStat_.st_size)+"\r\n\r\n");
}


void HttpResponse::ErrorHtml_(){
    if(CODE_PATH.count(code_)==1){
        path_=CODE_PATH.find(code_)->second;
        stat((srcDir_+path_).data(),&mmFileStat_);
    }

}

string HttpResponse::GetFileType_(){
    string::size_type idx=path_.find_last_of('.');
    if(idx==string::npos){
        return "text/plain";
    }
    string suffix=path_.substr(idx);
    if(SUFFIX_TYPE.count(suffix)==1){
        return SUFFIX_TYPE.find(suffix)->second;
    }


    return "text/plain";
}

void HttpResponse::AddPostContent_(Buffer& buff)
{
    int a,b;
    a=post__["a"];/*POST请求处理数据，处理后封装进响应报文*/
    b=post__["b"];


    int sum=a+b;
    string body;
    body+="<html><head><title>niliushall's CGI</titlt></head>";
    body+="<body><p>The result is "+to_string(a)+"+"+to_string(b)+"="+to_string(sum);
    body+="</p></body></html>";

    buff.Append("Content-length: "+to_string(body.size())+"\r\n\r\n");
    buff.Append(body);

}

