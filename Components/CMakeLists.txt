cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
set(CMAKE_PROJECT_NAME              components)
##########################################
### Export CMakeLists.txt for Components
##########################################
project(${CMAKE_PROJECT_NAME})
add_library(${CMAKE_PROJECT_NAME} INTERFACE)
target_link_libraries(${CMAKE_PROJECT_NAME} INTERFACE
    algo bled buzzer
)



##########################################
### Link Algorithm Libraries
##########################################
add_library(algo INTERFACE)
file(GLOB COMPONENTS_ALGO Algorithm/*.c)
target_include_directories(algo INTERFACE
    Algorithm/Include
)
target_sources(algo INTERFACE ${COMPONENTS_ALGO})
target_link_libraries(algo INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/Algorithm/AHRS.lib
)


##########################################
### Board LED Driver
##########################################
add_library(bled INTERFACE)
file(GLOB COMPONENTS_BLED BLED/*.c)
target_include_directories(bled INTERFACE BLED)
target_sources(bled INTERFACE
    ${COMPONENTS_BLED}
)


##########################################
### Buzzer Driver
##########################################
add_library(buzzer INTERFACE)
file(GLOB COMPONENTS_BUZZER Buzzer/*.c)
target_include_directories(buzzer INTERFACE Buzzer)
target_sources(buzzer INTERFACE
    ${COMPONENTS_BUZZER}
)



##########################################
### Example Driver
##########################################

add_library(example INTERFACE)

file(GLOB COMPONENTS_EXAMPLE example/*.c)

target_compile_definitions(example INTERFACE
)

target_include_directories(example INTERFACE
    Example/Include
)

target_sources(example INTERFACE
    ${COMPONENTS_EXAMPLE}
)
