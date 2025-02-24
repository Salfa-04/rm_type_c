//! ----------------------------------------------------------
//!         See http://www.freertos.org/a00110.html
//! ----------------------------------------------------------

#ifndef __FREERTOS_CONFIG_H
#define __FREERTOS_CONFIG_H

/* Ensure stdint is only used by the compiler, and not the assembler. */
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
#include <type_def.h>
extern uint32_t SystemCoreClock;
#endif

#define USE_CMSIS_V2_PRIORITIES 1

#define configUSE_PREEMPTION 1
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configSUPPORT_STATIC_ALLOCATION 1
#define configCPU_CLOCK_HZ (SystemCoreClock)
#define configTICK_RATE_HZ ((TickType_t)1000)
#define configMINIMAL_STACK_SIZE ((uint16_t)128)
#define configTOTAL_HEAP_SIZE ((size_t)(15 * 1024))
#define configMAX_TASK_NAME_LEN (16)
#define configUSE_TRACE_FACILITY 1
#define configUSE_16_BIT_TICKS 0
#define configIDLE_SHOULD_YIELD 1
#define configUSE_MUTEXES 1
#define configQUEUE_REGISTRY_SIZE 8
#define configCHECK_FOR_STACK_OVERFLOW 0
#define configUSE_RECURSIVE_MUTEXES 1
#define configUSE_MALLOC_FAILED_HOOK 0
#define configUSE_APPLICATION_TASK_TAG 0
#define configUSE_COUNTING_SEMAPHORES 1
#define configGENERATE_RUN_TIME_STATS 0
#define configRECORD_STACK_HIGH_ADDRESS 1

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 0
#define configMAX_CO_ROUTINE_PRIORITIES (2)

/* Software timer definitions. */
#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY (2)
#define configTIMER_QUEUE_LENGTH 10
#define configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE * 2)

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet 0
#define INCLUDE_uxTaskPriorityGet 0
#define INCLUDE_vTaskDelete 0
#define INCLUDE_vTaskCleanUpResources 0
#define INCLUDE_vTaskSuspend 0
#define INCLUDE_vTaskDelayUntil 1
#define INCLUDE_vTaskDelay 1
#define INCLUDE_xTaskGetSchedulerState 0

#define INCLUDE_xTaskGetIdleTaskHandle 0
#define INCLUDE_xTaskAbortDelay 0
#define INCLUDE_xQueueGetMutexHolder 0
#define INCLUDE_xSemaphoreGetMutexHolder 0
#define INCLUDE_xTaskGetHandle 1
#define INCLUDE_uxTaskGetStackHighWaterMark 0
#define INCLUDE_eTaskGetState 0
#define INCLUDE_xTaskResumeFromISR 0
#define INCLUDE_xTimerPendFunctionCall 0
#define INCLUDE_xTaskGetCurrentTaskHandle 0

/*------------- CMSIS-RTOS V2 specific defines -----------*/
/* When using CMSIS-RTOSv2 set configSUPPORT_STATIC_ALLOCATION
 * to 1 is mandatory to avoid compile errors.
 * CMSIS-RTOS V2 implmentation requires the following defines

#define configSUPPORT_STATIC_ALLOCATION          1
cmsis_os threads are created using xTaskCreateStatic() API

#define configMAX_PRIORITIES (56)
Priority range in CMSIS-RTOS V2 is [0 ..= 56]

#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
when set to 1, configMAX_PRIORITIES can't be more than 32
which is not suitable for the new CMSIS-RTOS v2 priority range
*/

#if USE_CMSIS_V2_PRIORITIES
/* CMSIS-RTOSv2 defines 56 levels of priorities.
 * To be able to use them all and avoid application misbehavior,
 * configUSE_PORT_OPTIMISED_TASK_SELECTION must be set to 0
 * and configMAX_PRIORITIES to 56 */
#define configSUPPORT_STATIC_ALLOCATION 1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configMAX_PRIORITIES (56)
#else
#define configMAX_PRIORITIES (7)
#endif

/* Defaults to size_t for backward compatibility, but can be changed
   if lengths will always be less than the number of bytes in a size_t. */
#define configMESSAGE_BUFFER_LENGTH_TYPE size_t

/* the CMSIS-RTOS V2 FreeRTOS wrapper is dependent on the heap implementation
 * used by the application thus the correct define need to be enabled from
 * the list below */

// #define USE_FreeRTOS_HEAP_1
// #define USE_FreeRTOS_HEAP_2
// #define USE_FreeRTOS_HEAP_3
#define USE_FreeRTOS_HEAP_4
// #define USE_FreeRTOS_HEAP_5

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
/* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
#define configPRIO_BITS __NVIC_PRIO_BITS
#else
#define configPRIO_BITS 4 /* 15 priority levels */
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 0xf

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY \
  (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
  (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
#define configASSERT(x)       \
  if ((x) == 0) {             \
    taskDISABLE_INTERRUPTS(); \
    for (;;);                 \
  }

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
   standard names. */
#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler

/* IMPORTANT: FreeRTOS is using the SysTick as internal time base, thus make
   sure the system and peripherials are using a different time base (TIM based
   for example).
 */
#define xPortSysTickHandler SysTick_Handler

#endif /* __FREERTOS_CONFIG_H */
