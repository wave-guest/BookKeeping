cmake_minimum_required(VERSION 3.22)

project(MyProject)


set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Od /Zi")

#用环境变量
set(SDKPATH $ENV{SDKPATH})
set(Qt_DIR $ENV{Qt6_DIR})

#设置QT版本
set(QT_VERSION_MAJOR 6)

#头文件路径
include_directories(${SDKPATH}/include)

#动态库路径
link_directories(${SDKPATH}/lib)


if (MSVC)
    add_compile_options(/utf-8)
else()
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置全局输出路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${SDKPATH}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${SDKPATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${SDKPATH}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${SDKPATH})


# 添加子目录
add_subdirectory(MyWindow)
add_subdirectory(DataCenter)
add_subdirectory(MainApp)
add_subdirectory(Tests)
