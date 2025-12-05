<!-- filename: README.md -->

# ESP-NOW Cooperative Greenhouse Monitoring System_Team 6

A multi-node, energy-efficient IoT system using **ESP32**, **ESP-NOW**, **MQTT over TLS**, and a **Node-RED dashboard** to monitor greenhouse temperature and propagate cooperative alerts.

Team Members- Ananthapadmanabhan Manoj, Kwin-Isis Din Madiba, Spandana Meanlingi and Jahnavi Makaraju

Refer IOT_Team6_Group_Project_Demonstration.mp4 video for the detailed explaination of our system.

## 1. Overview

This project implements a **cooperative thermal alert network** with:

- **Node 1** – ESP32 WROOM (Sensor+ Gateway Intermediary)
  - DHT11 temperature sensor 
  - NeoPixel RGB LED
  - Monitors temperature/humidity locally.  
  - Sends data and alerts to Node 2 (peer-to-peer) via ESP-NOW.
  - Implements Deep Sleep/Duty Cycling.
  - Triggers local LED alert.
  
- **Node 2** – ESP32 WROOM  
  - DHT11 temperature sensor  
  - NeoPixel RGB LED  
  - Receives alerts from Node 1 via ESP-NOW and changes its LED state cooperatively.
  - Monitors temperature/humidity locally.
  - Implements Deep Sleep/Duty Cycling.
  - Triggers local LED alert.
      
- **Gateway** – ESP32-C3  
  - Receives ESP-NOW packets from both nodes
  - Handles Wi-Fi connection and MQTT communication
  - Listens for ESP-NOW messages from Node 1.  
  - Forwards data and alerts to **HiveMQ Cloud** via MQTT over TLS  
  - Bridges local ESP-NOW network with the Internet  

- **Dashboard** – Node-RED  
  - Subscribes to `greenhouse/#` MQTT topics  
  - Shows real-time node readings and alerts  

The system was implemented as part of the **B31OT – IoT Group Assignment: Cooperative Thermal Alert Network**.
We were unable to use 3 nodes+ gateway setup because one of our Esp32 kit was broken and not functioning properly. So, we used 2 nodes+ gateway setup to demonstrate the assignment requirements. This issue we faced was discussed with our Prof.Mauro during the demonstration and we successfully completed the assignment requirements with system architecture.

---

## 2. Features

- Cooperative **alert propagation** between Node1 and Node2  
- Local **ESP-NOW** for low-latency, energy-efficient communication  
- **MQTT over TLS** to HiveMQ Cloud (secure remote monitoring)  
- **Node-RED dashboard** for visualization and supervision  
- **Duty cycling + deep sleep** on sensor nodes to reduce energy usage  
- **Simple redundant alert suppression** (rate-limiting propagation)  
- Compact JSON payloads for cloud transmission

---

## 3. Hardware

- 2 × ESP32 WROOM Dev Kit (Node1, Node2)  
- 1 × ESP32-C3 Dev Kit (Gateway)  
- 2 × DHT11 temperature sensors  
- 2 × NeoPixel RGB LEDs (1 pixel each)  
- USB cables (for power and programming)

### Pin Mapping (Nodes)

- DHT11 data → `GPIO4`  
- NeoPixel data → `GPIO5`  

> Power is provided via USB for demonstration, but the system is designed with battery operation in mind for energy-efficiency analysis.

---

## 4. MAC Address Configuration

These are the MAC addresses for reference purpose in the codes only (and were replaced with the real board addresses of ours during testing):

```c
Gateway (ESP32-C3):   24:6F:28:AA:BB:01
Node1  (ESP32 WROOM): 24:6F:28:AA:BB:02
Node2  (ESP32 WROOM): 24:6F:28:AA:BB:03
