IO多路复用：
-   select,poll,epoll (linux下)
        IO多路复用是指使用单个线程同时处理多个IO请求。
        在IO多路复用模型中一个线程可以监视多个文件描述符(fd)，
        一旦某个fd就绪（读/写就绪）或者超时，就能够通知应用程序进行相应的读写操作。
        IO多路复用使线程不会阻塞在某一个特定的IO请求上，而是不断地去对内核通知的请求进行处理，
        其具体实现方式有select、poll和epoll三种。


I/O 模型通过dispatch() 监听事件


基类：
    Dispatcher ： 
        virtual int add();
        virtual int remove();
        virtual int modify();
        virtual int dispatch(int timeout = 2); // 单位: s

派生类: 
    SelectDispatcher
    PollDispatcher
    EpollDispatcher




