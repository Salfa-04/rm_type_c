cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
set(CMAKE_PROJECT_NAME              components)
##########################################
### Export CMakeLists.txt for Components
##########################################
project(${CMAKE_PROJECT_NAME})
add_library(${CMAKE_PROJECT_NAME} INTERFACE)
target_link_libraries(${CMAKE_PROJECT_NAME} INTERFACE
    algo bled buzzer laser adc can imu
    remtctrl refuart shoot
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
### Laser Driver
##########################################
add_library(laser INTERFACE)
file(GLOB COMPONENTS_LAZER Laser/*.c)
target_include_directories(laser INTERFACE Laser)
target_sources(laser INTERFACE
    ${COMPONENTS_LAZER}
)

##########################################
### ADC Driver
##########################################
add_library(adc INTERFACE)
file(GLOB COMPONENTS_ADC ADC/*.c)
target_include_directories(adc INTERFACE ADC)
target_sources(adc INTERFACE
    ${COMPONENTS_ADC}
)

##########################################
### CAN Driver
##########################################
add_library(can INTERFACE)
file(GLOB COMPONENTS_CAN CAN/*.c)
target_include_directories(can INTERFACE CAN)
target_sources(can INTERFACE
    ${COMPONENTS_CAN}
)

##########################################
### IMU Driver
##########################################
add_library(imu INTERFACE)
file(GLOB COMPONENTS_IMU IMU/*.c)
target_include_directories(imu INTERFACE IMU/Include)
target_sources(imu INTERFACE
    ${COMPONENTS_IMU}
)

##########################################
### Remote Contral Driver
##########################################
add_library(remtctrl INTERFACE)
file(GLOB COMPONENTS_REMTCTRL RemtCtrl/*.c)
target_include_directories(remtctrl INTERFACE RemtCtrl)
target_sources(remtctrl INTERFACE
    ${COMPONENTS_REMTCTRL}
)

##########################################
### Referee Driver
##########################################
add_library(refuart INTERFACE)
file(GLOB COMPONENTS_REFUART RefUART/*.c)
target_include_directories(refuart INTERFACE RefUART)
target_sources(refuart INTERFACE
    ${COMPONENTS_REFUART}
)

##########################################
### Shoot Task
##########################################
add_library(shoot INTERFACE)
file(GLOB COMPONENTS_SHOOT Shoot/*.c)
target_include_directories(shoot INTERFACE Shoot)
target_sources(shoot INTERFACE
    ${COMPONENTS_SHOOT}
)


##########################################
### Example Driver
##########################################

add_library(example INTERFACE)
file(GLOB COMPONENTS_EXAMPLE example/*.c)
target_compile_definitions(example INTERFACE)
target_include_directories(example INTERFACE Example/Include)
target_sources(example INTERFACE ${COMPONENTS_EXAMPLE})
