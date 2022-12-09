#pragma once
#include "../kits/Channel.h"
#include "../src/EventLoop.h"
#include "Dispatcher.h"
#include <string>
#include <sys/epoll.h>
using namespace std;

//epoll 就三个函数
// epoll_create, epoll_create1 - open an epoll file descriptor
// epoll_wait();        监听epoll树，返回监听到的有事件的fd 数量
// epoll_create();      创建epoll树
// epoll_ctl();         

class EpollDispatcher : public Dispatcher
{
public:
    EpollDispatcher(EventLoop* evloop);
    ~EpollDispatcher();

public:
    int add() override;
    int remove() override;
    int modify() override;
    int dispatch(int timeout = 2) override; // 单位: s

private:
    int epollCtl(int op);

private:
    int                     m_epfd;
    struct epoll_event*     m_events;
    const int               m_maxNode = 520;
};