cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
set(CMAKE_PROJECT_NAME              drivers)
##########################################
### Export CMakeLists.txt for Drivers
##########################################
project(${CMAKE_PROJECT_NAME})
add_library(${CMAKE_PROJECT_NAME} INTERFACE)
target_link_libraries(${CMAKE_PROJECT_NAME} INTERFACE
    stm32cubemx freertos usbdevice
)



##########################################
### STM32CubeMX CMSIS&HAL Driver
##########################################

add_library(stm32cubemx INTERFACE)

file(GLOB HAL_SOURCE F4HAL/Src/*.c)

target_compile_definitions(stm32cubemx INTERFACE
    $<$<CONFIG:Debug>:DEBUG>
    USE_HAL_DRIVER STM32F407xx
)

target_include_directories(stm32cubemx INTERFACE
    CMSIS/Device/ST/STM32F4xx/Include
    F4HAL/Inc F4HAL/Inc/Legacy CMSIS/Include
)

target_sources(stm32cubemx INTERFACE ${HAL_SOURCE})


##########################################
### FreeRTOS Driver
##########################################

add_library(freertos INTERFACE)

file(GLOB_RECURSE FREERTOS_SRC FreeRTOS/*.c)

target_include_directories(freertos INTERFACE
    FreeRTOS/portable/CMSIS
    FreeRTOS/portable/GCC/ARM_CM4F
    FreeRTOS/include
)

target_sources(freertos INTERFACE ${FREERTOS_SRC})


##########################################
### USB Device Driver
##########################################

add_library(usbdevice INTERFACE)

file(GLOB_RECURSE USBDEVICE_SRC USBDevice/*.c)

target_include_directories(usbdevice INTERFACE
    USBDevice/Class/CDC/Inc
    USBDevice/Core/Inc
)

target_sources(usbdevice INTERFACE ${USBDEVICE_SRC})
