cmake_minimum_required(VERSION 3.16)

project(MprisDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

# 添加 X11 库
find_package(X11 REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets DBus WaylandCompositor Gui)
find_package(LayerShellQt REQUIRED)

add_executable(MprisDemo
    main.cpp

    MprisPlayer.cpp
    MprisPlayer.h

    LyricParser.cpp
    LyricParser.h

    LyricFinder.cpp
    LyricFinder.h

    LyricSelectionDialog.cpp
    LyricSelectionDialog.h

    DesktopLyricWindow.cpp
    DesktopLyricWindow.h

    LyricWindowController.cpp
    LyricWindowController.h

    LyricTray.cpp
    LyricTray.h

    LyricsStyleDialog.h
    LyricsStyleDialog.cpp
)

set_source_files_properties(mpris_test/player.cpp PROPERTIES SKIP_AUTOMOC ON)

add_executable(test
    mpris_test/player.cpp)

target_link_libraries(MprisDemo
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::DBus
        Qt6::WaylandCompositor
        LayerShellQt::Interface
        ${X11_LIBRARIES}
)

target_link_libraries(test
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::DBus
)