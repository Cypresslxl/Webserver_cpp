//#define _GNU_SOURCE
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include "../src/TcpConnection.h"
#include <assert.h>
#include <ctype.h>
HttpRequest::HttpRequest()  { reset(); }

HttpRequest::~HttpRequest() {}

// auto methodFunc = bind(&HttpRequest::setMethod, this, placeholders::_1);
// start = splitRequestLine(start, end, " ", methodFunc);

void HttpRequest::reset()
{
    m_curState = PrecessState::ParseReqLine;
    m_method = m_url = m_version = string(); 
    m_reqHeaders.clear();
}

void HttpRequest::addHeader(const string key, const string value)
{
    if (key.empty() || value.empty())
    {
        return;
    }
    m_reqHeaders.insert(make_pair(key, value));
}

string HttpRequest::getHeader(const string key)
{
    auto item = m_reqHeaders.find(key);
    if (item == m_reqHeaders.end())
    {
        return string();
    }
    return item->second;
}

char* HttpRequest::splitRequestLine(const char* start, 
                                    const char* end,
                                    const char* sub, 
                                    function<void(string)> callback)
{
    char* space = const_cast<char*>(end);//屏蔽const 属性
    if (sub != nullptr)
    {
        space = static_cast<char*>(memmem(start, end - start, sub, strlen(sub)));
        assert(space != nullptr);
    }
    int length = space - start;
    callback(string(start, length));
    return space + 1;
}

bool HttpRequest::parseRequestLine(Buffer* readBuf)
{
    char* end = readBuf->findCRLF(); 
    char* start = readBuf->data();
    int lineSize = end - start;

    if (lineSize > 0)
    {
        auto methodFunc = bind(&HttpRequest::setMethod, this, placeholders::_1);
        start = splitRequestLine(start, end, " ", methodFunc);
        auto urlFunc = bind(&HttpRequest::seturl, this, placeholders::_1);
        start = splitRequestLine(start, end, " ", urlFunc);
        auto versionFunc = bind(&HttpRequest::setVersion, this, placeholders::_1);

        splitRequestLine(start, end, nullptr, versionFunc);
        readBuf->readPosIncrease(lineSize + 2);

        setState(PrecessState::ParseReqHeaders);
        cout<<m_method<<" "<<m_url<<" "<<m_version<<"\n";
        return true;
    }
    return false;
}

bool HttpRequest::parseRequestHeader(Buffer* readBuf)
{
    char* end = readBuf->findCRLF();
    if (end != nullptr)
    {
        char* start = readBuf->data();
        int lineSize = end - start;
        // 基于: 搜索字符串
        char* middle = static_cast<char*>(memmem(start, lineSize, ": ", 2));
        if (middle != nullptr)
        {
            int keyLen = middle - start;
            int valueLen = end - middle - 2;
            if (keyLen > 0 && valueLen > 0)
            {
                string key(start, keyLen);
                string value(middle + 2, valueLen);
                addHeader(key, value);
            }
            readBuf->readPosIncrease(lineSize + 2);
        }
        else
        {
            readBuf->readPosIncrease(2);
            // 修改解析状态
            // 忽略 post 请求, 按照 get 请求处理
            setState(PrecessState::ParseReqDone);
        }
        return true;
    }
    return false;
}

// bool HttpRequest::parseHttpRequest(Buffer* readBuf, HttpResponse* response, Buffer* sendBuf, int socket)
bool HttpRequest::parseHttpRequest(Buffer* readBuf,shared_ptr<HttpResponse>& response,Buffer* sendBuf,int socket)
{
    bool flag = true;
    while (m_curState != PrecessState::ParseReqDone)
    {
        switch (m_curState)
        {
        case PrecessState::ParseReqLine:
            flag = parseRequestLine(readBuf);       break;
        case PrecessState::ParseReqHeaders:
            flag = parseRequestHeader(readBuf);     break;
        case PrecessState::ParseReqBody:            break;    //不处理 post 请求  
        default:                                    break;
        }
        if (!flag)
            return flag;

        if (m_curState == PrecessState::ParseReqDone)
        {
            processHttpRequest(response);
            response->prepareMsg(sendBuf, socket);
        }
    }
    m_curState = PrecessState::ParseReqLine;   // 状态还原, 保证还能继续处理第二条及以后的请求
    return flag;
}

