# ESP-NOW Cooperative Greenhouse Monitoring System - Team 6

A multi-node, energy-efficient IoT system using **ESP32**, **ESP-NOW**, **MQTT over TLS**, and a **Node-RED dashboard** to monitor greenhouse temperature and propagate cooperative alerts.

**Team Members:** Ananthapadmanabhan Manoj, Kwin-Isis Din Madiba, Spandana Meanlingi, and Jahnavi Makaraju

---

## 1. System Overview and Architecture

This project implements a **cooperative thermal alert network** with a hybrid communication architecture :

- **Node 1 & Node 2 (ESP32 WROOM):** Sensor nodes that implement **Deep Sleep/Duty Cycling**, send data/alerts to the Gateway, and exchange **P2P alerts** for cooperative visualization (Blinking Orange NeoPixel).
- **Gateway (ESP32-C3):** Bridges the local **ESP-NOW** network with the Internet, securely forwarding data to **HiveMQ Cloud** via **MQTT over TLS**.
- **Dashboard (Node-RED):** Subscribes to `greenhouse/#` MQTT topics to display real-time readings and global alerts.

---

## 2. Key Technical Features and Achievements

- **Energy Efficiency (Req. 4):** **Deep Sleep Duty Cycling** implemented across all sensor nodes (15-second cycle), minimizing the wake time to conserve battery power.
- **Cooperative Alert Propagation (Req. 3):** Achieved **P2P alert exchange** via ESP-NOW, bypassing the cloud for low-latency network reaction.
- **Hardware Stability Fix:** Implemented **Dynamic Channel Synchronization** (ESP-NOW channel lock on both Gateway and Nodes) to ensure stable communication between the **ESP32-C3** and **ESP32 WROOM** modules.
- **Secure Messaging:** **MQTT over TLS** to HiveMQ Cloud.
- **Data Integrity:** Compact binary **`SensorPacket`** used for low-power transmission, converted to **JSON** by the Gateway for the dashboard.

---

## 3. Challenges Faced and Resolution

During implementation, the team faced several significant challenges that required architectural and technical fixes:

### A. Non-Compliant Architecture (Initial Design Flaw)
* **The Challenge:** The initial code relied on nodes connecting via **Wi-Fi/MQTT** to send *alerts*, which directly violated the **Deep Sleep/Energy Efficiency** requirement (Req. 4). The continuous Wi-Fi connection prevented duty cycling.
* **Resolution:** We fundamentally redesigned the architecture, migrating all local traffic (data and alerts) to the **ESP-NOW P2P protocol**. This allowed us to successfully implement the mandatory Deep Sleep cycle.

### B. Mixed Hardware Communication Failure
* **The Challenge:** Establishing a reliable ESP-NOW link between the **ESP32-C3 Gateway** and the **ESP32-WROOM Nodes** was difficult because these chips handle Wi-Fi channels differently.
* **Resolution:** We implemented **Dynamic Channel Synchronization**: the Gateway connects to the router normally, reads the router's channel, and then forces ESP-NOW to that channel. The Nodes use the **SoftAP trick** to force their channel to match. This resolved the stability issue.

### C. Hardware Failure (Setup Reduction)
* **The Challenge:** One of the three ESP32 kits failed during the initial setup phase.
* **Resolution:** We successfully demonstrated all assignment requirements (including cooperative alerting) using a **2-Node + 1-Gateway setup**. This issue was documented and discussed with Prof. Mauro during the demonstration.

---

## 4. Hardware Specifications

- 2 × ESP32 WROOM Dev Kit (Node1, Node2)   
- 1 × ESP32-C3 Dev Kit (Gateway)  
- 2 × DHT11 temperature sensors  
- 2 × NeoPixel RGB LEDs (1 pixel each)  

### Pin Mapping (Nodes)

- DHT11 data → `GPIO4`  
- NeoPixel data → `GPIO5`   

---

## 5. MAC Address Configuration

These MAC addresses are for code submission and reference purposes only. They were replaced with the real board addresses during testing and deployment.

```c
// MAC ADDRESS ARRAYS (in code)
uint8_t GATEWAY_MAC_ADDR[6] = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0x01};
uint8_t NODE1_MAC_ADDR[6]   = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0x02};
uint8_t NODE2_MAC_ADDR[6]   = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0x03};
