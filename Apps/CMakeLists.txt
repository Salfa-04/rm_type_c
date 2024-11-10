cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
set(CMAKE_PROJECT_NAME              applications)
##########################################
### Export CMakeLists.txt for Applications
##########################################
project(${CMAKE_PROJECT_NAME})
add_library(${CMAKE_PROJECT_NAME} INTERFACE)
target_link_libraries(${CMAKE_PROJECT_NAME} INTERFACE
    start blinky loop device ins detect
)



##########################################
### Start Task
##########################################
add_library(start INTERFACE)
file(GLOB APPS_START Start/*.c)
target_include_directories(start INTERFACE Start/Include)
target_sources(start INTERFACE ${APPS_START})

##########################################
### Blinky Task
##########################################
add_library(blinky INTERFACE)
file(GLOB APPS_BLINKY Blinky/*.c)
target_include_directories(blinky INTERFACE Blinky)
target_sources(blinky INTERFACE ${APPS_BLINKY})

##########################################
### Loop Task
##########################################
add_library(loop INTERFACE)
file(GLOB APPS_LOOP Loop/*.c)
target_include_directories(loop INTERFACE Loop)
target_sources(loop INTERFACE ${APPS_LOOP})

##########################################
### Device Task
##########################################
add_library(device INTERFACE)
file(GLOB APPS_DEVICE Device/*.c)
target_include_directories(device INTERFACE Device)
target_sources(device INTERFACE ${APPS_DEVICE})

##########################################
### INS Task (imu)
##########################################
add_library(ins INTERFACE)
file(GLOB APPS_INS InsTask/*.c)
target_include_directories(ins INTERFACE InsTask)
target_sources(ins INTERFACE ${APPS_INS})


##########################################
### Detect Task
##########################################
add_library(detect INTERFACE)
file(GLOB APPS_DETECT Detect/*.c)
target_include_directories(detect INTERFACE Detect)
target_sources(detect INTERFACE ${APPS_DETECT})
