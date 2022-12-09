#pragma once
#include "../dispatcher/Dispatcher.h"
#include "../kits/Channel.h"
#include <thread>
#include <queue>
#include <map>
#include <mutex>
using namespace std;

class Dispatcher;

// 处理channel的方式
enum class ElemType:char{ADD, DELETE, MODIFY};
// 定义任务队列的节点
struct ChannelElement
{
    ElemType type;   // 如何处理该节点中的channel
    Channel* channel;
    ChannelElement(ElemType type,Channel* channel) : type(type),channel(channel)   {}
};

class Dispatcher;

class EventLoop
{
public:
    EventLoop();
    EventLoop(const string threadName);
    ~EventLoop();
    
    int run();// 启动反应堆模型
    int eventActive(int fd, int event); // 处理被激活的文件fd
    int addTask(struct Channel* channel, ElemType type);// 添加任务到任务队列
    int processTaskQ();// 处理任务队列中的任务
    
    int add(Channel* channel);// 处理dispatcher中的节点
    int remove(Channel* channel);
    int modify(Channel* channel);
    int freeChannel(Channel* channel);// 释放channel
    int readMessage();
    inline thread::id getThreadID(){  return m_threadID;  } // 返回线程ID
    inline string getThreadName(){  return m_threadName;  }

    static int readLocalMessage(void* arg);
private:
    void taskWakeup();

private:
    queue<ChannelElement*>  m_taskQ;// 任务队列
    map<int, Channel*>      m_channelMap;   //通过fd 匹配Channel
    bool                    m_isQuit;
    Dispatcher*             m_dispatcher;// 事件分发 ！
    thread::id              m_threadID;// 线程id, name, mutex
    string                  m_threadName;
    mutex                   m_mutex;
    int                     m_socketPair[2];  // 存储本地通信的fd 通过socketpair 初始化
};

