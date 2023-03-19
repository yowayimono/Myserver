/*
 * @Author: yowayimono 3485000346@qq.com
 * @Date: 2023-03-18 09:29:35
 * @LastEditors: yowayimono 3485000346@qq.com
 * @LastEditTime: 2023-03-18 21:49:37
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
    if(stat((srcDir_+path_).data(),&mmFileStat_)<0||S_ISDIR(mmFileStat_.st_mode)){
        code_=404;
    }
    else if(!(mmFileStat_.st_mode&S_IROTH)){
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

void HttpResponse::AddStateLine_(Buffer& buff)
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



