cmake_minimum_required(VERSION 3.13)

project(giagui)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable")

include_directories(source)

option(ENABLE_ASSERT "Enable additional assertions" OFF)
if(ENABLE_ASSERT)
    add_compile_definitions(ENABLE_ASSERT=1)
endif()

option(ENABLE_DRAW_SELECTION "Enable drawing the currently selected polygon in edit mode" OFF)
if(ENABLE_DRAW_EDIT_MODE_SELECTION)
    add_compile_definitions(ENABLE_DRAW_EDIT_MODE_SELECTION=1)
endif()

add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x051200) # We don't want old APIs
add_compile_definitions(QT_NO_CAST_FROM_ASCII) # Disable ascii strings in qt API
add_compile_definitions(QT_NO_CAST_TO_ASCII)   # Disable ascii strings in qt API

find_package(h3 REQUIRED)

find_package(cpptoml REQUIRED)

### Qt stuff ###
set(CMAKE_INCLUDE_CURRENT_DIR ON) # Find includes in corresponding build directories
set(CMAKE_AUTOMOC ON) # Instruct CMake to run the moc automatically when needed
set(CMAKE_AUTOUIC ON) # Create code from a list of Qt designer ui files
set(CMAKE_AUTORCC ON) # Run resource compiler
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg     REQUIRED)

set(SOURCE_FILES
        source/main.cpp
        source/map.hpp source/map.cpp
        source/MainWindow.cpp source/MainWindow.hpp
        source/MapWindow.cpp source/MapWindow.hpp
        source/MapView.cpp source/MapView.hpp)

set(UI_FILES
        source/mainwindow.ui)

set(RESOURCE_FILES
        resources/resources.qrc)

add_executable(${PROJECT_NAME}
        ${SOURCE_FILES}
        ${UI_FILES}
        ${RESOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
        h3::h3
        cpptoml
        Qt5::Widgets
        Qt5::Svg)
