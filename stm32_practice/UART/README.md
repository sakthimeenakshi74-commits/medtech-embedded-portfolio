# Bare-Metal STM32F4 UART & Timer Practice Project

A low-level **bare-metal C learning project** for the **STM32F407VG (Discovery Board)** microcontroller. The objective of this project is to practice direct hardware register manipulation, basic timer configurations, and interrupt-driven serial communications without relying on HAL drivers.

## 🎯 Learning & Practice Objectives

This project targets the following embedded firmware concepts:
*   **Bitwise Configuration:** Utilizing raw bit-shifts (`|=` and `&= ~`) to enable clock buses, toggle operational modes, and set up Alternate Function routing.
*   **Interrupt Handling:** Writing manual ISR routines (`TIM2_IRQHandler` and `USART2_IRQHandler`) and initializing them inside the Core NVIC matrix.

## ⚙️ How the Practice Logic Works

1. **Periodic Loop (TIM2):** Timer 2 is configured with a prescaler and auto-reload limit to fire an update flag exactly every 1 second. When the flag triggers, the `main` loop prints out a mock data string to demonstrate periodic UART transmission.
2. **Interrupt Command Processing (USART2):** The code listens for incoming data bytes asynchronously. Pushing commands into the console triggers actions:
    *   `S` → Responds with a generic start acknowledgement string.
    *   `P` → Responds with a pause acknowledgement string.
    *   `R` → Responds with a reset acknowledgement string.
