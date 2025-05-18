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

void initLogger() {
    logMutex = xSemaphoreCreateMutex();
    if (logMutex == NULL) {
        printf("Failed to create log mutex!\n");
        configASSERT(0); // Остановить систему если мьютекс не создан
    }
    
    // Создаем задачу логгера
    xTaskCreate(logWriterTask, "Logger", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL);
    printf("Logger INIT OK\n");
}

static void logWrite(const char *data, size_t len) {
    if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        for (size_t i = 0; i < len; i++) {
            logBuffer[head] = data[i];
            head = (head + 1) % LOG_BUFFER_SIZE;
            if (head == tail) {
                tail = (tail + 1) % LOG_BUFFER_SIZE;
            }
        }
        xSemaphoreGive(logMutex);
    }
}

void logState(TaskHandle_t task, const char *state) {
    TickType_t timestamp = xTaskGetTickCount();
    uint32_t id = (uint32_t)task;
    char logLine[64];
    
    int len = snprintf(logLine, sizeof(logLine), "%lu,%u,%s\n", 
                      pdTICKS_TO_MS(timestamp), id, state);
    if (len > 0) {
        logWrite(logLine, len);
    }
}

void vApplicationTaskSwitchedIn(void) {
    logState(xTaskGetCurrentTaskHandle(), "Running");
}

void vApplicationMallocFailedHook(void) {
    const char* error = "Malloc Failed!\n";
    logWrite(error, strlen(error));
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    char logLine[64];
    int len = snprintf(logLine, sizeof(logLine), "Stack overflow in %s!\n", pcTaskName);
    if (len > 0) logWrite(logLine, len);
}

void logWriterTask(void *pvParams) {
    (void)pvParams;
    char line[128];
    size_t idx = 0;

    while (1) {
        if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            while (tail != head && idx < sizeof(line) - 1) {
                line[idx++] = logBuffer[tail];
                tail = (tail + 1) % LOG_BUFFER_SIZE;
                
                if (line[idx - 1] == '\n') {
                    line[idx] = '\0';
                    printf("%s", line);
                    fflush(stdout); // Важно для немедленного вывода
                    idx = 0;
                }
            }
            xSemaphoreGive(logMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}