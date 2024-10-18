#IDF-CMAKE
#添加*.c文件
file(GLOB CORE "core/*.c")              #核心
file(GLOB COMMON "common/*.c")              #公共
file(GLOB DRIVERS "drivers/*.c")    #驱动程序
file(GLOB SENSOR "sensor/*.c")      #传感器
file(GLOB AUDIO "audio/*.c")      #音频
file(GLOB NETWORK "network/*.c")      #网络
file(GLOB STORAGE "storage/*.c")      #存储

file(GLOB UI "ui/*.c")      #显示和触摸
file(GLOB ASSETS "assets/*.c")      #资源:图片&音频

file(GLOB SZP_COMPONENTS "szp_components/**/*.c")              #依赖库(部分idf下载的库需要修改则放在此处)
file(GLOB DEMO "demo/*.c")  #测试demo

#添加工程文件
set(PROJECT_SOURCES
        "main.c"

        ${CORE}
        ${COMMON}
        ${DRIVERS}
        ${SENSOR}
        ${AUDIO}
        ${NETWORK}
        ${STORAGE}

        ${UI}
        ${ASSETS}

        ${SZP_COMPONENTS}
        ${DEMO}
)

#添加包含目录
list(APPEND INC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/sensor
    ${CMAKE_CURRENT_SOURCE_DIR}/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/network
    ${CMAKE_CURRENT_SOURCE_DIR}/storage
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    ${CMAKE_CURRENT_SOURCE_DIR}/szp_components

    ${CMAKE_CURRENT_SOURCE_DIR}/demo
)
#IDF注册
idf_component_register(
                    SRCS  ${PROJECT_SOURCES}
                    INCLUDE_DIRS ${INC_DIR}
                    )
