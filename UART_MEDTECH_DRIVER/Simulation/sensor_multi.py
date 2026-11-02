import socket
import time

def xor_checksum(data):
    chk = 0
    for b in data:
        chk ^= b
    return chk

def send_spo2_bpm(sock, bpm, spo2):
    data = [0x01, bpm, spo2]          # SENSOR_ID + data
    chk = xor_checksum(data)
    packet = bytes([0xAA, len(data), *data, chk, 0x55])
    sock.send(packet)
    print(f"[SPO2_BPM] BPM:{bpm} SpO2:{spo2}")

def send_temperature(sock, temp):
    data = [0x99, temp]               # SENSOR_ID + data
    chk = xor_checksum(data)
    packet = bytes([0xAA, len(data), *data, chk, 0x55])
    sock.send(packet)
    print(f"[TEMP] Temp:{temp}C")

def send_accel(sock, x, y, z):
    data = [0x03, x, y, z]            # SENSOR_ID + data
    chk = xor_checksum(data)
    packet = bytes([0xAA, len(data), *data, chk, 0x55])
    sock.send(packet)
    print(f"[ACCEL] X:{x} Y:{y} Z:{z}")

# Connect to Renode
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('localhost', 4321))
print("Multi-sensor simulator connected")

bpm = 60
spo2 = 95
temp = 36
x, y, z = 0, 0, 9

while True:
    send_spo2_bpm(sock, bpm, spo2)
    time.sleep(0.5)

    send_temperature(sock, temp)
    time.sleep(0.5)

    send_accel(sock, x, y, z)
    time.sleep(0.5)

    # Vary values
    bpm = bpm + 1 if bpm < 100 else 60
    spo2 = spo2 + 1 if spo2 < 99 else 95
    temp = temp + 1 if temp < 39 else 36
    x = x + 1 if x < 20 else 0
