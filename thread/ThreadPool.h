#pragma once
#include "../src/EventLoop.h"
#include <stdbool.h>
#include "WorkerThread.h"
#include <vector>
using namespace std;


class ThreadPool
{
public:
    ThreadPool(EventLoop* mainLoop,int count);
    ~ThreadPool();
    
public:
    void run();     
    EventLoop* takeWorkerEventLoop();

private:
    bool                    m_isStart;//标记线程池的启动与否
    int                     m_threadNum;
    int                     m_index;
    EventLoop*              m_mainLoop;// 主线程的反应堆模型
    vector<WorkerThread*>   m_workerThreads;
};




