# Scrolling LED Screens (30x30 Matrix)

An ESP32-powered scrolling LED display project featuring a custom-built 900 LED matrix (30 strips of 30 LEDs) and a high-current modified ATX power supply.

---

## 🚀 Project Overview
This project uses **WS2812B addressable LED strips** to create a large-scale display. It is designed to handle high-density animations and scrolling text. To meet the massive power demands of 900 LEDs (which can draw up to 18-20 Amps at full white), it utilizes a modified computer ATX power supply.

## 🛠 Hardware Components For EACH SCREEN
* **Microcontroller:** ESP32-WROOM-DevKitC (ESP32) for controlling the LED matrix and handling WiFi communication.
* **LED Strips:** 30x WS2812B strips (30 LEDs per strip, totaling 900 pixels).
* **Wooden Frame:** Custom-built frame to hold the LED strips in a 30x30 matrix configuration.
* **Lots of Wires:** For data and power connections.

## ⚡ Power Supply
* **Modified ATX Power Supply:** Capable of delivering high current (20A+) at 5V to power the LED matrix.
* **Power Distribution:** Proper wiring and distribution to ensure stable voltage across all LEDs.


## 📡 Communication
* **WiFi Communication:** the python server should run on the same WiFi network as the ESP32 sender. The ESP32 connects to the WiFi network and communicates with the server to receive pixel data for display. It uses ESP-NOW protocol for low-latency data transmission to the 2 ESP32 receivers controlling the other LED matrices.


## 🖥 Software Component
* **Python Server:** A Python-based server application running on a local machine or Raspberry Pi, responsible for generating and sending pixel data to the ESP32 sender module.
* **ESP32 Sender Module:** The ESP32 module that connects to the WiFi network, receives pixel data from the Python server, and transmits it to the ESP32 receivers using ESP-NOW.
* **ESP32 Receiver Modules:** Two ESP32 modules that receive pixel data from the sender and control their respective LED matrices.
* **Libraries Used:** FastLED for LED control, ESPAsyncWebServer for web server functionality, and ESP-NOW for wireless communication.

## Installation and Setup
1. **Hardware Assembly:** Follow the wiring diagram to connect the LED strips to the ESP32 and power supply.
2. **Software Setup:** Upload the provided code to the ESP32 sender and receiver modules using PlatformIO or Arduino IDE.
3. **Python Server:** Set up and run the Python server on your local machine or Raspberry Pi.
4. **Configuration:** Update the WiFi credentials and server IP address in the ESP32 sender code.
5. **Power On:** Power on the ATX power supply and ensure all components are functioning correctly.
6. **Enjoy the Display:** Send pixel data from the Python server to the ESP32 sender and watch your LED matrix come to life!




## To Do:

* Implement a way of adding characters on the fly (currently hardcoded in the python server)
* Implement a way of drawing your own characters (currently hardcoded font)
* make a mode selector and be able to switch from one mode to another (scrolling text, static image, animations, etc)