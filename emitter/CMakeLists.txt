cmake_minimum_required(VERSION 3.10)
project(RobotVRStreamer VERSION 1.0)

# Forzar el estándar C++17 para un manejo moderno de memoria
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Encontrar la librería estándar de hilos de Linux (pthread)
find_package(Threads REQUIRED)

# Utilizar pkg-config para localizar GStreamer dinámicamente
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)

# Vincular las rutas de los headers de GStreamer al compilador
include_directories(${GST_INCLUDE_DIRS})

# Declarar el ejecutable y compilar ambas clases
add_executable(robot_core main.cpp emitter.cpp)

# Enlazar explícitamente GStreamer y los hilos al binario final
target_link_libraries(robot_core 
    PRIVATE 
    ${GST_LIBRARIES}
    Threads::Threads
)