cmake_minimum_required(VERSION 3.16)
project(Slidenote VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Tools)

# QHotkey
include(FetchContent)
FetchContent_Declare(
    QHotkey
    GIT_REPOSITORY https://github.com/Hellmark/QHotkey.git
    GIT_TAG master
)
FetchContent_MakeAvailable(QHotkey)

# Add X11 for Linux
if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    include_directories(${X11_INCLUDE_DIR})
    set(EXTRA_LIBS ${X11_LIBRARIES})
endif()

# for MacOS
# macOS Bundle Setup
set(MACOSX_BUNDLE_ICON_FILE Slidenote.icns)
set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.hellmark.slidenote")
set(MACOSX_BUNDLE_BUNDLE_NAME "Slidenote")
set(MACOSX_BUNDLE_COPYRIGHT "Copyright 2025 Hellmark Programming Group")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")

# Set icon and Info.plist if present
set(APP_ICON_MAC "${CMAKE_SOURCE_DIR}/resources/Slidenote.icns")
set(MY_INFO_PLIST "${CMAKE_SOURCE_DIR}/resources/Info.plist")
if(APPLE AND EXISTS "${APP_ICON_MAC}")
    set_source_files_properties(${APP_ICON_MAC} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

# Add executable with bundle support for macOS
if(APPLE)
    set(MACOSX_BUNDLE_FLAG MACOSX_BUNDLE)
else()
    set(MACOSX_BUNDLE_FLAG "")
endif()

add_executable(Slidenote ${MACOSX_BUNDLE_FLAG}
    main.cpp
    SlideWindow.cpp
    SlideWindow.h
    Slidenote.qrc
    SettingsDialog.h
    SettingsDialog.cpp
    ${APP_ICON_MAC}
)

target_include_directories(Slidenote PUBLIC ${QHotkey_SOURCE_DIR}/QHotkey)
target_link_libraries(Slidenote Qt6::Core Qt6::Gui Qt6::Widgets QHotkey::QHotkey ${EXTRA_LIBS})

# Finalize Qt executable (for plugin deployment, etc.)
# Apply Info.plist to bundle
if(APPLE)
    find_package(Qt6MacDeployQt QUIET)
    set_target_properties(Slidenote PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${MY_INFO_PLIST}"
    )
    # Try to finalize executable if tools are present
    find_package(Qt6MacDeployQt QUIET)
    if (Qt6MacDeployQt_FOUND)
        include(Qt6MacDeployQt)
        qt6_finalize_executable(Slidenote)
    else()
        message(WARNING "Qt6MacDeployQt not found. Manually run macdeployqt after building.")
    endif()
endif()
