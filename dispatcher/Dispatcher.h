#pragma once
#include "../kits/Channel.h"
#include "../src/EventLoop.h"
#include <string>
using namespace std;
class EventLoop;


class Dispatcher
{
public:
    Dispatcher(EventLoop* evloop);
    virtual ~Dispatcher();

public:
    virtual int add();
    virtual int remove();
    virtual int modify();
    virtual int dispatch(int timeout = 2); // 单位: s
    inline  void setChannel(Channel* channel){ m_channel = channel; }

protected:
    string      m_name = string();
    Channel*    m_channel;
    EventLoop*  m_evLoop;
};



