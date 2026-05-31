# 🤖 ZephyrAI – Edge AI Assistant on Zephyr RTOS

## Overview

ZephyrAI is an Edge AI-powered embedded assistant built on ESP32-S3 using Zephyr RTOS, TinyML, Edge Impulse, and TensorFlow Lite Micro.

The system performs real-time natural language intent classification directly on-device without requiring cloud connectivity. It can process user commands, control hardware peripherals, monitor IMU sensor data, and respond intelligently using an embedded machine learning model.

The project demonstrates practical deployment of Edge AI models on resource-constrained microcontrollers while leveraging the capabilities of a real-time operating system.

---

# Features

## TinyML Intent Classification

Classifies user commands into:

* LED ON
* LED OFF
* BLINK
* TEMPERATURE
* STATUS
* GREETING

All inference runs locally on ESP32-S3.

---

## Edge AI Processing

* Edge Impulse Integration
* TensorFlow Lite Micro
* On-device Inference
* No Internet Required
* Low Latency Predictions

---

## Zephyr RTOS

* Real-Time Task Management
* Hardware Abstraction
* Device Tree Support
* Modular Architecture
* Embedded Driver Framework

---

## IMU Monitoring

Supports:

* Accelerometer Data
* Gyroscope Data
* Motion Detection
* Shake Detection
* Tilt Detection

Using:

LSM6DSOX IMU Sensor

---

## Hardware Control

Supports:

* LED ON/OFF
* LED Blink Patterns
* Status Monitoring
* Sensor Feedback

---

# Hardware Components

| Component                  | Quantity |
| -------------------------- | -------- |
| ESP32-S3 Development Board | 1        |
| LSM6DSOX IMU Sensor        | 1        |
| LED                        | 1        |
| USB UART Interface         | 1        |

---

# System Architecture

User Command
↓
Text Processing
↓
Feature Extraction
↓
TinyML Model
↓
Intent Classification
↓
Action Execution

Examples:

LED Control

Sensor Reading

System Status

IMU Analysis

---

# AI Pipeline

User Input

↓

Keyword Extraction

↓

Feature Vector Generation

↓

Edge Impulse Model

↓

TensorFlow Lite Micro

↓

Classification Result

↓

Embedded Response

---

# Supported Commands

## LED Commands

turn on led

turn off led

blink

---

## Temperature Queries

temperature

temp

how hot is it

---

## System Status

status

check system

system report

---

## Greeting Commands

hello

hi

good morning

hey

---

## IMU Commands

imu

gyro

accel

tilt

shake

motion

axis

---

# Machine Learning Features

## TinyML Inference

* Local Execution
* Low Memory Footprint
* Real-Time Predictions
* Energy Efficient

## Classification Engine

Built using:

* Edge Impulse Studio
* TensorFlow Lite Micro
* Edge Impulse SDK

---

# IMU Functions

## Accelerometer

Measures:

* X Axis
* Y Axis
* Z Axis

---

## Gyroscope

Measures:

* Angular Velocity
* Motion Orientation

---

## Shake Detection

Automatically detects sudden motion.

---

## Tilt Detection

Detects:

* Flat Position
* Side Position
* Upside-Down Orientation

---

# Software Stack

## RTOS

* Zephyr RTOS

## Machine Learning

* Edge Impulse
* TensorFlow Lite Micro
* TinyML

## Programming

* Embedded C++
* C

## Build System

* CMake
* West Build

---

# Project Structure

```text
project/
│
├── src/
│   ├── main.cpp
│   └── esp_dsp_stub.c
│
├── ei-model/
│   ├── edge-impulse-sdk/
│   ├── tflite-model/
│   └── model-parameters/
│
├── CMakeLists.txt
│
└── prj.conf
```

---

# Build & Flash

## Initialize Zephyr

```bash
west init
west update
```

## Build

```bash
west build -b esp32s3_devkitm
```

## Flash

```bash
west flash
```

## Monitor

```bash
west espressif monitor
```

---

# Example Interaction

```text
You: turn on led

[ZephyrAI] Let there be light! LED is ON!
```

```text
You: show status

[ZephyrAI] SYSTEM STATUS REPORT
LED : ON
IMU : LSM6DSOX OK
Uptime : 120 sec
```

```text
You: imu

[ZephyrAI] Accel X : 0.24
[ZephyrAI] Accel Y : -0.12
[ZephyrAI] Accel Z : 9.81
```

---

# Applications

* Edge AI Research
* TinyML Deployment
* Smart Embedded Assistants
* Voice Command Systems
* Sensor Intelligence
* Industrial Monitoring
* Intelligent IoT Devices
* Human-Machine Interfaces

---

# Advantages

✔ Edge AI Inference

✔ No Cloud Dependency

✔ Zephyr RTOS Based

✔ TinyML Deployment

✔ Real-Time Processing

✔ IMU Integration

✔ Hardware Control

✔ Resource Efficient

---

# Future Improvements

* Voice Recognition
* Wake Word Detection
* BLE Connectivity
* WiFi Dashboard
* MQTT Integration
* Edge AI Sensor Fusion
* Multi-Intent Classification
* Natural Language Processing
* OTA Firmware Updates

---

# Project Highlights

✔ ESP32-S3

✔ Zephyr RTOS

✔ Edge Impulse

✔ TinyML

✔ TensorFlow Lite Micro

✔ Embedded AI

✔ IMU Analytics

✔ Real-Time Embedded Systems

✔ On-Device Intelligence

---

# Author

Hariharan Balakrishnan

B.E Electronics and Communication Engineering

Embedded Systems | Edge AI | TinyML | Zephyr RTOS | ESP32-S3 | IoT
