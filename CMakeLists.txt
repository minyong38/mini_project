cmake_minimum_required(VERSION 3.16)

project(SharedScheduleManager VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Sql Multimedia MultimediaWidgets)

# 공통 헤더 포함 경로
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 1. 서버 타겟 설정
add_executable(Server
    server_main.cpp
    Server.cpp
    Server.h
    Common.h
)
target_link_libraries(Server PRIVATE Qt6::Core Qt6::Network Qt6::Sql)

# 2. 클라이언트 타겟 설정
add_executable(Client
    client_main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    CustomCalendarWidget.cpp
    CustomCalendarWidget.h
    WeatherManager.cpp
    WeatherManager.h
    GoogleAuthManager.cpp
    GoogleAuthManager.h
    LoginDialog.cpp
    LoginDialog.h
    RegisterDialog.cpp
    RegisterDialog.h
    ScheduleDialog.cpp
    ScheduleDialog.h
    scheduledialog.ui
    ChatDialog.cpp
    ChatDialog.h
    SharedCalDialog.h
    MyPageDialog.cpp
    MyPageDialog.h
    SearchDialog.cpp
    SearchDialog.h
    Common.h
    resources.qrc
)
target_link_libraries(Client PRIVATE Qt6::Widgets Qt6::Network Qt6::Multimedia Qt6::MultimediaWidgets)
