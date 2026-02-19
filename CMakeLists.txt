cmake_minimum_required(VERSION 3.16)
project(film_scanner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SANE REQUIRED sane-backends)

add_executable(film_scanner
    src/main.cpp
    src/MainWindow.cpp
    src/ScanWorker.cpp
)

target_include_directories(film_scanner PRIVATE
    ${SANE_INCLUDE_DIRS}
)

target_link_libraries(film_scanner PRIVATE
    Qt6::Widgets
    ${SANE_LIBRARIES}
)
