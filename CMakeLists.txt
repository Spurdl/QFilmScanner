cmake_minimum_required(VERSION 3.16)
project(QFilmScanner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SANE REQUIRED sane-backends)

add_executable(QFilmScanner
    src/main.cpp
    src/MainWindow.cpp
    src/workers/ScanWorker.cpp
    src/workers/ImageWorker.cpp
)

target_include_directories(QFilmScanner PRIVATE
    ${SANE_INCLUDE_DIRS}
    src/workers
    src/device_quirks
    src/device_quirks/profiles
)

target_link_libraries(QFilmScanner PRIVATE
    Qt6::Widgets
    ${SANE_LIBRARIES}
)
