cmake_minimum_required(VERSION 3.16)
project(QFilmScanner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(VERSION_STRING "0.2.3")
add_definitions(-DVERSION_STRING="${VERSION_STRING}")

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SANE REQUIRED sane-backends)

#qt6_add_resources(QFilmScanner "fonts/MontserratLight.ttf"
#    PREFIX "/fonts"
#    FILES "fonts/MontserratLight.ttf")

qt_add_resources(sourcesrcc ${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc)

qt6_add_executable(QFilmScanner
    src/main.cpp
    src/MainWindow.cpp
    src/workers/ScanWorker.cpp
    src/workers/ImageWorker.cpp
    src/devices/deviceQuirks.cpp
    ${sourcesrcc}
)

target_include_directories(QFilmScanner PRIVATE
    ${SANE_INCLUDE_DIRS}
    src/workers
    src/devices
)

target_link_libraries(QFilmScanner PRIVATE
    Qt6::Widgets
    ${SANE_LIBRARIES}
)