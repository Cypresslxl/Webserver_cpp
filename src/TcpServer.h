#pragma once
#include "../src/EventLoop.h"
#include "../thread/ThreadPool.h"
#include "../kits/Channel.h"


class TcpServer
{
public:
    TcpServer(unsigned short port, int threadNum);
    void run();

private:
    void setListen();

public:
    static int acceptConnection(void* arg);

private:
    EventLoop*      m_mainLoop;
    ThreadPool*     m_threadPool;
    int             m_threadNum;
    int             m_lfd;
    unsigned short  m_port;
};

