cmake_minimum_required(VERSION 3.16)
project(HouseWeb VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Sql WebEngineWidgets)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/databasemanager.cpp
    src/usermanager.cpp
    src/houseinfo.cpp
    src/deepseekaiservice.cpp
    src/configmanager.cpp
    src/emailservice.cpp
    src/webserver.cpp
)

set(HEADERS
    include/mainwindow.h
    include/databasemanager.h
    include/usermanager.h
    include/houseinfo.h
    include/deepseekaiservice.h
    include/configmanager.h
    include/emailservice.h
    include/webserver.h
)

qt_add_resources(RESOURCES resources/resources.qrc)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES})

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::WebEngineWidgets
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES config.json DESTINATION bin)
install(DIRECTORY web/ DESTINATION bin/web)
