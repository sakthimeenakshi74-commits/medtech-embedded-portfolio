### Multi-Task Patient Vital Signs Monitor (FreeRTOS Simulation)

An **IEC 62304-inspired** real-time patient monitoring system simulation built on **FreeRTOS**. Designed to showcase multi-tasking, thread-safe communication pipelines, defensive programming and hardware-independent fault handling tailored for **MedTech embedded systems**. 

### 🚀 Key MedTech Focus Areas Demonstrated

* **IEC 62304 Standards Integration:** Implements core software safety lifecycle paradigms, focusing on software fault categorization, data validation and deterministic safe-recovery states.
* **Defensive Firmware Architectural Patterns:** Uses bounded mutex timeouts to prevent deadlocks, strict input boundary checks and a prioritized software watchdog loop.
* **Advanced FreeRTOS Inter-Task Communication (ITC):** Demonstrates high-performance, low-overhead primitives (Stream Buffers, Event Groups and Task Notifications).

### 🏗️ System Architecture

┌──────────────────────────────────────────────────────────┐
│  Sensor Tasks: HR(P4), SpO2(P4), BP(P3), Temp(P2)       │
│       │                │                                 │
│  Stream Buffer    Event Group         Task Notification  │
│  (telemetry log)  (abnormal detect)   (UI alert)        │
│       │                │                    │            │
│  Logging Task    Critical Alert Task   Display Task     │
│  (P1)            (P5)                  (P1)             │
│                                                          │
│  Watchdog Task (P6) ──► Safe Shutdown on failure        │
│  Health Monitor (P1) ──► System diagnostics every 10s  │
└──────────────────────────────────────────────────────────┘

### Task Priority Hierarchy (Deterministic Design)

1. **Priority 6 (Highest):** Watchdog Task — Critical liveness supervisor.
2. **Priority 5:** Critical Alert Task — High-priority immediate alert aggregator.
3. **Priority 4:** HeartRate & SpO2 Sensor Tasks — High-frequency biological telemetry.
4. **Priority 3:** Blood Pressure Sensor Task — Medium-frequency telemetry.
5. **Priority 2:** Temperature Sensor Task — Low-frequency telemetry.
6. **Priority 1 (Lowest):** Logging, Display, and Health Monitor — Non-blocking UI/Diagnostic elements.

### 🛠️ FreeRTOS Features Utilized

* **Multi-Producer Single-Consumer (MPSC) Logging Pipeline:** Shared Stream Buffers guarded by a Mutex serialize concurrent telemetry data from four distinct sensors into a dedicated asynchronous logging core.
* **Multi-Vital Alert Aggregation:** An Event Group acts as an OR-wait condition barrier (xEventGroupWaitBits), allowing a single high-priority task to handle multi-threshold safety violations instantly.
* **Zero-Overhead Lightweight Signaling:** Direct Task Notifications (xTaskNotify with eSetBits) pass discrete events to the UI layer with 45% faster execution speeds and zero RAM allocation compared to traditional semaphores.

### 🛡️ Medical-Grade Safety & Reliability Features

* **Software Watchdog Tracking:** A dedicated monitor ensures system liveness using an atomic bitmask (ulWatchdogKickRegister). If any sensor thread stalls for >5s, the system gracefully triggers an error log and enters a controlled safe-halt state.
* **Strict Runtime Object Validation:** Instantiates a strict NULL/pdPASS confirmation loop across all core kernel objects before scheduler execution (vTaskStartScheduler) to prevent runtime memory faults.
* **Physical Boundary Filtering:** Employs explicit input validation arrays on simulated sensor feeds (e.g., rejecting SpO2 readings <70% as sensor disconnect anomalies) before dispatching to data streams.
* **Bounded Block Windows:** Mutex locks are strictly configured with maximum wait thresholds (100ms) instead of infinite blocking (portMAX_DELAY) to isolate and trace potential priority inversions or deadlocks.

Output:

sakthi@Sakthi:~/PatientMonitor$ ./PatientMonitor
Heart Rate: 103 BPM
[CRITICAL] Heart Rate abnormal!
SpO2: 98%
Blood Pressure: 97 mmHg
Temperature: 98.5 F
[Notify] Heart Rate needs attention
Heart Rate: 113 BPM
[CRITICAL] Heart Rate abnormal!
[Notify] Heart Rate needs attention
Heart Rate: 105 BPM
[CRITICAL] Heart Rate abnormal!
Blood Pressure: 106 mmHg
Temperature: 97.2 F
[Notify] Heart Rate needs attention
Heart Rate: 119 BPM
[CRITICAL] Heart Rate abnormal!
[Notify] Heart Rate needs attention
SpO2: 95%
[CRITICAL] SpO2 abnormal!
Heart Rate: 82 BPM
Blood Pressure: 107 mmHg
Temperature: 98.0 F
[Notify] SpO2 needs attention
Heart Rate: 79 BPM
Heart Rate: 83 BPM
Blood Pressure: 86 mmHg
Temperature: 96.0 F
Heart Rate: 96 BPM
SpO2: 98%
Heart Rate: 106 BPM
[CRITICAL] Heart Rate abnormal!
Blood Pressure: 91 mmHg
Temperature: 96.8 F
[Notify] Heart Rate needs attention
Heart Rate: 87 BPM
[Watchdog] All tasks alive - system nominal
Heart Rate: 99 BPM
Blood Pressure: 102 mmHg
Temperature: 98.0 F
Heart Rate: 82 BPM
SpO2: 95%
[CRITICAL] SpO2 abnormal!
[Notify] SpO2 needs attention
Heart Rate: 87 BPM
Blood Pressure: 95 mmHg
