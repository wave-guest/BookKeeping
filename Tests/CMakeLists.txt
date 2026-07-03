project(DataCenterTest)

include(${CMAKE_SOURCE_DIR}/QtConfig.cmake)

set(MODULEPATH $ENV{MODULEPATH})
set(SQLITE_INCLUDE_DIR ${MODULEPATH}/SQLite/include)
set(SQLITE_LIB_DIR ${MODULEPATH}/SQLite/lib)

include_directories(
    ${SQLITE_INCLUDE_DIR}
)
link_directories(
    ${SQLITE_LIB_DIR}
)

set(CMAKE_AUTOMOC ON)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

file(GLOB SOURCES "*.cpp")

add_executable(${PROJECT_NAME}
    ${SOURCES}
)

target_link_libraries(${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    DataCenter
    sqlite3
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${SDKPATH}/include
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${SDKPATH}
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

if(MSVC)
    add_compile_options(/utf-8)
endif()