# QtConfig.cmake

function(setup_qt_library TARGET_NAME)
	cmake_parse_arguments(SETUP "" "" "MODULES;LIBRARIES" ${ARGN})
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${SETUP_MODULES})

    # 添加当前路径下的所有.cpp文件和.h文件
    file(GLOB SOURCES "*.cpp")
    file(GLOB HEADERS "*.h" "*.hpp")
    file(GLOB UIS "*.ui")

    # 使用 CMAKE_AUTOMOC 来自动处理 Qt 元对象编译
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
	
    # 生成动态库
    add_library(${TARGET_NAME} SHARED ${SOURCES} ${HEADERS} ${UIS})

    # 添加宏定义
    target_compile_definitions(${TARGET_NAME} PRIVATE BUILDING_MYQT_LIBRARY)

    # 自动生成包含 Qt 元对象、UI 文件和资源文件的头文件
    target_sources(${TARGET_NAME} PRIVATE ${UIS})
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}) # 用于自动生成的头文件
    #qt5_wrap_ui(UI_HEADERS ${UIS})

    message(STATUS "TARGET_NAME: ${TARGET_NAME}")


    # 设置输出路径
    set_target_properties(${TARGET_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${SDKPATH}
        ARCHIVE_OUTPUT_DIRECTORY ${SDKPATH}
        RUNTIME_OUTPUT_DIRECTORY ${SDKPATH}
    )

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${SDKPATH}/include
    )
   # 链接 Qt 库
    foreach(module IN LISTS SETUP_MODULES)
        target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::${module})
    endforeach()

   foreach(module IN LISTS SETUP_LIBRARIES)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${module})
   endforeach()



    # 设置 C++ 标准
    set_target_properties(${TARGET_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
	
	source_group("Header Files" FILES ${HEADERS})
	source_group("Source Files" FILES ${SOURCES})
	source_group("UI Files" FILES ${UIS})
	source_group("zero" REGULAR_EXPRESSION "^.*ui_.*$|mocs_compilation_[^/]*\\.cpp$|\\.stamp$|\\.rule$")

    copy_header_files_recursive(${CMAKE_CURRENT_SOURCE_DIR} ${SDKPATH}/include)
	
	if(MSVC)
		# 添加额外的编译选项
		target_compile_options(${TARGET_NAME} PRIVATE 
			/wd4251  # 禁用类需要导出接口的警告
			/wd4273  # 禁用不一致的dll链接警告
		)
		
		# 导出宏由各模块头文件自行控制（DATA_CENTER_EXPORT / MYWINDOW_EXPORT）
		# 不再使用 WINDOWS_EXPORT_ALL_SYMBOLS
	endif()
	
endfunction()


function(setup_qt_exe PROJECT_NAME)
    # 查找 Qt5 模块
	cmake_parse_arguments(SETUP "" "" "MODULES;LIBRARIES" ${ARGN})
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${SETUP_MODULES})
	
	file(GLOB SOURCES "*.cpp")
    file(GLOB HEADERS "*.h" "*.hpp")
    file(GLOB UIS "*.ui")
	
	set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
	
	
    # 添加可执行文件
    add_executable(${PROJECT_NAME} WIN32
        ${SOURCES}
        ${HEADERS}
		${UIS}
    )
	
    # 链接 Qt 库
    foreach(module IN LISTS SETUP_MODULES)
        target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::${module})
    endforeach()

    # 链接额外的库
    foreach(module IN LISTS SETUP_LIBRARIES)
        target_link_libraries(${PROJECT_NAME} ${module})
    endforeach()


    # 设置 C++ 标准
    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    # 设置 include 目录
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${SDKPATH}/include
    )

    # 设置可执行文件的输出路径
    set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${SDKPATH}
    )
	
	source_group("zero" FILES ${MOC_SOURCES})
	source_group("Header Files" FILES ${HEADERS})
	source_group("Source Files" FILES ${SOURCES})
	source_group("UI Files" FILES ${UIS})
	source_group("zero" REGULAR_EXPRESSION "^.*ui_.*$|mocs_compilation_[^/]*\\.cpp$|\\.stamp$|\\.rule$")


endfunction()


function(include_subdirectories)
    # 获取当前目录下的所有子目录
    file(GLOB SUBDIRECTORIES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*/")
    
    # 遍历每个子目录
    foreach(SUBDIR ${SUBDIRECTORIES})
        # 检查子目录中是否存在 CMakeLists.txt 文件
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR}/CMakeLists.txt)
            # 如果存在，则包含该子目录
            add_subdirectory(${SUBDIR})
        endif()
    endforeach()
endfunction()


function(copy_header_files SDKPATH)
    # 获取当前目录下的所有 .h 和 .hpp 文件
    file(GLOB HEADER_FILES "*.h" "*.hpp")
    
    # 遍历每个头文件
    foreach(HEADER_FILE ${HEADER_FILES})
        # 将头文件复制到目标目录
        file(COPY ${HEADER_FILE} DESTINATION ${SDKPATH})
    endforeach()
endfunction()

function(copy_header_files_recursive CURRENT_DIR SDKPATH)
    # 获取当前目录下的所有 .h 和 .hpp 文件
    file(GLOB HEADER_FILES "${CURRENT_DIR}/*.h" "${CURRENT_DIR}/*.hpp")

    # 拷贝当前目录下的头文件
    foreach(HEADER_FILE ${HEADER_FILES})
        # 获取头文件的相对路径（相对于起始目录）
        file(RELATIVE_PATH RELATIVE_HEADER_PATH ${CMAKE_SOURCE_DIR} ${HEADER_FILE})
        # 拼接目标路径
        set(TARGET_PATH "${SDKPATH}/${RELATIVE_HEADER_PATH}")
        # 创建目标目录（如果不存在）
        get_filename_component(TARGET_DIR ${TARGET_PATH} DIRECTORY)
        file(MAKE_DIRECTORY ${TARGET_DIR})
        # 拷贝文件
        file(COPY ${HEADER_FILE} DESTINATION ${TARGET_DIR})
    endforeach()

    # 递归处理子目录
    file(GLOB SUBDIRECTORIES LIST_DIRECTORIES true RELATIVE ${CURRENT_DIR} "${CURRENT_DIR}/*")
    foreach(SUBDIR ${SUBDIRECTORIES})
        if(IS_DIRECTORY "${CURRENT_DIR}/${SUBDIR}")
            copy_header_files_recursive("${CURRENT_DIR}/${SUBDIR}" "${SDKPATH}")
        endif()
    endforeach()
endfunction()
