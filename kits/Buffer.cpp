//#define _GNU_SOURCE
#include "Buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>


Buffer::Buffer(int size):m_capacity(size)
{
    m_data = (char*)malloc(size);
    bzero(m_data, size);//memset(m_data,0,sizeof m_data);
}

Buffer::~Buffer()
{
    if (m_data != nullptr)
    {
        free(m_data);
    }
}

void Buffer::extendRoom(int size)
{
    // 1.不需要扩容
    if (writeableSize() >= size)    return;
/*
    2. 内存需要合并后够用 - 不需要扩容
    剩余的可写的内存 + 已读的内存 > size
*/
    else if (m_readPos + writeableSize() >= size)
    {
        int readable = readableSize();
        memcpy(m_data, m_data + m_readPos, readable);
        m_readPos = 0;
        m_writePos = readable;
    }
    // 3. 扩容
    else
    {
        //realloc 和malloc 的区别：1、malloc函数：malloc函数对没有分配过的内存块直接进行分配。
        // 2、realloc函数：realloc函数是在已经分配好的内存块重新进行分配。
        void* temp = realloc(m_data, m_capacity + size);
        if (temp ==NULL)
        {
            return; 
        }
        //新申请的内存进行初始化
        memset((char*)temp + m_capacity, 0, size);
        m_data = static_cast<char*>(temp);
        m_capacity += size;
    }
}

int Buffer::appendString(const char* data, int size)
{
    if (data == nullptr || size <= 0)
    {
        return -1;
    }
    // 检查是否扩容，需要则会扩容 check and extend
    extendRoom(size);
    // 数据拷贝
    memcpy(m_data + m_writePos, data, size);
    m_writePos += size;
    return 0;
}

int Buffer::appendString(const char* data)
{
    int size = strlen(data);
    int ret = appendString(data, size);
    return ret;
}

int Buffer::appendString(const string data)
{
    int ret = appendString(data.data());
    return ret;
}

int Buffer::socketRead(int fd)
{
    /*
为什么引出readv()和writev()
因为使用read()将数据读到不连续的内存、使用write()将不连续的内存发送出去，要经过多次的调用read、write
如果要从文件中读一片连续的数据至进程的不同区域，有两种方案：①使用read()一次将它们读至一个较大的缓冲区中，然后将它们分成若干部分复制到不同的区域； ②调用read()若干次分批将它们读至不同区域。
同样，如果想将程序中不同区域的数据块连续地写至文件，也必须进行类似的处理。
怎么解决多次系统调用+拷贝带来的开销呢？
UNIX提供了另外两个函数—readv()和writev()，它们只需一次系统调用就可以实现在文件和进程的多个缓冲区之间传送数据，免除了多次系统调用或复制数据的开销。
readv/writev
在一次函数调用中：
① writev以顺序iov[0]、iov[1]至iov[iovcnt-1]从各缓冲区中聚集输出数据到fd
② readv则将从fd读入的数据按同样的顺序散布到各缓冲区中，readv总是先填满一个缓冲区，然后再填下一个
    */
    // read/recv/readv
    struct iovec vec[2];
    // 初始化数组元素
    int writeable = writeableSize();
    vec[0].iov_base = m_data + m_writePos;
    vec[0].iov_len = writeable;
    char* tmpbuf = (char*)malloc(40960);
    vec[1].iov_base = tmpbuf;
    vec[1].iov_len = 40960;
    int result = readv(fd, vec, 2);
    if(result==-1){
        perror("readv");
        exit(0);
    }
    else if (result <= writeable)
    {
        m_writePos += result;
    }
    else
    {
        m_writePos = m_capacity;
        appendString(tmpbuf, result - writeable);
    }
    free(tmpbuf);
    return result;
}

char* Buffer::findCRLF()
{
    char* ptr = (char*)memmem(m_data + m_readPos, readableSize(), "\r\n", 2);
    return ptr;
}

int Buffer::sendData(int socket)
{
    // 判断有无数据
    int readable = readableSize();
    if (readable > 0)
    {
        // MSG_NOSIGNALlinux下当连接断开，还发数据的时候，不仅send()的返回值会有反映，
        // 而且还会向系统发送一个异常消息，如果不作处理，系统会出BrokePipe，程序会退出，
        // 这对于服务器提供稳定的服务将造成巨大的灾难。为此，send()函数的最后一个参数
        // 可以设MSG_NOSIGNAL，禁止send()函数向系统发送异常消息

        int count = send(socket, m_data + m_readPos, readable, MSG_NOSIGNAL);
        if (count > 0)
        {
            m_readPos += count;
            usleep(1);
        }
        return count;
    }
    return 0;
}

