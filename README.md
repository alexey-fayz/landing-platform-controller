# Active Stabilization Platform Firmware

## Overview

This project implements an embedded firmware for an active stabilization platform that compensates for marine vessel motion (roll and pitch), enabling a quadcopter to land safely on a moving boat or buoy.

The system uses a gimbal-like structure with two BLDC servos, a cascade PID controller, absolute encoder feedback, Hall sensors for commutation, current monitoring, and CAN/UART communication.  
FreeRTOS is used for real-time task management.

---

## Table of Contents

- Features  
- Prerequisites   
- Architecture  
- Communication Interfaces  
- Safety & Monitoring  

---

## Features

- **Two-axis stabilization (roll & pitch)**  
  Keeps the landing platform level despite vessel motion.

- **6-step BLDC commutation**  
  Hall sensors provide rotor position for efficient motor control.

- **Absolute encoder feedback**  
  AS5311 (12-bit, SSI interface) provides precise angular position.

- **Cascade PID control**  
  Inner speed loop + outer position loop for smooth and accurate control.

- **Current limiting & monitoring**  
  INA219 (I²C) provides over-current protection and adaptive power reduction.

- **Real-time operating system (FreeRTOS)**  
  Dedicated tasks for control, communication, telemetry, and diagnostics.

- **CAN bus communication (250 kbit/s)**  
  Receives target angles and transmits telemetry data.

- **UART debug console (115200 baud)**  
  Simple command interface for manual control and debugging.

- **Modular architecture**  
  Clear separation of BSP, control, communication, and utilities.

---

## Prerequisites

### Hardware

- STM32L451CET6 microcontroller  
- BLDC motors with Hall sensors  
- DRV8353 gate driver  
- AS5311 absolute encoder  
- INA219 current sensor  
- CAN transceiver (e.g., ISO1050)  
- 24 V / 5 V / 3.3 V power supplies  

### Software

- STM32CubeMX (project generation)  
- STM32CubeIDE / Keil / IAR / ARM GCC toolchain  
- FreeRTOS (included via CubeMX)  
- UART terminal / CAN analyzer (optional)  

---

## Architecture

The firmware is organized into logical layers:

- **App/** – system initialization, global state, interrupt callbacks
- **BSP/** – low-level hardware drivers (encoder, Hall sensors, DRV8353, INA219)
- **Control/** – PID controllers, filters, estimators, trajectory generation
- **Communication/** – CAN protocol, UART telemetry, command parser
- **Utils/** – ring buffers, math utilities
- **Tasks/** – FreeRTOS task implementations

All hardware-specific code is isolated in BSP, ensuring that control logic remains portable and testable.

---

## Communication Interfaces

### UART Console (115200 baud, 8N1)

Connection:
- PA2 → TX  
- PA3 → RX  

#### Commands

| Command      | Action                        |
|--------------|-------------------------------|
| `enable`     | Enable motor control          |
| `disable`    | Disable motor control         |
| `angle 12.3` | Set target angle (degrees)    |
| `status`     | Show system state             |


---

### CAN Bus (250 kbit/s, standard ID)

#### Receive (ID: 0x100)

Target angle command:

- Data length: 2 bytes  
- Format:
  - `angle (deg) = (byte0 << 8 | byte1) / 10.0`

---

#### Transmit (ID: 0x101)

Telemetry frame (6 bytes):

- `uint16` current angle ×10  
- `int16` current speed ×10  
- `uint8` enable flag  
- `uint8` error flags  

This protocol is compatible with marine-style CAN networks (similar to NMEA2000-like systems).

---

## Safety & Monitoring

### Over-current protection
The INA219 sensor continuously measures motor current.  
If the current exceeds a defined threshold:
- Motor power is reduced gradually, or
- The system is shut down if the condition persists

---

### Fault detection
The DRV8353 gate driver provides an `nFAULT` signal.  
On fault detection:
- PWM outputs are disabled immediately
- System enters safe state

---

### FreeRTOS safety mechanisms
The system uses built-in RTOS safety features:
- Stack overflow detection hooks
- Runtime error callbacks
- Task watchdog logic (if enabled)

---

### Emergency stop behavior
The system can be disabled via:
- UART `disable` command
- CAN stop command

In all cases:
- Motor outputs are immediately disabled
- Control loops are paused
- System transitions to safe idle state

