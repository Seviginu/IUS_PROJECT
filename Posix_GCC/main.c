#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "logger.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>

void vAssertCalled(const char *pcFile, unsigned long ulLine) {
    printf("ASSERT FAILED: %s:%lu\n", pcFile, ulLine);
    for(;;);
}

// Дескрипторы задач и очередей
static TaskHandle_t producerTaskHandle = NULL;
static TaskHandle_t consumerTaskHandle = NULL;
static QueueHandle_t queueHandle = NULL;

// Прототипы задач
void producerTask(void *pvParameters);
void consumerTask(void *pvParameters);
void idleTask(void *pvParameters);

int main(void) {
    // Инициализация логгера
    initLogger("freertos_log.txt");
    printf("FreeRTOS Kernel Test Application\n");

    // Создаем очередь
    queueHandle = xQueueCreate(5, sizeof(uint32_t));
    if (queueHandle == NULL) {
        printf("Queue creation failed!\n");
        for(;;);
    }

    // Создаем задачи с проверкой ошибок
    if (xTaskCreate(producerTask, "Producer", configMINIMAL_STACK_SIZE * 2, NULL, 2, &producerTaskHandle) != pdPASS) {
        printf("Producer task creation failed!\n");
        for(;;);
    }

    if (xTaskCreate(consumerTask, "Consumer", configMINIMAL_STACK_SIZE * 2, NULL, 2, &consumerTaskHandle) != pdPASS) {
        printf("Consumer task creation failed!\n");
        for(;;);
    }

    if (xTaskCreate(consumerTask, "Consumer 2", configMINIMAL_STACK_SIZE * 2, NULL, 2, &consumerTaskHandle) != pdPASS) {
        printf("Consumer task creation failed!\n");
        for(;;);
    }

    int i = 100;
    while (i-- > 0) {
        if (xTaskCreate(idleTask, "Idle", configMINIMAL_STACK_SIZE, NULL, 0, NULL) != pdPASS) {
            printf("Idle task creation failed!\n");
            for(;;);
        }
    }

    // Запускаем планировщик
    vTaskStartScheduler();

    // Сюда не должны попасть
    for(;;);
    return 0;
}

void producerTask(void *pvParameters) {
    uint32_t value = 0;
    const TickType_t delay = pdMS_TO_TICKS(500);

    for(;;) {
        if (xQueueSend(queueHandle, &value, portMAX_DELAY) == pdPASS) {
            printf("Produced: %lu\n", value);
            value++;
        }

        vTaskDelay(delay);

        if (value == 5) {
            printf("Producer suspending consumer\n");
            vTaskSuspend(consumerTaskHandle);
        }

        if (value == 10) {
            printf("Producer resuming consumer\n");
            vTaskResume(consumerTaskHandle);
        }
    }
}

void consumerTask(void *pvParameters) {
    uint32_t receivedValue;
    const TickType_t delay = pdMS_TO_TICKS(700);

    for(;;) {
        if (xQueueReceive(queueHandle, &receivedValue, portMAX_DELAY) == pdPASS) {
            printf("Consumed: %lu\n", receivedValue);
        }

        vTaskDelay(delay);
    }
}

// Idle Task для low-power кода
void idleTask(void *pvParameters) {
    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Обязательные обработчики для FreeRTOS Kernel
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    printf("STACK OVERFLOW in task %s\n", pcTaskName);
    for(;;);
}

void vApplicationMallocFailedHook(void) {
    printf("HEAP IS FULL - malloc failed\n");
    for(;;);
}

void vApplicationIdleHook(void) {
    // Опционально: код для выполнения в idle
}


// Память под Idle Task
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
    *ppxIdleTaskStackBuffer = xIdleStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

// Память под Timer Task
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
    *ppxTimerTaskStackBuffer = xTimerStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void vApplicationTickHook(void)
{
    // Можно оставить пустой
}

void vApplicationDaemonTaskStartupHook(void)
{
    // Можно оставить пустой
}

uint32_t uiTraceTimerGetValue(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000000 + ts.tv_nsec / 1000); // микросекунды
}

uint32_t uiTraceTimerGetFrequency(void)
{
    return 1000000; // герц (микросекунды)
}

void vTraceTimerReset(void)
{
    // Ничего не делаем, если не требуется сброс
}