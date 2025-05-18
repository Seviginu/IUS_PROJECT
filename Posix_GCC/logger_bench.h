#ifndef LOGGER_TEST_TASK_H
#define LOGGER_TEST_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>


/**
 * @brief Тестовая задача для демонстрации работы логгера и различных состояний задач
 * 
 * Задача демонстрирует:
 * - Нормальное выполнение
 * - Блокировку на очереди
 * - Блокировку на задержке
 * - Разблокировку
 * - Создание и удаление задач
 */
void vLoggerTestTask(void *pvParameters);

/**
 * @brief Временная задача для тестирования создания/удаления задач
 * 
 * @param pvParameters Параметр, передаваемый при создании задачи
 */
void vTempTask(void *pvParameters);

#endif /* LOGGER_TEST_TASK_H */