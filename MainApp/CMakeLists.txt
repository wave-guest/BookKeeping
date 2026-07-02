project(BKPro)

# 包含 Qt 配置模块
include(${CMAKE_SOURCE_DIR}/QtConfig.cmake)

# 核心修改：将所有模块放在 MODULES 后面，用空格分隔
setup_qt_exe(${PROJECT_NAME}
    MODULES 
        Core 
        Gui 
        Widgets
    LIBRARIES MyWindow DataCenter

)
