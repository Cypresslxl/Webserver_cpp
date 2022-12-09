#pragma once
#include "../kits/Buffer.h"
#include <map>
#include <functional>
using namespace std;

// 定义状态码枚举
enum class StatusCode
{
    Unknown,
    OK = 200,
    MovedPermanently = 301,
    MovedTemporarily = 302,
    BadRequest = 400,
    NotFound = 404
};

// 定义结构体
class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse();
    function<void(const string, struct Buffer*, int)> sendDataFunc;

    
    // 1.添加响应头2.组织http响应数据
    void addHeader(const string key, const string value);
    void prepareMsg(Buffer* sendBuf, int socket);
public:
    inline void setFileName(string name)        { m_fileName = name; }
    inline void setStatusCode(StatusCode code)  { m_statusCode = code; }
private:
    // 状态行: 状态码, 状态描述
    StatusCode      m_statusCode;
    string          m_fileName;
    // 响应头 - 键值对
    map<string, string> m_headers;
    // 定义状态码和描述的对应关系
    const map<int, string> m_info = {
        {200, "OK"},
        {301, "MovedPermanently"},
        {302, "MovedTemporarily"},
        {400, "BadRequest"},
        {404, "NotFound"},
    };
};

