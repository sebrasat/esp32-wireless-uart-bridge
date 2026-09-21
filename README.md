# ESP32 Wireless UART Bridge

A high-performance, transparent bidirectional Wi-Fi to UART bridge for embedded systems and satellite payloads. Supports both WPA2-Enterprise (university/corporate networks such as eduroam) and standalone Access Point (AP) fallback.

---

## Features

- **Bidirectional Transparent Stream:** Full-duplex forwarding between Hardware UART and TCP socket.
- **Enterprise Network Support:** Native 802.1X PEAP/MSCHAPv2 authentication (eduroam compatible).
- **Automatic Fallback AP:** Deploys a standalone local Wi-Fi Access Point if enterprise Wi-Fi is unreachable.
- **Standard 3.3V LVCMOS Logic:** Direct electrical compatibility with microcontrollers, FPGAs, and embedded SBCs.

---

## Hardware Pinout & Wiring

| ESP32 Pin | Target Device Pin | Function |
|:---:|:---:|:---|
| **GND** | Target GND | Common Signal Ground (Mandatory) |
| **GPIO 16 (RX2)** | Target TX | ESP32 Receiver (Listens to target) |
| **GPIO 17 (TX2)** | Target RX | ESP32 Transmitter (Drives target) |

*Note: Power the ESP32 via its micro-USB connector. Do not power from external 3.3V logic rails due to Wi-Fi transmission current spikes.*

---

## Configuration

Copy the example credentials header and insert your network credentials:

```bash
cp credentials.h.example credentials.h
```

Edit `credentials.h` to configure eduroam or standalone Access Point mode. This file is ignored by Git to prevent leaking private credentials.

---

## Workstation Usage

### 1. Direct Terminal Access (Raw Stream)
Connect directly to the ESP32 IP address on port 8888 using Netcat or Telnet:

```bash
nc <ESP32_IP> 8888
```

### 2. Virtual Serial Port over Network (socat)
Create a virtual PTY loopback on Linux or macOS:

```bash
socat pty,raw,echo=0,link=/tmp/vtty_wireless tcp:<ESP32_IP>:8888
```

### 3. Integration with External Test Harnesses
Once the virtual serial port is exposed at `/tmp/vtty_wireless`, point any external script or test harness from another repository to that device path:

```bash
python3 /path/to/your/test_script.py --port /tmp/vtty_wireless
```
