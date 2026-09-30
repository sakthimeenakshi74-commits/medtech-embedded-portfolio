# Real-Time FreeRTOS Ventilator Simulator

A bare-metal concurrent application designed to simulate life-critical software scheduling loops on a FreeRTOS platform.

## 🛠️ Architecture Details
* **Watchdog Task (Priority 6):** Safety checkpoint supervising execution frames every 3.5 seconds.
* **Alarm Manager Task (Priority 5):** Dispatches asynchronous panic flags and complete records.
* **Pressure Monitor Task (Priority 4):** Evaluates airway sensor data parameters every 100ms.
* **Breathing Cycle Task (Priority 3):** Drives physiological inspiration/expiration metrics at exactly **20 BPM**.

## 🚦 Features Checked
* Thread safe console access via a designated Print Mutex.
* Isolated shared resource data access using a unique telemetry wrapper handle.
