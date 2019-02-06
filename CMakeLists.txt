cmake_minimum_required(VERSION 3.13)
project(giagui)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable")

include_directories(source)
#include_directories(${CMAKE_BINARY_DIR}/${PROJECT_NAME}_autogen/include)

find_package(h3 REQUIRED)

### Qt stuff ###
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)
# Run resource compiler
set(CMAKE_AUTORCC ON)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg     REQUIRED)

set(SOURCE_FILES
        source/main.cpp
        source/MainWindow.cpp source/MainWindow.hpp
        source/map.hpp
        source/MapView.cpp source/MapView.hpp)

set(UI_FILES
        source/MainWindow.ui)

set(RESOURCE_FILES
        resources/resources.qrc)

add_executable(${PROJECT_NAME}
        ${SOURCE_FILES}
        ${UI_FILES}
        ${RESOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
        h3::h3
        Qt5::Widgets
        Qt5::Svg)
