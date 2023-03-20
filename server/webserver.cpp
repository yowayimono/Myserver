#include "webserver.h"

#include "spdlog/spdlog.h"

using namespace std;

WebServer::WebServer(
    int port,int trigMode,
    int connPoolNum,int threadNum
):port_(port),isClose_(false),
threadpool_(new ThreadPool(threadNum)),epoller_(new Epoller())
{
    srcDir_=getcwd(nullptr,256);
    assert(srcDir_);  
    strncat(srcDir_,"/resources/",16);
    HttpConn::userCount=0;
    HttpConn::srcDir=srcDir_;

    InitEventMode_(trigMode);
    if(!InitSocket_()){isClose_=true;}
}

WebServer::~WebServer(){
    close(listenFd_);
    isClose_=true;
    free(srcDir_);
}


void WebServer::_Start(){
    int timeMs=-1;
    if(!isClose_){cout<<"。。。。。。。。。覃文敏一号启动。。。。。。。。。。。"<<endl;}
    while(!isClose_){
        int eventCnt=epoller_->Wait(timeMs);
        for(int i=0;i<eventCnt;i++){
            int fd=epoller_->GetEventFd(i);
            uint32_t events=epoller_->GetEvents(i);

            cout<<"判断事件的文件描述符"<<endl;

            if(fd==listenFd_){
                DealListen_();
            }
            else if(events & (EPOLLRDHUP |EPOLLERR)){
                assert(users_.count(fd)>0);
                CloseConn_(&users_[fd]);
            }
            else if(events & EPOLLIN){
                assert(users_.count(fd)>0);
                DealRead_(&users_[fd]);
            }
            else if(events & EPOLLOUT){
                assert(users_.count(fd)>0);
                DealWrite_(&users_[fd]);
            }
            else{
                cout<<"Unexpexted event"<<endl;
            }
        }
    }
}


bool WebServer::InitSocket_(){
    struct sockaddr_in addr;
    if(port_>65535||port_<1024)
    {
        cout<<"Post:"<<port_<<"Error"<<endl;
        return false;
    }

    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(port_);

    listenFd_=socket(AF_INET,SOCK_STREAM,0);
    if(listenFd_<0){
        cout<<"Create socket error!"<<endl;
        return false;
    }
    int ret;
    int optval=1;
    ret=setsockopt(listenFd_,SOL_SOCKET,SO_REUSEADDR,(const void*)&optval,sizeof(int));
    if(ret<0){
        cout<<"Set socket setsockopt error"<<endl;
        return false;
    }

    ret=bind(listenFd_,(struct sockaddr*)&addr,sizeof(addr));
    if(ret<0){
        cout<<"Bind sock error!"<<endl;
        close(listenFd_);
        return false;
    }

    ret=listen(listenFd_,6);
    if(ret<0){
        cout<<"listen error! Port: "<<port_<<endl;
        close(listenFd_);
        return false; 
    }

    ret-epoller_->AddFd(listenFd_,listenEvent_|EPOLLIN);

    if(ret==0){
        cout<<"Add listen error!"<<endl;
        close(listenFd_);
        return false;
    }

    SetFdNonblock(listenFd_);
    cout<<"Server Port:"<<port_<<endl;//设置非阻塞
    //cout<<"Server Port"<<port_<<endl;
    return true;


}

void WebServer::InitEventMode_(int trigMod){
    listenEvent_=EPOLLRDHUP;
    connEvent_=EPOLLONESHOT | EPOLLRDHUP;

    switch(trigMod){
        case 0:
            break;
        case 1:
            connEvent_=EPOLLET;
            break;
        case 2:
            listenEvent_|=EPOLLET;
            break;
        case 3:
            listenEvent_|=EPOLLET;
            connEvent_|=EPOLLET;
            break;
        default:
            listenEvent_|=EPOLLET;
            connEvent_|=EPOLLET;
            break;

    }
    HttpConn::isET=(connEvent_ & EPOLLET);
}

void WebServer::AddClient_(int fd,sockaddr_in addr)
{
    assert(fd>0);
    users_[fd].Init(fd,addr);
    epoller_->AddFd(fd,EPOLLIN | connEvent_);
    SetFdNonblock(fd);
    cout<<"AddClient_"<<users_[fd].GetFd()<<" in!"<<endl;
}


void WebServer::DealListen_(){
    cout<<"小覃开始处理监听"<<endl;
    struct sockaddr_in addr;
    socklen_t len=sizeof(addr);
    do{
        int fd=accept(listenFd_,(struct sockaddr*)&addr,&len);
        if(fd<0){return;}
        else if(HttpConn::userCount>-MAX_FD){
            SendError_(fd,"Server busy");
            cout<<"Clients is full"<<endl;
            return;
        }
        AddClient_(fd,addr);
    }while(listenFd_&EPOLLET);
}


void WebServer::DealRead_(HttpConn* client)
{
    cout<<"小覃开始处理读事件"<<endl;
    assert(client);
    threadpool_->AddTask(std::bind(&WebServer::onRead_,this,client));

}


void WebServer::SendError_(int fd,const char* info)
{
    assert(fd>0);
    int ret=send(fd,info,strlen(info),0);
    if(ret<0)
    {
        cout<<"send error to client"<<fd<<"error!"<<endl;
    }
    close(fd);
}


void WebServer::DealWrite_(HttpConn *client)
{
    cout<<"小覃开始处理写事件"<<endl;
    assert(client);
    threadpool_->AddTask(std::bind(&WebServer::onWrite_,this,client));
}

void WebServer::CloseConn_(HttpConn* client)
{
    assert(client);
    cout<<"Client:"<<client->GetFd()<<" 走啦您嘞"<<endl;
    epoller_->DelFd(client->GetFd());
    client->Close();
}


void WebServer::onRead_(HttpConn* client)
{
    assert(client);
    int ret=-1;
    int readErrno=0;
    ret=client->read(&readErrno);
    if(ret<=0&&readErrno!=EAGAIN){
        CloseConn_(client);
        return;
    }
    onProcess(client);
}

void WebServer::onWrite_(HttpConn* client)
{
    assert(client);
    int ret=-1;
    int writeErrno=0;
    ret-client->write(&writeErrno);
    if(client->ToWriteBytes()==0)
    {
        if(client->IsKeepAlive()==0)
        {
            onProcess(client);
            return;
        }
    }
    else if(ret<0){
        if(writeErrno==EAGAIN){
            epoller_->ModFd(client->GetFd(),connEvent_|EPOLLET);
            return;
        }
    }
    CloseConn_(client);
}

void WebServer::onProcess(HttpConn* client)
{
    if(client->process()){
        epoller_->ModFd(client->GetFd(),connEvent_ | EPOLLOUT);
    }else{
        epoller_->ModFd(client->GetFd(),connEvent_ |EPOLLIN);
    }
}

int WebServer::SetFdNonblock(int fd){
    assert(fd>0);

    int flag=fcntl(fd,F_GETFL,0);
    flag |=O_NONBLOCK;
    return fcntl(fd,F_SETFL,flag);
}

