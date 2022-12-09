#pragma once
#include <functional>


// 定义函数指针
// typedef int(*handleFunc)(void* arg);
// using handleFunc = int(*)(void*);

// 定义文件描述符的读写事件
enum class FDEvent
{
    TimeOut     = 0x01,
    ReadEvent   = 0x02,
    WriteEvent  = 0x04
};

// 可调用对象包装器打包的是什么? 1. 函数指针 2. 可调用对象(可以向函数一样使用)
// 最终得到了地址, 但是没有调用
class Channel
{
public:
    using handleFunc = std::function<int(void*)>;
    Channel(int fd,
            FDEvent events, 
            handleFunc readFunc, 
            handleFunc writeFunc, 
            handleFunc destroyFunc, 
            void* arg);
    
    handleFunc readCallback;                
    handleFunc writeCallback;
    handleFunc destroyCallback;
    
    void writeEventEnable(bool flag);       // 修改fd的写事件(检测 or 不检测)
    bool isWriteEventEnable();              // 判断是否需要检测文件描述符的写事件
    inline int getEvent()                   {  return m_events; }
    inline int getSocket()                  {  return m_fd;     }
    inline const void* getArg()             {return m_arg;      }
private:
    int     m_fd;   // 文件描述符
    int     m_events;// 事件
    void*   m_arg;// 回调函数的参数
};

