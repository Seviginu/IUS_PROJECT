#pragma once

#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

void initLogger(const char *filename);
void logWriterTask(void *pvParams);

/* Объявления hook-функций */
void vApplicationTaskSwitchedInHook(void);
void vApplicationTaskSwitchedOutHook(void);
void vApplicationMallocFailedHook(void);
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);
void vApplicationTaskCreateHook(TaskHandle_t xCreatedTask);
void vApplicationTaskDeleteHook(TaskHandle_t xTaskToDelete);
void vApplicationBlockingHook(void *xQueue);

#ifdef __cplusplus
}
#endif