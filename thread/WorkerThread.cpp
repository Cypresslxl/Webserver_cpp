#include "WorkerThread.h"
#include <stdio.h>
#include <thread>


WorkerThread::WorkerThread(int index)
{
    m_evLoop    = nullptr;
    m_thread    = nullptr;
    m_threadID  = thread::id();
    m_name      =  "SubThread-" + to_string(index);
}

WorkerThread::~WorkerThread()
{
    if (m_thread != nullptr)
        delete m_thread;
}

// 子线程的回调函数
void WorkerThread::running()
{
    m_mutex.lock();
    m_evLoop = new EventLoop(m_name);       
    m_mutex.unlock();
    m_cond.notify_one();
    m_evLoop->run();            
}

void WorkerThread::run()
{
    m_thread = new thread(&WorkerThread::running, this);

    unique_lock<mutex> locker(m_mutex);
    while (m_evLoop == nullptr)
    {
        m_cond.wait(locker);
        //  在自线程创建事件循环（EventLoop）实例后 条件变量（m_cond)接收到信号后解除阻塞，
        //  子线程阻塞在其EventLoop中的I/O模型中（I/O检测事件）
    }
}
