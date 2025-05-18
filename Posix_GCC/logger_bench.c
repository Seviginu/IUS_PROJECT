#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "logger.h"
#include <stdio.h>

/* Тестовая очередь для демонстрации блокировки */
static QueueHandle_t xTestQueue = NULL;

/* Тестовая задача-имитатор */
void vLoggerTestTask(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xDelay = pdMS_TO_TICKS(1000);
    uint32_t ulCounter = 0;
    
    (void)pvParameters;
    
    /* Создаем очередь для теста блокировки */
    xTestQueue = xQueueCreate(1, sizeof(uint32_t));
    
    xLastWakeTime = xTaskGetTickCount();
    
    for(;;)
    {
        /* 1. Простое выполнение (лог "Running") */
        printf("[TestTask] Normal execution, counter: %u\n", ulCounter++);
        
        /* 2. Блокировка на очереди (лог "Blocked") */
        if(xQueueSend(xTestQueue, &ulCounter, xDelay) != pdPASS)
        {
            printf("[TestTask] Failed to send to queue!\n");
        }
        
        /* 3. Искусственная задержка (лог "Blocked" на vTaskDelay) */
        vTaskDelayUntil(&xLastWakeTime, xDelay);
        
        /* 4. Попытка получения из очереди (лог "Unblocked" при успехе) */
        uint32_t xReceivedValue;
        if(xQueueReceive(xTestQueue, &xReceivedValue, 0) == pdPASS)
        {
            printf("[TestTask] Received: %u\n", xReceivedValue);
        }
        
        /* 5. Создание/удаление дополнительной задачи */
        if((ulCounter % 3) == 0)
        {
            TaskHandle_t xTempTask;
            xTaskCreate(xTempTask, "TempTask", configMINIMAL_STACK_SIZE, 
                       (void*)ulCounter, tskIDLE_PRIORITY, &xTempTask);
            vTaskDelay(pdMS_TO_TICKS(10));
            vTaskDelete(xTempTask); // Лог "Deleted"
        }
    }
}

/* Временная задача для теста создания/удаления */
void vTempTask(void *pvParameters)
{
    printf("[TempTask] Created with param: %lu\n", (uint32_t)pvParameters);
    vTaskDelete(NULL); // Самоудаление
}
