cmake_minimum_required(VERSION 3.5)

project(chessgui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)


#include(FetchContent)
#FetchContent_Declare(
#    PVS_CMakeModule
#    GIT_REPOSITORY "https://github.com/viva64/pvs-studio-cmake-module.git"
#    GIT_TAG        "master" 
#)
#FetchContent_MakeAvailable(PVS_CMakeModule)
#include("${pvs_cmakemodule_SOURCE_DIR}/PVS-Studio.cmake")



set(PROJECT_SOURCES
src/gui/main.cxx
src/gui/mainwindow.cxx
src/gui/mainwindow.ui
src/gui/boardview.cxx
src/gui/notationview.cxx
src/db/bitboard.cxx
src/db/game.cxx
assets/assets.qrc
)

qt_add_executable(chessgui
MANUAL_FINALIZATION
${PROJECT_SOURCES}
)

target_link_libraries(chessgui PRIVATE Qt6::Widgets Qt6::Svg)
target_include_directories(chessgui PUBLIC
src/db/
)
install(TARGETS chessgui
BUNDLE DESTINATION .
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
qt_finalize_executable(chessgui)
endif()

#pvs_studio_add_target(TARGET chessgui.analyze ALL
#                      FORMAT json
#                      ANALYZE chessgui
#                      MODE GA:1,2
#                      LOG target.json)
