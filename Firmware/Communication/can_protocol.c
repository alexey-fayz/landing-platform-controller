#include "communication/can_protocol.h"

#include "control/motor_controller.h"

#include <string.h>

extern CAN_HandleTypeDef hcan1;

static CAN_RxHeaderTypeDef rxHeader;
static uint8_t rxData[8];

static CAN_TxHeaderTypeDef txHeader;

void CAN_Protocol_Init(void)
{
    txHeader.StdId = 0x101;

    txHeader.ExtId = 0;

    txHeader.IDE = CAN_ID_STD;

    txHeader.RTR = CAN_RTR_DATA;

    txHeader.DLC = 8;

    txHeader.TransmitGlobalTime = DISABLE;
}

void CAN_Protocol_Process(void)
{
    while(HAL_CAN_GetRxFifoFillLevel(
            &hcan1,
            CAN_RX_FIFO0))
    {
        HAL_CAN_GetRxMessage(
                &hcan1,
                CAN_RX_FIFO0,
                &rxHeader,
                rxData);

        uint8_t cmd =
                rxData[0];

        float value;

        memcpy(
            &value,
            &rxData[1],
            sizeof(float));

        switch(cmd)
        {
            case CAN_CMD_ENABLE_MOTOR:

                MotorController_Enable();

                break;

            case CAN_CMD_DISABLE_MOTOR:

                MotorController_Disable();

                break;

            case CAN_CMD_SET_POSITION:

                MotorController_SetPosition(
                        value);

                break;

            default:
                break;
        }
    }
}

HAL_StatusTypeDef CAN_SendHeartbeat(void)
{
    uint32_t mailbox;

    uint8_t data[8] =
    {
        0xAA,
        0x55,
        0,
        0,
        0,
        0,
        0,
        0
    };

    return HAL_CAN_AddTxMessage(
                &hcan1,
                &txHeader,
                data,
                &mailbox);
}

HAL_StatusTypeDef CAN_SendStatus(
        float position,
        float velocity,
        float current)
{
    uint32_t mailbox;

    uint8_t data[8];

    int16_t pos =
            (int16_t)(position * 10.0f);

    int16_t vel =
            (int16_t)(velocity * 10.0f);

    int16_t cur =
            (int16_t)(current * 100.0f);

    memcpy(&data[0], &pos, 2);
    memcpy(&data[2], &vel, 2);
    memcpy(&data[4], &cur, 2);

    data[6] = 0;
    data[7] = 0;

    return HAL_CAN_AddTxMessage(
                &hcan1,
                &txHeader,
                data,
                &mailbox);
}