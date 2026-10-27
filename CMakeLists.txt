cmake_minimum_required(VERSION 3.16)

project(p1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(p1
    src/main.cpp
    src/model/Vertex.h
    src/model/Edge.h
    src/model/Constraint.h
    src/model/Polygon.h
    src/model/Polygon.cpp
    src/model/Scene.h
    src/model/Scene.cpp
    src/ui/CanvasWidget.h
    src/ui/CanvasWidget.cpp
    src/render/LineDrawing.h
    src/render/LineDrawing.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/render/Bezier.h
    src/render/Bezier.cpp
    src/render/Arc.h
    src/render/Arc.cpp
    src/ui/HelpDialog.h
    src/ui/HelpDialog.cpp
    resources.qrc
)

target_link_libraries(p1 PRIVATE Qt6::Widgets)

target_include_directories(p1 PRIVATE src)
