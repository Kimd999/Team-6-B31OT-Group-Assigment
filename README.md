# B31OT-Group-Assigment


## 📘 Overview

The **Cooperative Thermal Alert Network (CTAN)** system is designed to detect abnormal temperature conditions and coordinate alerts between interconnected sensor nodes, representing by ESP32 MCU equipped with DHT11 sensors.

Each node monitors local temperature, communicates peer-to-peer using **ESP-NOW**, and shares data with a **gateway** connected to a **web dashboard** for real-time supervision.

---

## 🧩 System Architecture

### Components
- **ESP32 Sensor Nodes (x3+)**
  - Equipped with a DHT11 temperature sensor
  - RGB LED for visual alert indication
  - Communicate using ESP-NOW
- **ESP32 Gateway Node**
  - Connects to nodes via ESP-NOW
  - Publishes data and alerts to an MQTT broker
  - Hosts or connects to a web-based dashboard

### Communication Flow
1. Each node monitors temperature locally.  
2. If a threshold is exceeded, the node:
   - Activates local LED alert  
   - Broadcasts an alert message to neighbors via ESP-NOW  
3. Gateway receives and forwards data to MQTT broker.  
4. Dashboard visualizes node data and alerts.

   <img width="625" height="417" alt="image" src="https://github.com/user-attachments/assets/a59cfec7-1f52-45d3-911d-dcead2e79e37" />


---

## ⚙️ Features

- 🔥 **Anomaly Detection** – Automatic detection of temperature beyond configurable limits.  
- 💬 **Peer-to-Peer Coordination** – ESP-NOW-based alert propagation between nodes.  
- 💤 **Energy Efficiency** – Duty cycling and message encoding to reduce power consumption.  
- 📡 **Gateway Integration** – MQTT-based data forwarding for cloud or local dashboard visualization.  
- 💻 **Web Dashboard** – Real-time monitoring of temperatures, alert states, and network topology.  
- 🧠 **Scalable & Extensible** – Easy node addition and flexible configuration options.

---

## 🧠 System Requirements

### Hardware
- ESP32 microcontrollers (x4)
- DHT11 temperature sensors (x3)
- RGB NeoPixel LEDs
- Power via USB (for demo) or battery

### Software
- Arduino IDE / PlatformIO
- Required libraries:
  - `WiFi.h`
  - `esp_now.h`
  - `PubSubClient.h`
  - `DHT.h`
  - `ArduinoJson.h`

### Server
- Mosquitto MQTT broker 
- Node-RED 

---