bool HttpRequest::processHttpRequest(shared_ptr<HttpResponse>& response)
{
    if (strcasecmp(m_method.data(), "get") != 0)    //不处理 非 get 请求
    {
        return -1;
    }
    m_url = decodeMsg(m_url);   //因为当请求url 出现中文时，httpRequest 中的url 会出现类似这种Linux%E5%86%85%E6%A0%B8.jpg
    // 处理客户端请求的静态资源(目录或者文件)
    const char* file = NULL;
    if (strcmp(m_url.data(), "/") == 0)
    {
        file = "./resource/index.html";
    }
    else if(strcmp(m_url.data(), "/sourcefile") == 0){
        file = "./";
    }
    else
    {
            //   /home/parallels/Desktop/ws_cpp/t1/t1/bin    文件格式，所以要 + 1
        file = m_url.data() + 1;
    }
    // 获取文件属性
    struct stat st;
    int ret = stat(file, &st);
    if (ret == -1)
    {
        // 文件不存在 -- 回复404
        //sendHeadMsg(cfd, 404, "Not Found", getFileType(".html"), -1);
        //sendFile("404.html", cfd);
        response->setFileName("./resource/404.html");
        response->setStatusCode(StatusCode::NotFound);
        // 响应头
        response->addHeader("Content-type", getFileType(".html"));
        response->sendDataFunc = sendFile;
        return false;
    }
    //文件存在时
    response->setFileName(file);
    response->setStatusCode(StatusCode::OK);
    // 判断文件类型
    if (S_ISDIR(st.st_mode))    //目录
    {
        // 把这个目录中的内容发送给客户端
        //sendHeadMsg(cfd, 200, "OK", getFileType(".html"), -1);
        //sendDir(file, cfd);
        // 响应头
        response->addHeader("Content-type", getFileType(".html"));
        response->sendDataFunc = sendDir;
    }
    else
    {
        // 把文件的内容发送给客户端
        //sendHeadMsg(cfd, 200, "OK", getFileType(file), st.st_size);
        //sendFile(file, cfd);
        // 响应头
        response->addHeader("Content-type", getFileType(file));
        response->addHeader("Content-length", to_string(st.st_size));
        response->sendDataFunc = sendFile;
    }
    return true;
}

void HttpRequest::sendDir(string dirName, Buffer* sendBuf, int cfd)
{
    char buf[4096] = { 0 };
    sprintf(buf, "<html><head><title>%s</title></head><body><table>", dirName.data());
    struct dirent** namelist;
    int num = scandir(dirName.data(), &namelist, NULL, alphasort);
    for (int i = 0; i < num; ++i)
    {
        // 取出文件名 namelist 指向的是一个指针数组 struct dirent* tmp[]
        char* name = namelist[i]->d_name;
        struct stat st;
        char subPath[1024] = { 0 };
        sprintf(subPath, "%s/%s", dirName.data(), name);
        stat(subPath, &st);
        if (S_ISDIR(st.st_mode))
        {
            sprintf(buf + strlen(buf),
                "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>",
                name, name, st.st_size);
        }
        else
        {
            sprintf(buf + strlen(buf),
                "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
                name, name, st.st_size);
        }
        // send(cfd, buf, strlen(buf), 0);
        sendBuf->appendString(buf);
#ifndef MSG_SEND_AUTO
        sendBuf->sendData(cfd);
#endif
        memset(buf, 0, sizeof(buf));
        free(namelist[i]);
    }
    sprintf(buf, "</table></body></html>");
    // send(cfd, buf, strlen(buf), 0);
    sendBuf->appendString(buf);
#ifndef MSG_SEND_AUTO
    sendBuf->sendData(cfd);
#endif
    free(namelist);
}

void HttpRequest::sendFile(string fileName, Buffer* sendBuf, int cfd)
{
    printf("open file : %s\n",fileName.c_str());
    int fd = open(fileName.data(), O_RDONLY);   //报错处
    if(fd<=0 ){
        perror("open");
        exit(0);
    }
#if 1
    while (1)
    {
        char buf[1024];
        int len = read(fd, buf, sizeof buf);
        if (len > 0)
        {
            sendBuf->appendString(buf, len);
#ifndef MSG_SEND_AUTO
            sendBuf->sendData(cfd);
#endif
        }
        else if (len == 0)  
            break;
        else
        {
            close(fd);
            perror("read");
        }
    }
#else
    off_t offset = 0;
    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    while (offset < size)
    {
        int ret = sendfile(cfd, fd, &offset, size - offset);
        printf("ret value: %d\n", ret);
        if (ret == -1 && errno == EAGAIN)
        {
            printf("没数据...\n");
        }
    }
#endif
    close(fd);
}

// 将字符转换为整形数
int HttpRequest::hexToDec(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return 0;
}

string HttpRequest::decodeMsg(string msg)
{
    string str = string();
    const char* from = msg.data();
    for (; *from != '\0'; ++from)
    {
        // isxdigit -> 判断字符是不是16进制格式, 取值在 0-f
        // Linux%E5%86%85%E6%A0%B8.jpg
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
        {
            // 将16进制的数 -> 十进制 将这个数值赋值给了字符 int -> char
            // B2 == 178
            // 将3个字符, 变成了一个字符, 这个字符就是原始数据
            str.append(1, hexToDec(from[1]) * 16 + hexToDec(from[2]));

            // 跳过 from[1] 和 from[2] 因此在当前循环中已经处理过了
            from += 2;
        }
        else
        {
            // 字符拷贝, 赋值
            str.append(1, *from);
        }
    }
    str.append(1, '\0');
    return str;
}

const string HttpRequest::getFileType(const string name)
{
    // a.jpg a.mp4 a.html
    // 自右向左查找‘.’字符, 如不存在返回NULL
    const char* dot = strrchr(name.data(), '.');
    if (dot == NULL)
        return "text/plain; charset=utf-8";	// 纯文本
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".au") == 0)
        return "audio/basic";
    if (strcmp(dot, ".wav") == 0)
        return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
        return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
        return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
        return "application/ogg";
    if (strcmp(dot, ".pac") == 0)
        return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";
}