<!-- filename: README.md -->

# ESP-NOW Cooperative Greenhouse Monitoring System

A multi-node, energy-efficient IoT system using **ESP32**, **ESP-NOW**, **MQTT over TLS**, and a **Node-RED dashboard** to monitor greenhouse temperature and propagate cooperative alerts.

## 1. Overview

This project implements a **cooperative thermal alert network** with:

- **Node 1** – ESP32 WROOM  
  - DHT11 temperature sensor  
  - NeoPixel RGB LED  
  - ESP-NOW peer-to-peer + ESP-NOW → Gateway  

- **Node 2** – ESP32 WROOM  
  - DHT11 temperature sensor  
  - NeoPixel RGB LED  
  - ESP-NOW peer-to-peer + ESP-NOW → Gateway  

- **Gateway** – ESP32-C3  
  - Receives ESP-NOW packets from both nodes  
  - Forwards data and alerts to **HiveMQ Cloud** via MQTT over TLS  
  - Bridges local ESP-NOW network with the Internet  

- **Dashboard** – Node-RED  
  - Subscribes to `greenhouse/#` MQTT topics  
  - Shows real-time node readings and alerts  

The system was implemented as part of the **B31OT – IoT Group Assignment: Cooperative Thermal Alert Network (2025/26)**.

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

These are the MAC addresses used in the code (and were replaced with the real board addresses during testing):

```c
Gateway (ESP32-C3):   24:6F:28:AA:BB:01
Node1  (ESP32 WROOM): 24:6F:28:AA:BB:02
Node2  (ESP32 WROOM): 24:6F:28:AA:BB:03
