cmake_minimum_required(VERSION 3.16)
project(SlideInApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# QHotkey
include(FetchContent)
FetchContent_Declare(
    QHotkey
    GIT_REPOSITORY https://github.com/Skycoder42/QHotkey.git
    GIT_TAG master
)
FetchContent_MakeAvailable(QHotkey)

# Add X11 for Linux
if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    include_directories(${X11_INCLUDE_DIR})
    set(EXTRA_LIBS ${X11_LIBRARIES})
endif()

add_executable(SlideInApp
    main.cpp
    SlideWindow.cpp
    SlideWindow.h
    Slidenote.qrc
    SettingsDialog.h
    SettingsDialog.cpp
)

target_link_libraries(SlideInApp Qt6::Core Qt6::Gui Qt6::Widgets QHotkey ${EXTRA_LIBS})
