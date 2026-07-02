project(DataCenter)

# 包含 Qt 配置模块
include(${CMAKE_SOURCE_DIR}/QtConfig.cmake)

add_definitions(-DDATA_CENTER_LIBRARY)

set(MODULEPATH $ENV{MODULEPATH})
set(SQLITE_INCLUDE_DIR ${MODULEPATH}/SQLite/include)
set(SQLITE_LIB_DIR ${MODULEPATH}/SQLite/lib)

include_directories(
	${SQLITE_INCLUDE_DIR}
)
link_directories(
	${SQLITE_LIB_DIR}
)

message(${SQLITE_INCLUDE_DIR})
setup_qt_library(${PROJECT_NAME}
	MODULES Core
	LIBRARIES ConfigCore sqlite3
)