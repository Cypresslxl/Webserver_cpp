#pragma once
#include "../kits/Buffer.h"
#include <stdbool.h>
#include <map>
#include <iostream>
#include <memory>
#include "HttpResponse.h"
// #include <tr1/memory>
using namespace std;

// 当前的解析状态
enum class PrecessState:char
{
    ParseReqLine,
    ParseReqHeaders,
    ParseReqBody,
    ParseReqDone
};
// 定义http请求结构体
class HttpRequest
{
public:
    HttpRequest();
    ~HttpRequest();

    void reset();


    void addHeader(const string key, const string value);
    string getHeader(const string key);

    // 解析请求行，头，协议
    bool parseRequestLine(Buffer* readBuf);
    bool parseRequestHeader(Buffer* readBuf);
    // bool parseHttpRequest(Buffer* readBuf, HttpResponse* response, Buffer* sendBuf, int socket);
    bool parseHttpRequest(Buffer* ,shared_ptr<HttpResponse>&  response,Buffer* sendBuf,int socket);
    bool processHttpRequest(shared_ptr<HttpResponse>& response);
    
    // 解码字符串
    string decodeMsg(string from);
    const string getFileType(const string name);


    static void sendDir(string dirName, Buffer* sendBuf, int cfd);
    static void sendFile(string dirName, Buffer* sendBuf, int cfd);


    inline void setMethod(string method)        { m_method = method;    }
    inline void seturl(string url)              { m_url = url;          }
    inline void setVersion(string version)      { m_version = version;  }
    inline void setState(PrecessState state)    { m_curState = state;   }
    
    // 获取当前处理状态
    inline PrecessState getState() {return m_curState; }

private:
    int hexToDec(char c);
    char* splitRequestLine(const char* start,
                            const char* end,
                            const char* sub,
                            function<void(string)> callback);

private:
    //请求行 
    string m_method;
    string m_url;
    string m_version;
    //请求头
    std::map<string, string>    m_reqHeaders;
    PrecessState                m_curState;
};

