#ifndef LOGGER_H
#define LOGGER_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/**
 * @brief Initialize the logger system
 */
void initLogger(void);

/**
 * @brief Log task state change
 * @param task Handle of the task
 * @param state New state as string ("Ready", "Running", "Blocked", "Deleted")
 */
void logState(TaskHandle_t task, const char *state);

/**
 * @brief Handler for task switch event
 */
void vTaskSwitchedIn(void);

/**
 * @brief Handler for task blocking event
 * @param queue Queue that caused the block (unused in current implementation)
 */
void vTaskBlocking(void *queue);

/**
 * @brief Handler for task unblocking event
 * @param task Task being unblocked (current task if NULL)
 */
void vTaskUnblocked(void *task);

/**
 * @brief Handler for task creation event
 * @param task Newly created task
 */
void vTaskCreated(TaskHandle_t task);

/**
 * @brief Handler for task deletion event
 * @param task Task being deleted
 */
void vTaskDeleted(TaskHandle_t task);

/**
 * @brief Task function that writes logs to console
 * @param pvParams Task parameters (unused)
 */
void logWriterTask(void *pvParams);

#endif // LOGGER_H