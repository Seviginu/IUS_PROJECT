#include "FreeRTOS.h"
#include <semphr.h>
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>
#include "logger.h"

#define LOG_BUFFER_SIZE 1024
static char logBuffer[LOG_BUFFER_SIZE];
static size_t head = 0;
static size_t tail = 0;
static SemaphoreHandle_t logMutex;
static FILE *logFile = NULL;  // Добавляем файловый дескриптор

void initLogger(const char *filename)
{
    // Инициализация файла логов
    logFile = fopen(filename, "w");
    if (logFile == NULL) {
        printf("Failed to open log file!\n");
        configASSERT(0);
    }

    logMutex = xSemaphoreCreateMutex();
    if (logMutex == NULL)
    {
        printf("Failed to create log mutex!\n");
        fclose(logFile);
        configASSERT(0);
    }

    #if (configUSE_TRACE_FACILITY == 1)
    printf("Trace facility ENABLED\n");
    #else
    printf("Trace facility DISABLED - hooks won't work!\n");
    #endif

    if (xTaskCreate(logWriterTask, "Logger", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    {
        printf("Failed to create logger task!\n");
        fclose(logFile);
        vSemaphoreDelete(logMutex);
        configASSERT(0);
    }

    printf("Logger INIT OK\n");
}

static void logWrite(const char *data, size_t len)
{
    if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        for (size_t i = 0; i < len; i++)
        {
            logBuffer[head] = data[i];
            head = (head + 1) % LOG_BUFFER_SIZE;
            if (head == tail)
            {
                tail = (tail + 1) % LOG_BUFFER_SIZE;
            }
        }
        xSemaphoreGive(logMutex);
    }
}

void logState(TaskHandle_t task, const char *state)
{
    TickType_t timestamp = xTaskGetTickCount();
    uint32_t id = (uint32_t)task;
    char logLine[64];

    int len = snprintf(logLine, sizeof(logLine), "%lu,%u,%s\n",
                       pdTICKS_TO_MS(timestamp), id, state);
    if (len > 0)
    {
        logWrite(logLine, len);
    }
}

__attribute__((weak)) void vApplicationTaskSwitchedInHook(void) {
    logState(xTaskGetCurrentTaskHandle(), "Running");
}

__attribute__((weak)) void vApplicationTaskSwitchedOutHook(void) {
    logState(xTaskGetCurrentTaskHandle(), "Waiting");
}

__attribute__((weak)) void vApplicationTaskCreateHook(TaskHandle_t xCreatedTask) {
    logState(xCreatedTask, "Created");
}

__attribute__((weak)) void vApplicationTaskDeleteHook(TaskHandle_t xTaskToDelete) {
    logState(xTaskToDelete, "Deleted");
}

__attribute__((weak)) void vApplicationBlockingHook(void *xQueue) {
    (void)xQueue;
    logState(xTaskGetCurrentTaskHandle(), "Blocked");
}

void logWriterTask(void *pvParams)
{
    (void)pvParams;
    char line[128];
    size_t idx = 0;

    while (1)
    {
        if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            while (tail != head && idx < sizeof(line) - 1)
            {
                line[idx++] = logBuffer[tail];
                tail = (tail + 1) % LOG_BUFFER_SIZE;

                if (line[idx - 1] == '\n')
                {
                    line[idx] = '\0';
                    // Вывод в консоль
                    printf("%s", line);
                    fflush(stdout);
                    
                    // Запись в файл
                    if (logFile != NULL) {
                        fprintf(logFile, "%s", line);
                        fflush(logFile);  // Обеспечиваем запись на диск
                    }
                    
                    idx = 0;
                }
            }
            xSemaphoreGive(logMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void deinitLogger(void)
{
    if (logFile != NULL) {
        fclose(logFile);
        logFile = NULL;
    }
    
    if (logMutex != NULL) {
        vSemaphoreDelete(logMutex);
        logMutex = NULL;
    }
}