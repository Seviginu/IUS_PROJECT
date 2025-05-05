#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include "console.h"
#include "logger.h"

// Circular buffer for logs
#define LOG_BUFFER_SIZE 1024
static char logBuffer[LOG_BUFFER_SIZE];
static size_t head = 0;
static size_t tail = 0;
static SemaphoreHandle_t logMutex;

// Logger init
void initLogger() {
    logMutex = xSemaphoreCreateMutex();
}

// Write to buffer
static void logWrite(const char *data, size_t len) {
    xSemaphoreTake(logMutex, portMAX_DELAY);
    for (size_t i = 0; i < len; i++) {
        logBuffer[head] = data[i];
        head = (head + 1) % LOG_BUFFER_SIZE;
        if (head == tail) tail = (tail + 1) % LOG_BUFFER_SIZE; // Rewrite in case of overflow
    }
    xSemaphoreGive(logMutex);
}

// Log entry
void logState(TaskHandle_t task, const char *state) {
    TickType_t timestamp = xTaskGetTickCount();
    uint32_t id = (uint32_t)task; // Use task address as ID
    char logLine[64];
    int len = snprintf(logLine, sizeof(logLine), "%lu,%u,%s\n", 
                      pdTICKS_TO_MS(timestamp), id, state);
    if (len > 0) logWrite(logLine, len);
}

// Task Event Handler
void vTaskSwitchedIn() {
    logState(xTaskGetCurrentTaskHandle(), "Running");
}

void vTaskBlocking(void *queue) {
    (void)queue; // Unused yet
    logState(xTaskGetCurrentTaskHandle(), "Blocked");
}

void vTaskUnblocked(void *task) {
    TaskHandle_t t = task ? (TaskHandle_t)task : xTaskGetCurrentTaskHandle();
    logState(t, "Ready");
}

void vTaskCreated(TaskHandle_t task) {
    logState(task, "Ready");
}

void vTaskDeleted(TaskHandle_t task) {
    logState(task, "Deleted");
}

// Temporary function to write in console
void logWriterTask(void *pvParams) {
    (void)pvParams;
    char line[128];
    size_t idx = 0;

    while (1) {
        xSemaphoreTake(logMutex, portMAX_DELAY);
        while (tail != head && idx < sizeof(line) - 1) {
            line[idx++] = logBuffer[tail];
            tail = (tail + 1) % LOG_BUFFER_SIZE;
            if (line[idx - 1] == '\n') {
                line[idx] = '\0';
                console_print("%s", line);
                idx = 0;
            }
        }
        xSemaphoreGive(logMutex);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}