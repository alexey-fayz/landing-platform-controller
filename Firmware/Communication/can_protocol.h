#ifndef CAN_PROTOCOL_H
#define CAN_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

typedef enum
{
    CAN_CMD_NOP = 0,

    CAN_CMD_ENABLE_MOTOR = 1,

    CAN_CMD_DISABLE_MOTOR = 2,

    CAN_CMD_SET_POSITION = 3,

    CAN_CMD_SET_VELOCITY = 4,

    CAN_CMD_REQUEST_STATUS = 5

} CAN_Command_t;

typedef struct
{
    uint8_t command;

    float value;

} CAN_CommandFrame_t;

void CAN_Protocol_Init(void);

void CAN_Protocol_Process(void);

HAL_StatusTypeDef CAN_SendStatus(
        float position,
        float velocity,
        float current);

HAL_StatusTypeDef CAN_SendHeartbeat(void);

#ifdef __cplusplus
}
#endif

#endif