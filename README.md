# 🚀 Heliosphere – AI Smart Helmet System
 🏆 Developed for Confluence2k26

---

## 📌 Executive Summary

Heliosphere is an edge-AI powered smart helmet system designed for real-time safety monitoring in road and industrial environments.

Built using ESP32 + TinyML, the system performs on-device risk prediction by combining environmental sensing, motion analysis, and contextual awareness.

Unlike traditional safety systems that rely on static thresholds, Heliosphere delivers:

- ⚡ Real-time inference (no cloud dependency)
- 🧠 Context-aware decision making
- 🚨 Immediate hazard detection and response

---


## 🆕 System Evolution


### 🟢 Version 1.0 — Rule-Based Prototype

📅 10 April 2026

- Threshold-based alerts
- Basic helmet safety logic
- IoT connectivity

---

### 🔵 Version 2.0 — Heliosphere (AI System)

📅 13 April 2026

- TinyML-based prediction on ESP32
- Multi-sensor fusion
- Risk classification:
  - SAFE
  - WARNING
  - CRITICAL
- Accident detection
- Gas hazard detection
- GPS emergency alerts
- Blynk cloud integration
- OLED live display

👉 Transitioned from reactive system → predictive intelligence

---

## 🧠 System Architecture

Sensors → Feature Engineering → Normalization → TinyML Model →
Risk Prediction → Alerts + Cloud + Display

---

## 🎥 Project Demo

[![Watch Heliosphere Demo](https://img.youtube.com/vi/YOUR_VIDEO_ID/0.jpg)](https://www.youtube.com/watch?v=YOUR_VIDEO_ID)

---
## 📂 Repository Structure
'''
ConfluenceK26/
│
├── esp32/
│   ├── smart_helmet.ino
│   ├── model.h
│
├── TinyML/
│   ├── data.csv
│   ├── model.tflite
│   ├── model.h
│   ├── norm.json
│   ├── metrics.txt
│
├── simulation/
│   ├── diagram.json
│   ├── simulation.png
│   ├── wokwi-project.txt
│
├── docs/
│   ├── initial/
│
└── README.md
'''
---

## ⚙️ Key Features

- Real-time sensor monitoring
- Edge AI inference (TinyML)
- Multi-sensor data fusion
- Accident & hazard detection
- GPS-based emergency response
- IoT dashboard (Blynk)
- On-device OLED visualization

---

## 🧠 Tech Stack

- ESP32
- Arduino (C/C++)
- TensorFlow Lite Micro
- Blynk IoT
- Wokwi Simulation

---

## 🔌 Hardware

- ESP32
- MPU6050
- MQ2 / MQ7 / MQ135
- DHT22
- BMP280
- SW420
- GPS Module
- OLED Display
- Buzzer + SOS Button

---

## 🎯 Applications

- Smart road safety
- Industrial worker protection
- Wearable safety systems
- Edge AI research

---

## 🚧 Future Scope

- Mobile app integration
- Cloud analytics dashboard
- Improved ML accuracy
- Fully wearable compact design

---

## 👥 Contributors & Responsibilities

- Anand Kumar (Anand Ambastha)
  Role: System Architect & Lead Developer
  Contributions:
  - Designed overall system architecture (ESP32 + TinyML pipeline)
  - Developed and integrated all sensor modules
  - Built and deployed TinyML model (data → training → model.h)
  - Implemented ESP32 firmware, IoT (Blynk), and real-time logic
  - Debugging, optimization, and final system integration

---

- Ananya Suman
  Role: Team Lead & Presentation Design
  Contributions:
  - Coordinated team workflow and task management
  - Designed project poster and visual presentation
  - Structured project narrative for expo/demo

---

- Aayush Gupta
  Role: Technical Documentation & Research
  Contributions:
  - Prepared technical paper and documentation
  - Assisted in system explanation and methodology writing
  - Contributed to report structuring

---

- Sonvi Goyal
  Role: Documentation & Compilation
  Contributions:
  - Compiled project reports and supporting documents
  - Assisted in formatting and organizing documentation
  - Ensured clarity and consistency in written materials

---

## 🏁 Positioning

«Heliosphere demonstrates how Edge AI can transform conventional safety systems into intelligent, real-time decision platforms, enabling faster and more reliable responses in critical situations.»