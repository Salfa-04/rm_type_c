cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
set(CMAKE_PROJECT_NAME              components)
##########################################
### Export CMakeLists.txt for Components
##########################################
project(${CMAKE_PROJECT_NAME})
add_library(${CMAKE_PROJECT_NAME} INTERFACE)
target_link_libraries(${CMAKE_PROJECT_NAME} INTERFACE
    example
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
