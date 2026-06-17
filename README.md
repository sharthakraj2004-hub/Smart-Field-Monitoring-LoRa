# Smart Field Monitoring System using LoRa Communication

## Overview

The Smart Field Monitoring System is an IoT-based agricultural monitoring solution that uses Arduino Mega, ESP8266, and LoRa communication to collect, transmit, and visualize real-time environmental data. The system continuously monitors soil moisture, temperature, water level, and irrigation motor status, enabling remote observation of field conditions through a web-based dashboard.

---

## Hardware Components

### Sensor Node (Arduino Mega)

* Arduino Mega 2560
* LoRa RA-02 Module (433 MHz)
* YL-69 Soil Moisture Sensor
* DS18B20 Temperature Sensor
* HC-SR04 Ultrasonic Sensor
* 16x2 I2C LCD
* Status LEDs

### Gateway Node (ESP8266)

* ESP8266 NodeMCU
* LoRa RA-02 Module
* Wi-Fi Connectivity
* Embedded Web Server

---

## System Architecture

Sensor Node → LoRa Transmission → ESP8266 Gateway → Wi-Fi Network → Web Dashboard

---

## Features

* Real-time soil moisture monitoring
* Temperature monitoring using DS18B20
* Water tank level measurement using ultrasonic sensing
* Automatic motor status indication
* Long-range LoRa communication
* Wireless data transmission
* ESP8266 web server dashboard
* Live sensor visualization
* Moving-average filtering for stable water-level readings
* LCD-based local monitoring

---

## Working Principle

### Sensor Acquisition

The Arduino Mega periodically reads:

* Soil Moisture Sensor
* DS18B20 Temperature Sensor
* Ultrasonic Water-Level Sensor

Sensor values are converted into engineering units and displayed on the local LCD.

### Irrigation Logic

Motor status is determined using:

* Soil Moisture Threshold = 30%
* Temperature Threshold = 35°C

Condition:

If Soil Moisture < 30% AND Temperature < 35°C

Motor Status = ON

Else

Motor Status = OFF

Status LEDs indicate motor operation.

### LoRa Communication

The Arduino Mega packages:

* Soil Moisture
* Temperature
* Water Level
* Motor Status

into a LoRa packet and transmits it using the RA-02 module operating at 433 MHz.

### ESP8266 Gateway

The ESP8266 receives LoRa packets and:

* Extracts sensor values
* Applies moving-average filtering to water-level data
* Stores the latest measurements
* Hosts a web server over Wi-Fi

### Web Dashboard

Users can access the dashboard using a browser connected to the same Wi-Fi network.

Displayed Parameters:

* Soil Moisture (%)
* Temperature (°C)
* Water Level (%)
* Motor Status (ON/OFF)

The dashboard updates automatically every second using AJAX requests.

---

## Communication Flow

YL-69 Soil Sensor

DS18B20 Temperature Sensor

HC-SR04 Water Level Sensor
↓
Arduino Mega 2560
↓
LoRa RA-02 Transmitter
↓
Wireless LoRa Link (433 MHz)
↓
ESP8266 + LoRa Receiver
↓
Wi-Fi Web Server
↓
Live Monitoring Dashboard

---

## Technologies Used

* Embedded C++
* Arduino Mega 2560
* ESP8266 NodeMCU
* LoRa RA-02
* IoT
* Wireless Sensor Networks
* AJAX
* HTML
* SPI Communication
* I2C Communication
* OneWire Protocol

---

## Results

The system successfully demonstrated:

* Reliable long-range LoRa communication
* Real-time environmental monitoring
* Wireless transmission of agricultural data
* Live web-based visualization
* Stable sensor operation under continuous monitoring conditions

---

## Author

Sharthak Raj
