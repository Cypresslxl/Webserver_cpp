#pragma once
#include <string>
using namespace std;


class Buffer
{
public:
    Buffer(int size);
    ~Buffer();

    void extendRoom(int size);// 扩容
    inline int writeableSize(){return m_capacity - m_writePos;}// 得到剩余的可写的内存容量
    inline int readableSize(){return m_writePos - m_readPos;}// 得到剩余的可读的内存容量
    
    int appendString(const char* data, int size);// 写内存 1. 直接写 2. 接收套接字数据
    int appendString(const char* data);
    int appendString(const string data);

    int socketRead(int fd);
    
    char* findCRLF();
    
    int sendData(int socket);
    
    inline char* data(){return m_data + m_readPos;}// 得到读数据的起始位置
    
    inline int readPosIncrease(int count){m_readPos += count;  return m_readPos;}
private:
    char* m_data;
    int m_capacity;
    int m_readPos = 0;
    int m_writePos = 0;
};
