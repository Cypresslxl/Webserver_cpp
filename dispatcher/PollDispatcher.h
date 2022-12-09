#pragma once
#include "../kits/Channel.h"
#include "../src/EventLoop.h"
#include "../dispatcher/Dispatcher.h"
#include <string>
#include <poll.h>
using namespace std;

class PollDispatcher : public Dispatcher
{
public:
    PollDispatcher(EventLoop* evloop);
    ~PollDispatcher();

public:
    int add() override;
    int remove() override;
    int modify() override;
    int dispatch(int timeout = 2) override; // 单位: s

private:
    int             m_maxfd;    //poll()函数会 传出事件发生的个数
    struct pollfd*  m_fds;
    const int       m_maxNode = 1024;
};