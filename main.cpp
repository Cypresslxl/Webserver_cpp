#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "./src/TcpServer.h"
#include "./utility/Singleton.h"
#include "./utility/Logger.h"
#include "./utility/System.h"
#include <iostream>
#include <json/json.h>
#include "./utility/jjson.h"

using namespace::Cypresslxl::utility;
using namespace::std;



int main(int argc, char* argv[])
{
#if 0
    if (argc < 3)
    {
        cout<<"./a.out port path\n";
        return -1;
    }
    unsigned short port = atoi(argv[1]);

    chdir(argv[2]);
#else

    string resourcefile = jjson::instance()->get<std::string>("resourcefile");
    uint16_t port       = jjson::instance()->get<int>("port");
    string root         = System::getinstance()->get_root_path();
    chdir(root.c_str());
#endif

    // TcpServer* server = Singleton<TcpServer>::instance();
    TcpServer* server = new TcpServer(port,std::thread::hardware_concurrency());
    server->run();

    return 0;
}
