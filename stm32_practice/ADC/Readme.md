# ARM Cortex-M4 Temperature Monitoring System (Simulation Target)

A low-level, bare-metal C implementation designed to monitor core temperatures by leveraging hardware peripheral registers on an **ARM Cortex-M4 microcontroller simulator**. 

> [!IMPORTANT]  
> **Simulation Status:** This firmware is specifically configured and optimized for an **ARMCM4 Keil/QEMU simulation environment**. It utilizes direct memory-mapped register configurations to validate timing, interrupt handling, and ADC data conversion logic without requiring physical hardware.

---

## 🛠️ System Architecture & Simulation Workflow

The project simulates a real-time thermal safety monitor using three interconnected peripheral structures:

1. **Timer 2 (TIM2):** Configured as a periodic timebase. It generates an update interrupt every **500ms** (assuming a simulated 84MHz clock source).
2. **ADC1 (Channel 1 / Pin PA1):** Triggered by the Timer 2 interrupt event. It executes a single 12-bit analog-to-digital conversion on the temperature sensor channel.
3. **Core Monitoring Loop:** Processes the raw data, applies voltage-to-temperature translation math, and updates safety flags.

