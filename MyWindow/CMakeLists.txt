project(MyWindow)

# 包含 Qt 配置模块
include(${CMAKE_SOURCE_DIR}/QtConfig.cmake)

add_definitions(-DMYWINDOW_LIBRARY)

setup_qt_library(${PROJECT_NAME}
	MODULES Core Gui Widgets Charts
	LIBRARIES DataCenter
)