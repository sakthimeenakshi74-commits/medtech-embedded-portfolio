# STM32 Medtech UART Driver

Production-grade UART communication driver for medical devices.
Built on STM32F401, validated in Renode simulation.

## Features
- DMA + IDLE line interrupt — zero CPU overhead during reception
- CRC-8 packet validation (polynomial 0x07)
- Multi-sensor support via SENSOR_ID routing
- Ring buffer (256 bytes) with overflow protection
- IEC 62304 §5.5 compliant software unit documentation
- ISO 14971 risk controls implemented

## Architecture
Application → Protocol → Driver → Hardware (4-layer)

## Simulation
Tested with Renode virtual STM32F401
Python multi-sensor simulator included

## Standards
- IEC 62304 Class B software unit
- ISO 14971 risk management controls
- CRC-8 per AUTOSAR specification

## Target Hardware
STM32F401CCU6 (Black Pill)
USART1 — PA9 TX, PA10 RX
DMA2 Stream 5, Channel 4
