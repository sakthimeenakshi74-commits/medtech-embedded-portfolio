# UART Communication Software Unit
## IEC 62304 §5.5 Software Unit Implementation

### Unit Identification
- Unit Name: UART_MEDTECH_DRIVER
- Version: 1.0.0
- Device Class: Class B
- Risk Level: Medium

### Software Unit Requirements
- REQ-UART-001: Driver shall receive UART packets at 9600 baud, 8N1
- REQ-UART-002: All packets shall be validated using CRC-8 (poly 0x07)
- REQ-UART-003: Corrupted packets shall be discarded and logged
- REQ-UART-004: Buffer overflow shall raise COMM_OVERFLOW alarm
- REQ-UART-005: Communication failure after 3 retries shall raise
                COMM_LOST alarm
- REQ-UART-006: Driver shall support minimum 3 concurrent sensor types

### Verification Evidence
- REQ-UART-001: Verified via Renode simulation at 9600 baud
- REQ-UART-002: CRC error injection tested — corrupted packets discarded
- REQ-UART-003: Error log transmitted via UART on each discard
- REQ-UART-004: Ring buffer overflow tested with SIZE=10, 300 bytes sent
- REQ-UART-005: Python simulator timeout tested — alarm raised after 3s
- REQ-UART-006: Three sensor types verified in simulation

### Risk Controls (ISO 14971)
- RISK-001: Data corruption → CRC-8 validation → packet discard
- RISK-002: Buffer overflow → reject-newest policy → COMM_OVERFLOW alarm
- RISK-003: Sensor silence → timeout detection → COMM_LOST alarm
- RISK-004: Bus collision → time-division multiplexing in simulator
