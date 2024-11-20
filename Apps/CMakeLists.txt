cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
set(CMAKE_PROJECT_NAME              applications)
##########################################
### Export CMakeLists.txt for Applications
##########################################
project(${CMAKE_PROJECT_NAME})
add_library(${CMAKE_PROJECT_NAME} INTERFACE)
target_link_libraries(${CMAKE_PROJECT_NAME} INTERFACE
    start loop blinky ins referee
    adc_task can_task
)



##########################################
### Start Task
##########################################
add_library(start INTERFACE)
file(GLOB APPS_START Start/*.c)
target_include_directories(start INTERFACE Start/Include)
target_sources(start INTERFACE ${APPS_START})

##########################################
### Loop Task
##########################################
add_library(loop INTERFACE)
file(GLOB APPS_LOOP Loop/*.c)
target_include_directories(loop INTERFACE Loop)
target_sources(loop INTERFACE ${APPS_LOOP})

##########################################
### Blinky Task
##########################################
add_library(blinky INTERFACE)
file(GLOB APPS_BLINKY Blinky/*.c)
target_include_directories(blinky INTERFACE Blinky)
target_sources(blinky INTERFACE ${APPS_BLINKY})

##########################################
### ADC Task
##########################################
add_library(adc_task INTERFACE)
file(GLOB APPS_ADC AdcTask/*.c)
target_include_directories(adc_task INTERFACE AdcTask)
target_sources(adc_task INTERFACE ${APPS_ADC})

##########################################
### CAN Task
##########################################
add_library(can_task INTERFACE)
file(GLOB APPS_CAN CanTask/*.c)
target_include_directories(can_task INTERFACE CanTask)
target_sources(can_task INTERFACE ${APPS_CAN})

##########################################
### INS Task (imu)
##########################################
add_library(ins INTERFACE)
file(GLOB APPS_INS InsTask/*.c)
target_include_directories(ins INTERFACE InsTask)
target_sources(ins INTERFACE ${APPS_INS})

##########################################
### Referee Task
##########################################
add_library(referee INTERFACE)
file(GLOB APPS_REFEREE Referee/*.c)
target_include_directories(referee INTERFACE Referee)
target_sources(referee INTERFACE ${APPS_REFEREE})
