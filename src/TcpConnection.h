#pragma once
#include "../src/EventLoop.h"
#include "../kits/Buffer.h"
#include "../kits/Channel.h"
#include "../task/HttpRequest.h"
#include "../task/HttpResponse.h"


//#define MSG_SEND_AUTO

class TcpConnection
{
public:
    TcpConnection(int fd, EventLoop* evloop);
    ~TcpConnection();

    static int processRead(void* arg);
    static int processWrite(void* arg);
    static int destroy(void* arg);
private:
    string          m_name;
    EventLoop*      m_evLoop;
    Channel*        m_channel;
    Buffer*         m_readBuf;
    Buffer*         m_writeBuf;    


    shared_ptr<HttpRequest>     m_request;
    shared_ptr<HttpResponse>    m_response;
    // HttpRequest*    m_request;
    // HttpResponse*   m_response;
};


