#include "../src/TcpConnection.h"
#include "../task/HttpRequest.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "../utility/jjson.h"
// #include "../utility/Log.h"
#include "../utility/Logger.h"

using namespace::Cypresslxl::utility;

int TcpConnection::processRead(void* arg)
{
    TcpConnection* conn = static_cast<TcpConnection*>(arg);
    // 接收数据
    int socket  = conn->m_channel->getSocket();
    int count   = conn->m_readBuf->socketRead(socket);

    // Debug()
    // printf("接收到的http请求数据: \n%s", conn->m_readBuf->data());
    // debug("接收到的http请求数据: \n%s\n", conn->m_readBuf->data());//此log 写入文件
    if (count > 0)
    {
#ifdef MSG_SEND_AUTO
        conn->m_channel->writeEventEnable(true);
        conn->m_evLoop->addTask(conn->m_channel, ElemType::MODIFY);
#endif
        // !!! http 请求报文处理 
        bool flag = conn->m_request->parseHttpRequest(
            conn->m_readBuf, conn->m_response,
            conn->m_writeBuf, socket);
        // printf("%s %s %s",m_request->m_method.c_str(),m_request->m_url.c_str(),m_request->m_version.c_str());
        if (!flag)
        {
            string errMsg = "Http/1.1 400 Bad Request\r\n\r\n";
            conn->m_writeBuf->appendString(errMsg);
        }
    }
    else
    {
#ifdef MSG_SEND_AUTO
        // 断开连接
        conn->m_evLoop->addTask(conn->m_channel, ElemType::DELETE);
#endif
    }
#ifndef MSG_SEND_AUTO
    // 断开连接
    conn->m_evLoop->addTask(conn->m_channel, ElemType::DELETE);
#endif
    return 0;
}

int TcpConnection::processWrite(void* arg)
{
    printf("开始发送数据了(基于写事件发送)....");
    TcpConnection* conn = static_cast<TcpConnection*>(arg);
    // 发送数据
    //  
    int count = conn->m_writeBuf->sendData(conn->m_channel->getSocket());
    if (count > 0)
    {
        if (conn->m_writeBuf->readableSize() == 0)
        {
            // 1. 不再检测写事件 -- 修改channel中保存的事件
            conn->m_channel->writeEventEnable(false);
            conn->m_evLoop->addTask(conn->m_channel, ElemType::MODIFY);
            conn->m_evLoop->addTask(conn->m_channel, ElemType::DELETE);
        }
    }
    return 0;
}

int TcpConnection::destroy(void* arg)
{
    TcpConnection* conn = static_cast<TcpConnection*>(arg);
    if (conn != nullptr)
    {
        cout<<"\ndelete TcpConnection\n";
        delete conn;
    }
    return 0;
}

TcpConnection::TcpConnection(int fd, EventLoop* evloop)
{
    m_evLoop    = evloop;
    int size = jjson::instance()->get<int>("Buffer");
    m_readBuf   = new Buffer(size);
    m_writeBuf  = new Buffer(size);


    m_request   = make_shared<HttpRequest>();
    m_response  = make_shared<HttpResponse>();
    // http
    // m_request   = new HttpRequest;
    // m_response  = new HttpResponse;
    m_name = "TcpConnection-" + to_string(fd);
    m_channel = new Channel(fd, FDEvent::ReadEvent, processRead, processWrite, destroy, this);
    evloop->addTask(m_channel, ElemType::ADD); 
    std::cout<<m_name<<" : created!\n"; 
    //这里会出现 eventloop的threadID !=线程id，一个是主线程的id，一个是子线程的eventloop   ！！！
    //主线程调用子线程的eventloop中的addTask,将cfd添加到子线程的Eventloop中 ，
    // 调用takewakeup，激活子线程eventloop中的sockpair[0],解除阻塞dispatch，子线程再调用processTaskQ()将cfd添加到
    //子线程的diapatcher中，dispatch阻塞，监听cfd和sockpair[0]的事件，监听到cfd就解除阻塞，调用回调函数。
}

TcpConnection::~TcpConnection()
{
    if (m_readBuf && m_readBuf->readableSize() == 0 &&
        m_writeBuf && m_writeBuf->readableSize() == 0)
    {
        delete m_readBuf;
        delete m_writeBuf;

        // delete m_request;
        // delete m_response;
        m_evLoop->freeChannel(m_channel);
    }


    printf("TcpConnection:%s over", m_name.c_str());
    debug("\nconnect over: \n%s\n", m_name.c_str());
}
