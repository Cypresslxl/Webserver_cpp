cmake_minimum_required(VERSION 3.10)

project(main)


# EXECUTABLE_OUTPUT_PATH ：目标二进制可执行文件的存放位置       PROJECT_SOURCE_DIR：工程的根目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})


# include_directories。该命令是用来向工程添加多个指定头文件的搜索路径，路径之间用空格分隔。
# include_directories(dispatcher)
# include_directories(include)
# include_directories(src)
# include_directories(task)
# include_directories(utility)
# include_directories(thread)

aux_source_directory(./dispatcher   DIR_DISPATCHER_SRCS)
aux_source_directory(./kits         DIR_INCLUDE_SRCS)
aux_source_directory(./src          DIR_SRC_SRCS)
aux_source_directory(./task         DIR_TASK_SRCS)
aux_source_directory(./thread       DIR_THREAD_SRCS)
aux_source_directory(./utility      DIR_UTILITY_SRCS)

# 添加编译选项
add_compile_options(-std=c++17 -pthread -Wall)

find_package(Threads)   #当使用Cmake编译多线程程序时，需要手动添加pthread库，如果手动添加，就会报错：

set(INC_DIR /usr/include)
set(LINK_DIR /usr/local/lib)

# 包含目录：相当于 -I
include_directories(${INC_DIR})

# 链接库目录：相当于 -L
# link_directoried(${LINK_DIR})
link_directories(${LINK_DIR})


add_executable(main main.cpp ${DIR_DISPATCHER_SRCS} 
                             ${DIR_INCLUDE_SRCS} 
                             ${DIR_SRC_SRCS} 
                             ${DIR_TASK_SRCS} 
                             ${DIR_THREAD_SRCS} 
                             ${DIR_UTILITY_SRCS})
                             
# 引用库：需要放在 ADD_EXECUTABLE 后
# target_link_libraries(target pthread cjson)
                             
# 引入线程和json 静态库
target_link_libraries(main ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(main jsoncpp)     