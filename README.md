# Scrolling LED Screens (3x 30x30 Matrix)

An ESP32-powered scrolling LED display project featuring 3 custom-built 900 LED matrix (30 strips of 30 LEDs) and a high-current modified ATX power supply. This system uses a Python server and ESP-NOW for low-latency multi-screen synchronization.

---

## 🚀 Project Overview
This project uses **WS2812B addressable LED strips** to create a large-scale display. It is designed to handle high-density animations and scrolling text. To meet the massive power demands of 900 LEDs (up to 20 Amps), it utilizes a modified computer ATX power supply.

## 📹 Demo
![LED Screen Demo](assets/video_screen_scrolling.gif)
## 🛠 Hardware Components (Per Screen)
* **Microcontroller:** ESP32-WROOM-DevKitC.
* **LED Strips:** 30x WS2812B strips (30 LEDs per strip, totaling 900 pixels). (Recommended: 5V, 60 LEDs/meter).
* **Wooden Frame:** Custom-built 30x30 matrix housing.
* **Modified ATX Power Supply:** Dedicated **5V @ 20A+** rail.
* **Wiring:** Heavy-gauge power distribution wires to prevent voltage drop.

## 📡 Communication & Network Requirements
The system relies on a combination of standard WiFi (Server to Sender) and ESP-NOW (Sender to Receivers).

> ### ⚠️ CRITICAL: WiFi Configuration
> **The ESP32 is only compatible with 2.4GHz WiFi networks.** > * Ensure your router is broadcasting a **2.4GHz signal**. 
> * The system **will not connect** to 5GHz-only networks.
> * For best results with ESP-NOW, ensure the WiFi channel is fixed (not "Auto") in your router settings to prevent the Sender and Receivers from losing sync.

## 🖥 Software Architecture
* **Python Server:** Generates and sends pixel data to the ESP32 Sender.
* **ESP32 Sender:** Connects to 2.4GHz WiFi, receives server data, and broadcasts it via **ESP-NOW**.
* **ESP32 Receivers (x2):** Receive low-latency data from the Sender to drive their respective 900-LED matrices.
* **Libraries:** `FastLED`, `ESPAsyncWebServer`, and `ESPNOW`.

## ⚙️ Installation and Setup
1. **Hardware Assembly:** Connect LED strips to the ESP32 and power supply according to the schematic  
   [here](assets/schematic_global.kicad_prl). for kicad_prl  
   [here](assets/schematic_global.kicad_pro). for kicad_pro  
   [here](assets/schematic_global.kicad_sch). for kicad_sch  

2. **Software Upload:** Flash the sender and receiver code using PlatformIO or Arduino IDE.
3. **Network Config:** * Set your WiFi SSID and Password in the Sender code.
    * **Ensure you are connecting to a 2.4GHz channel.**
4. **Python Server:** Run the server on a machine within the same 2.4GHz network.
5. **Power On:** Turn on the ATX supply and check for stable 5V delivery.

## 📝 To Do
* [ ] **Dynamic Characters:** Implement on-the-fly character addition (remove hardcoding).
* [ ] **Custom Drawing:** Create a UI/method for custom character design.
* [ ] **Mode Selector:** Toggle between scrolling text, static images, and animations.

---

### ⚡ Power Safety Warning
A modified ATX supply can deliver very high current. Ensure all connections are soldered securely and insulated. Always use a common ground between the ATX PSU and the ESP32.