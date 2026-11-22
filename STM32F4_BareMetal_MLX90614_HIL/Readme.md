# Bare-Metal STM32F401 MLX90614 Non-Contact IR Temperature Monitor (Renode HIL)

A register-level, bare-metal embedded C driver suite for the **STM32F401 (ARM Cortex-M4)** interfacing with a medical-grade **Melexis MLX90614 infrared thermometer** over I2C/SMBus, validated end-to-end via **Renode Hardware-in-the-Loop (HIL) simulation**.

Designed for clinical and biomedical temperature-sensing applications, this firmware avoids high-level HAL peripheral abstractions in favor of direct **CMSIS register manipulation** for deterministic timing, minimal memory footprint, and full hardware transparency.

---

## System Architecture

```text
+----------------------------------+         +----------------------------------+

|     MLX90614 IR Sensor (0x5A)    |         |     Renode UART Analyzer         |
|  - RAM 0x07: Object Temp (TOBJ1) |         |  - Live Serial Telemetry Output  |
|  - RAM 0x08: Ambient Temp (TOBJ2)|         +----------------▲-----------------+
+----------------┬-----------------+                          │
                 │ I2C1 (PB6: SCL, PB7: SDA @ 100 kHz)        │ USART2 (PA2: TX @ 115200 bps)
                 ▼                                            │
+-------------------------------------------------------------------------------+

|                       STM32F401CCU6 (ARM Cortex-M4 @ 84 MHz)                  |
|                                                                               |
|  [TIM2 Hardware Interrupt] ──(1 Hz Flag)──> [Bare-Metal I2C Master State      |
|   (TIM2_IRQHandler + NVIC)                   Machine & Kelvin->Celsius Math]  |
+-------------------------------------------------------------------------------+
```

---

## Bare-Metal Firmware Highlights

* **Clock & GPIO Configuration (`RCC`, `GPIOA`, `GPIOB`):**
  * Configured system and peripheral clock gating via `RCC->AHB1ENR` and `RCC->APB1ENR`.
  * Mapped `PB6` (SCL) and `PB7` (SDA) to Alternate Function **`AF4`** (`GPIOB->AFR`) with open-drain output (`OTYPER`), high slew rate (`OSPEEDR`), and internal pull-ups (`PUPDR`).
  * Mapped `PA2` (TX) and `PA3` (RX) to Alternate Function **`AF7`** (`GPIOA->AFR`) for `USART2`.
* **Interrupt-Driven Scheduling (`TIM2` + `NVIC`):**
  * Configured `TIM2` prescaler (`PSC`) and auto-reload (`ARR`) registers with update interrupts (`TIM2->DIER`) routed through `NVIC_EnableIRQ(TIM2_IRQn)` to trigger non-blocking periodic sampling without `HAL_Delay()`.
* **Custom I2C Master Driver (`I2C1->CR1`, `CR2`, `CCR`, `TRISE`, `SR1`, `SR2`, `DR`):**
  * Implemented the complete SMBus/I2C read sequence from scratch: `START` condition, 7-bit slave addressing (`0x5A` write), register pointer transmission (`0x07` / `0x08`), `Repeated START`, slave read (`0x5A` read), 16-bit little-endian data reception (`RxNE`), PEC checksum byte handling, and `STOP` generation.
* **Thermal Conversion & UART Telemetry (`USART2`):**
  * Converted raw 16-bit sensor words (0.02 K/LSB) to degrees Celsius (\(T_{^\circ\text{C}} = \text{Raw} \times 0.02 - 273.15\)) and streamed formatted frames via a `TXE`-polled `USART2` driver.

---

## Repository Structure

```text
STM32F4_BareMetal_MLX90614_HIL/
├── Core/
│   ├── Inc/                # CMSIS & peripheral header files (main.h, stm32f4xx_it.h)
│   └── Src/                # Bare-metal C drivers (main.c, stm32f4xx_it.c)
├── stm32f401_mlx.repl      # Custom Renode platform description (STM32F4 + I2C slave @ 0x5A)
├── run_sim.resc            # Automated Renode HIL script with simulated thermal profiles
└── README.md               # Project documentation



   Renode automatically opens the `sysbus.usart2` analyzer window and streams real-time object and ambient temperature readings directly from the virtual Cortex-M4 CPU.
