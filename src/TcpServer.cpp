#include "TcpServer.h"
#include <arpa/inet.h>
#include "TcpConnection.h"
#include <stdio.h>
#include <stdlib.h>
// #include "../utility/Log.h"
#include "../utility/Logger.h"
#include <iostream>
#include <json/json.h>
#include <fstream>
#include "../utility/System.h"
#include "../utility/jjson.h"


using namespace::Cypresslxl::utility;
using namespace::std;


TcpServer::TcpServer(unsigned short port, int threadNum):
                            m_port(port),
                            m_mainLoop(new EventLoop()),
                            m_threadNum(threadNum),
                            m_threadPool(new ThreadPool(m_mainLoop,threadNum))
{
    //load logfile
    string logfile = jjson::instance()->get<std::string>("logfile");
    string root = System::getinstance()->get_root_path() + logfile;
    Logger::instance()->open(root.c_str());

    
    setListen();
}

void TcpServer::setListen()
{
    // 1. 创建监听的fd
    m_lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_lfd == -1)
    {
        perror("socket");
        exit(0);
    }
    // 2. 设置端口复用
    int opt = 1;
    int ret = setsockopt(m_lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
    if (ret == -1)
    {
        perror("setsockopt");
        exit(0);
    }
    // 3. 绑定
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(m_lfd, (struct sockaddr*)&addr, sizeof addr);
    if (ret == -1)
    {
        perror("bind");
        exit(0);
    }
    // 4. 设置监听
    ret = listen(m_lfd, 128);
    if (ret == -1)
    {
        perror("listen");
        exit(0);
    }
}

void TcpServer::run()
{
    std::cout<<"server is running ...\n";
    m_threadPool->run(); 

    Channel* channel = new Channel(m_lfd, 
                                    FDEvent::ReadEvent, 
                                    acceptConnection, 
                                    nullptr, 
                                    nullptr, 
                                    this);             
    m_mainLoop->addTask(channel, ElemType::ADD);  

    m_mainLoop->run();     
}

//static func
int TcpServer::acceptConnection(void* arg)
{
    TcpServer* server = static_cast<TcpServer*>(arg);
    int cfd = accept(server->m_lfd, NULL, NULL);

    EventLoop* evLoop = server->m_threadPool->takeWorkerEventLoop();   
    new TcpConnection(cfd, evLoop); 
    // 不用担心内存泄漏,通过channel 回调销毁 heap 内存
    return 0;
}