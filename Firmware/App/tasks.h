/**
  * @file    tasks.h
  * @brief   Заголовочный файл для пользовательских задач FreeRTOS.
  */

#ifndef TASKS_H
#define TASKS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Прототипы задач (объявлены в main.c, определены здесь) */
void StartDefaultTask(void *argument);
void StartEncoderTask(void *argument);
void StartHallTask(void *argument);
void StartControlTask(void *argument);
void StartCanTask(void *argument);
void StartTelemetryTask(void *argument);
void StartDiagnosticsTask(void *argument);

/* Опциональная инициализация задач */
void Tasks_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* TASKS_H */