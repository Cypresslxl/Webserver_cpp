#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include "../src/EventLoop.h"
using namespace std;


//封装thread
class WorkerThread
{
public:
    WorkerThread(int index /*记录线程序号*/ );
    ~WorkerThread();

public:
    void run();
    inline EventLoop* getEventLoop()   { return m_evLoop; }
    
private:
    void running();
    
private:
    thread*             m_thread;   // 保存线程的实例
    thread::id          m_threadID; // ID
    string              m_name;
    mutex               m_mutex;  // 互斥锁
    condition_variable  m_cond;    // 条件变量
    EventLoop*          m_evLoop;   //每个子线程对应一个事件循环Eventloop（反应堆模型）
};



