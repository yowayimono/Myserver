#include "httprequest.h"

//实现部分

using namespace std;
const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML{
    "index","/comepute","/picture","/video","/error",
};

void HttpRequest::Init(){
    method_=path_=version_=body_="";
    state_=REQUEST_LINE;//从请求行开始读取
    header_.clear();
    post_.clear();//将上次读取数据清空
}

bool HttpRequest::parse(Buffer& buff){
    const char CRLF[]="\r\n";//回车符换行符，是读取到完整行标志
    if(buff.ReadableBytes()<=0){//缓冲区没有可读数据
        return false;
    }
    while(buff.ReadableBytes()&&state_!=FINISH)//有可读信息并且没有解析完成
    {
        const char* lineEnd=search(buff.Peek(),buff.BeginWriteConst(),CRLF,CRLF+2);//识别到行结束标志
        std::string line(buff.Peek(),lineEnd);//获取完整一行
        switch(state_)//状态机解析
        {
            case REQUEST_LINE:
                if(!ParseRequestLine_(line)){
                    return false;
                }
                ParsePath_();
                break;
            case HEADERS:
                ParseHeader_(line);//解析请求头
                if(buff.ReadableBytes()<=2){//读取到结束标志，完成
                    state_=FINISH;
                }
                break;
            case BODY:
                ParseBody_(line);//解析请求体
                break;
            default:
                break;
        }
        if(lineEnd==buff.BeginWrite()){break;}//未读取到完整行
        buff.RetrieveUntil(lineEnd+2);//跳过标志开始读下一行
    }
    cout<<"解析结果  mothod_:"<<method_.c_str()<<"path_:"<<path_.c_str()<<"version_"<<version_.c_str()<<endl;
    return true;
}

std::string HttpRequest::method() const{
    return method_;
}


std::string HttpRequest::version() const{
    return version_;
}

std::string HttpRequest::path() const{
    return path_;
}

std::string& HttpRequest::path(){
    return path_;
}

bool HttpRequest::IsKeepAlive() const{//检测是否保持长链接
    if(header_.count("Connection")==1){
        return header_.find("Connection")->second=="keep-alive"&&version_=="1.1";
    }
    return false;//只有1.1版本支持长链接
}

bool HttpRequest::ParseRequestLine_(const string &line)
{
    regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");//匹配请求行
    smatch subMatch;
    if (regex_match(line, subMatch, patten))//匹配成功，保存到subMatch
    {
        method_ = subMatch[1];
        path_ = subMatch[2];
        version_ = subMatch[3];
        state_ = HEADERS;//状态转移
        return true;
    }
    cout << "RequestLine Error" << endl;//匹配失败
    return false;
}

void HttpRequest::ParseHeader_(const string &line)
{
    regex patten("^([^:]*): ?(.*)$");
    smatch subMatch;
    if (regex_match(line, subMatch, patten))
    {
        header_[subMatch[1]] = subMatch[2];
    }
    else
    {
        state_ = BODY; // 状态转移
    }
}

void HttpRequest::ParseBody_(const string& line)
{
    body_=line;
    ParsePost_();
    state_=FINISH;
    cout<<"Body:"<<line.c_str()<<" len:"<<line.size()<<endl;
}

void HttpRequest::ParsePath_(){
    if(path_=="/"){
        path_="/index.html";
    }
    else{
        for(auto &item:DEFAULT_HTML){
            if(item==path_){
                path_+=".html";
                break;
            }
        }
    }
}


void HttpRequest::ParsePost_(){
    if(method_=="POST"/*&&*/)
    {
        cout<<"解析POST请求"<<endl;
        ParseFromUrlencoded_();
        path_="/CGI/compute_.html";
    }
}

void HttpRequest::ParseFromUrlencoded_(){
    if(body_.size()==0){return;}


    string key;
    int value;

    int n=body_.size();
    int i=0,j=0;
    for(;i<n;i++)
    {
        char ch =body_[i];
        switch(ch){
            case '=':
                key=body_.substr(j,i-j);
                j=i+1;
                break;
            case '&':
                value=stoi(body_.substr(j,i-j));
                j=i+1;
                post_[key]=value;
                break;
            default:
                break;
        }
    }
    assert(j<=i);
    if(j<i){
        value=stoi(body_.substr(j,i-j));
        post_[key]=value;
    }
}


std::unordered_map<std::string,int> HttpRequest::Post_(){
    return post_;
}


