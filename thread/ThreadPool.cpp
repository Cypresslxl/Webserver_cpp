#include "ThreadPool.h"
#include <assert.h>
#include <stdlib.h>


ThreadPool::ThreadPool(EventLoop* mainLoop, int count = std::thread::hardware_concurrency())
{
    m_index     = 0;
    m_isStart   = false;
    m_mainLoop  = mainLoop;
    m_threadNum = count;
}

ThreadPool::~ThreadPool()
{
    for (auto item : m_workerThreads)
    {
        delete item;
    }
}

void ThreadPool::run()
{
    assert(!m_isStart);
    m_isStart = true;

    for (int i = 0; i < m_threadNum; ++i)
    {
        WorkerThread* tmp = new WorkerThread(i);
        tmp->run();
        m_workerThreads.push_back(tmp);
    }
}

EventLoop* ThreadPool::takeWorkerEventLoop()
{
    assert(m_isStart);

    // 从线程池中找一个子线程, 然后取出里边的反应堆实例
    EventLoop* evLoop = m_mainLoop;
    if (m_threadNum > 0)
    {
        evLoop = m_workerThreads[m_index]->getEventLoop();
        m_index = ++m_index % m_threadNum;
    }
    return evLoop;
}
