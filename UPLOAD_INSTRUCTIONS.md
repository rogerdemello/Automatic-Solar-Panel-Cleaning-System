# ESP32 Solar Panel Cleaning System - Complete Setup Guide

This guide will walk you through setting up the entire solar panel dust detection and cleaning system from scratch.

## 📋 Table of Contents

1. [Prerequisites](#prerequisites)
2. [Hardware Setup](#hardware-setup)
3. [Software Installation](#software-installation)
4. [Configuration](#configuration)
5. [Upload Firmware](#upload-firmware)
6. [Run the System](#run-the-system)
7. [Testing & Verification](#testing--verification)
8. [Troubleshooting](#troubleshooting)

---

## 📦 Prerequisites

### Hardware Requirements
- **ESP32-CAM (AI Thinker)** with OV2640 camera
- **ESP32 WROOM** (Dev Module)
- **TB6612 Motor Driver**
- **DC Motor** (5-12V)
- **2x IR Sensors** (for edge detection)
- **FTDI Programmer** (for ESP32-CAM)
- **USB cables** for programming
- **Jumper wires** and breadboard
- **External power supply** (5-12V for motor)

### Software Requirements
- **Arduino IDE 2.x** or **VS Code with PlatformIO**
- **Python 3.10+**
- **Git** (for cloning repository)
- **WiFi Network** (2.4GHz - ESP32 doesn't support 5GHz)

---

## 🔌 Hardware Setup

### ESP32-CAM Connections (AI Thinker)

**Camera Module:**
- Comes pre-connected on AI Thinker board

**Programming (via FTDI):**
```
FTDI    →   ESP32-CAM
VCC     →   5V
GND     →   GND
TX      →   U0R (RX)
RX      →   U0T (TX)
        →   GPIO 0 to GND (programming mode)
```

**Important:** Connect GPIO 0 to GND only during upload, then disconnect for normal operation.

### ESP32 WROOM + TB6612 Motor Driver

**TB6612 Motor Driver Connections:**
```
ESP32 Pin   →   TB6612
GPIO 21     →   STBY (Standby - must be HIGH)
GPIO 22     →   BIN1 (Motor direction control)
GPIO 23     →   BIN2 (Motor direction control)
GPIO 4      →   PWMB (PWM speed control)
3.3V/5V     →   VCC (Logic voltage)
GND         →   GND (Common ground - CRITICAL)
External    →   VM (Motor power 5-12V)

TB6612      →   Motor
B01         →   Motor wire 1
B02         →   Motor wire 2
```

**IR Sensors:**
```
ESP32 Pin   →   IR Sensor
GPIO 14     →   Front IR OUT
GPIO 27     →   Back IR OUT
5V          →   VCC (both sensors)
GND         →   GND (both sensors)
```

**⚠️ CRITICAL:** 
- **Common ground** between ESP32, motor driver, and sensors is essential
- **VM (motor power)** must be from external supply, NOT ESP32
- **STBY pin** must be HIGH for motor to work

---

## 💾 Software Installation

### 1. Install Arduino IDE

1. Download from: https://www.arduino.cc/en/software
2. Install and open Arduino IDE
3. Go to **File → Preferences**
4. Add to "Additional Board Manager URLs":
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
5. Go to **Tools → Board → Board Manager**
6. Search "esp32" and install **"ESP32 by Espressif Systems"** (version 3.0.0 or higher)

### 2. Install Required Libraries

**In Arduino IDE:**
- Go to **Sketch → Include Library → Manage Libraries**
- Install these libraries:
  - **ArduinoJson** (version 7.x)
  - **Blynk** (latest version)
  - **ESP32 Camera** (built-in with ESP32 board package)

### 3. Install Python Environment

**Windows (Git Bash):**
```bash
# Navigate to project directory
cd /e/Major\ Project

# Create virtual environment
python -m venv .venv

# Activate virtual environment
source .venv/Scripts/activate

# Install dependencies
pip install -r requirements.txt
```

**Linux/Mac:**
```bash
cd ~/Major\ Project
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

---

## ⚙️ Configuration

### Step 1: Find Your PC's IP Address

**Windows:**
```bash
ipconfig | grep "IPv4"
# Look for: IPv4 Address. . . . : 10.35.166.XXX
```

**Linux/Mac:**
```bash
ifconfig | grep "inet "
```

**Note down this IP** - you'll need it for ESP32-CAM configuration.

### Step 2: Update WiFi Credentials

Edit **both** `.ino` files:

**`ESP32_CAM_Capture/ESP32_CAM_Capture.ino` (lines 13-14):**
```cpp
const char* ssid = "YourWiFiName";      // Your WiFi SSID
const char* password = "YourPassword";   // Your WiFi password
```

**`ESP32_Cleaner/ESP32_Cleaner.ino` (lines 13-14):**
```cpp
const char* ssid = "YourWiFiName";      // Same WiFi network
const char* password = "YourPassword";   // Same password
```

### Step 3: Update Server URL (ESP32-CAM)

**`ESP32_CAM_Capture/ESP32_CAM_Capture.ino` (line 16):**
```cpp
const char* serverUrl = "http://10.35.166.XXX:8000/upload-image";  
// Replace XXX with your PC's IP from Step 1
```

---

## 📤 Upload Firmware

### Upload to ESP32-CAM (AI Thinker)

1. **Open** `ESP32_CAM_Capture/ESP32_CAM_Capture.ino` in Arduino IDE

2. **Configure Board:**
   - **Tools → Board** → ESP32 Arduino → **AI Thinker ESP32-CAM**
   - **Tools → Port** → Select your FTDI COM port
   - **Tools → Upload Speed** → **115200** (if errors, try 921600)

3. **Enter Programming Mode:**
   - Connect **GPIO 0 to GND**
   - Press **RESET** button on ESP32-CAM

4. **Upload:**
   - Click **Upload** button (→)
   - Wait for "Connecting..." message
   - If stuck, press **RESET** button again
   - Wait for "Hard resetting via RTS pin..."

5. **Exit Programming Mode:**
   - **Disconnect GPIO 0 from GND**
   - Press **RESET** button

6. **Verify:**
   - Open **Serial Monitor** (115200 baud)
   - Should see:
     ```
     ESP32-CAM Starting...
     Connecting to WiFi...
     WiFi connected!
     IP Address: 10.35.166.252
     Camera initialized successfully!
     ```
   - **Note down the IP address**

### Upload to ESP32 WROOM (Cleaner)

1. **Open** `ESP32_Cleaner/ESP32_Cleaner.ino` in Arduino IDE

2. **Configure Board:**
   - **Tools → Board** → ESP32 Arduino → **ESP32 Dev Module**
   - **Tools → Port** → Select your ESP32 WROOM COM port
   - **Tools → Upload Speed** → **921600**

3. **Upload:**
   - Click **Upload** button (→)
   - No need for GPIO 0 connection (built-in USB programmer)
   - Wait for completion

4. **Verify:**
   - Open **Serial Monitor** (115200 baud)
   - Should see:
     ```
     Connecting to WiFi...
     ✅ WiFi connected: 10.35.166.104
     🌐 HTTP Server Started
     [Blynk] Connecting...
     Front:1 Back:1 | State: IDLE
     ```
   - **Note down the IP address** (e.g., 10.35.166.104)

---

## 🖥️ Run the System

### Step 1: Update ESP32 Cleaner IP in Server

**Edit `app.py` (line 16):**
```python
ESP32_CLEANER_IP = "10.35.166.104"  # Replace with IP from ESP32 Cleaner Serial Monitor
```

### Step 2: Start Python Server

**Windows (Git Bash):**
```bash
cd /e/Major\ Project
source .venv/Scripts/activate
uvicorn app:app --host 0.0.0.0 --port 8000
```

**Expected Output:**
```
INFO:     Started server process [PID]
INFO:     Waiting for application startup.
INFO:     Application startup complete.
INFO:     Uvicorn running on http://0.0.0.0:8000
```

### Step 3: Verify System is Running

The system should now be fully operational:

1. **ESP32-CAM** captures images every 60 seconds
2. **Server** processes images and logs results
3. **ESP32 Cleaner** waits for cleaning commands

---

## 🧪 Testing & Verification

### Test 1: Image Upload

**After 60 seconds, check server terminal:**
```
INFO:     10.35.166.252:XXXXX - "POST /upload-image HTTP/1.1" 200 OK
[DEBUG] Dust: 15.18% (threshold: 20.0%)
[DEBUG] Contrast: 18.60 (threshold: 40.0)
[DEBUG] Cleaning required: True
```

**Check saved images:**
```bash
ls received_images/
# Should see: solar_panel_YYYYMMDD_HHMMSS.jpg
```

### Test 2: Manual Cleaning Command

**Test ESP32 Cleaner web server:**
```bash
curl -X POST http://10.35.166.104/clean \
  -H "Content-Type: application/json" \
  -d '{"command":"start"}'
```

**Expected Response:**
```json
{"status":"started"}
```

**ESP32 Cleaner Serial Monitor should show:**
```
🟢 Cleaning started via HTTP
Front:0 Back:1 | State: MOVING_FORWARD
```

**To stop:**
```bash
curl -X POST http://10.35.166.104/clean \
  -H "Content-Type: application/json" \
  -d '{"command":"stop"}'
```

### Test 3: Check Server Status

**In browser or curl:**
```bash
curl http://10.35.166.188:8000/status
```

**Response:**
```json
{
  "status": "running",
  "dust_threshold": 20.0,
  "contrast_threshold": 40.0,
  "esp32_cleaner_ip": "10.35.166.104"
}
```

### Test 4: Full Automated Cycle

1. Cover camera with dusty cloth
2. Wait 60 seconds
3. Check server logs - should show `cleaning_required: true`
4. Check server logs - should show `[DEBUG] Sending cleaning command...`
5. ESP32 Cleaner should start moving
6. Remove cloth and wait for cleaning cycle to complete

---

## 🔧 Troubleshooting

### ESP32-CAM Issues

**Problem: Camera init failed**
- Check camera ribbon cable connection
- Try different board variant: **Tools → Board → ESP32 Wrover Module**
- Power cycle the ESP32-CAM

**Problem: WiFi won't connect**
- Verify 2.4GHz network (ESP32 doesn't support 5GHz)
- Check SSID and password (case-sensitive)
- Move closer to router
- Check Serial Monitor for error messages

**Problem: Upload failed**
- Ensure GPIO 0 is connected to GND during upload
- Press RESET button when "Connecting..." appears
- Try lower upload speed: **115200**
- Check FTDI TX/RX are crossed (TX→RX, RX→TX)

**Problem: Connection refused to server**
- Verify PC IP address hasn't changed
- Check server is running: `netstat -an | grep 8000`
- Update `serverUrl` in ESP32_CAM_Capture.ino with current PC IP
- Check Windows Firewall - allow port 8000

### ESP32 Cleaner Issues

**Problem: Motor doesn't move**
- Check STBY pin is HIGH: `digitalWrite(STBY, HIGH);`
- Verify VM power supply (5-12V external)
- Check common ground between ESP32 and motor driver
- Increase dutyCycle to 255 for testing
- Measure voltage on PWMB pin (should vary)
- Test motor directly with power supply

**Problem: Web server connection refused**
- Check ESP32 Serial Monitor shows "🌐 HTTP Server Started"
- Verify IP address hasn't changed
- Update ESP32_CLEANER_IP in app.py
- Ping the ESP32: `ping 10.35.166.104`
- Check `server.handleClient()` is in loop()

**Problem: ArduinoJson errors**
- Ensure using **JsonDocument** not **StaticJsonDocument**
- Update to ArduinoJson v7
- Check for syntax errors in JSON parsing

### Server Issues

**Problem: Module not found**
```bash
# Reinstall dependencies
pip install -r requirements.txt
```

**Problem: Port already in use**
```bash
# Windows - find process using port 8000
netstat -ano | findstr :8000
# Kill process by PID
taskkill /PID <PID> /F

# Linux/Mac
lsof -ti:8000 | xargs kill -9
```

**Problem: Permission denied on port 8000**
```bash
# Use a different port
uvicorn app:app --host 0.0.0.0 --port 8080
# Update ESP32-CAM serverUrl to port 8080
```

### Network Issues

**Problem: Devices on different subnets**
- Check all devices show 10.35.166.XXX (same subnet)
- Reconnect all devices to same WiFi network
- Restart router if necessary
- Update all IP addresses in code

**Problem: IP addresses keep changing**
- Configure static IP in router (DHCP reservation)
- Or check and update IPs each time before running

---

## 📊 System Status Indicators

### ESP32-CAM Serial Monitor
```
✅ Normal Operation:
   "Camera initialized successfully!"
   "Capturing image..."
   "HTTP Response code: 200"

❌ Errors:
   "Camera init failed" → Hardware issue
   "WiFi connect failed" → Network issue  
   "HTTP Response code: -1" → Server unreachable
```

### ESP32 Cleaner Serial Monitor
```
✅ Normal Operation:
   "✅ WiFi connected: 10.35.166.104"
   "🌐 HTTP Server Started"
   "Front:1 Back:1 | State: IDLE"

🟢 Cleaning Active:
   "🟢 Cleaning started via HTTP"
   "Front:0 Back:1 | State: MOVING_FORWARD"

❌ Errors:
   No HTTP server message → server.begin() failed
   WiFi timeout → Check credentials
```

### Python Server Terminal
```
✅ Normal Operation:
   "INFO: Uvicorn running on http://0.0.0.0:8000"
   "[DEBUG] Dust: 15.18% (threshold: 20.0%)"
   "[DEBUG] Cleaning required: True"
   "[DEBUG] Response: 200 - {"status":"started"}"

❌ Errors:
   "Connection refused" → ESP32 Cleaner not responding
   "Timeout" → Network issue
   "[ERROR] Failed to send cleaning command" → Check IP
```

---

## 🔄 IP Address Quick Reference

| Device | Default IP | How to Find | Where to Update |
|--------|-----------|-------------|-----------------|
| **PC Server** | 10.35.166.188 | `ipconfig` (Windows)<br>`ifconfig` (Linux) | ESP32_CAM_Capture.ino line 16 |
| **ESP32-CAM** | 10.35.166.252 | Serial Monitor after boot | (Not needed elsewhere) |
| **ESP32 Cleaner** | 10.35.166.104 | Serial Monitor after boot | app.py line 16 |

**Note:** These IPs change based on your router's DHCP. Always verify using Serial Monitor and update accordingly.

---

## ✅ Success Checklist

- [ ] ESP32-CAM connects to WiFi and shows IP
- [ ] ESP32-CAM captures images successfully  
- [ ] ESP32 Cleaner connects to WiFi and shows IP
- [ ] ESP32 Cleaner shows "HTTP Server Started"
- [ ] Python server starts without errors
- [ ] Images appear in `received_images/` folder
- [ ] Server logs show dust detection results
- [ ] Manual cleaning command works (curl test)
- [ ] Motor moves forward and backward
- [ ] IR sensors detect surface vs air correctly
- [ ] Automated cleaning triggers when dust detected

---

## 📞 Getting Help

If you encounter issues not covered here:

1. Check Serial Monitor outputs for error messages
2. Verify all IP addresses are current
3. Test each component individually
4. Review the [Troubleshooting](#troubleshooting) section
5. Open an issue on GitHub with:
   - Error messages
   - Serial Monitor output
   - Server terminal output
   - Network configuration

---

## 🎉 System is Ready!

Once all checks pass, your automated solar panel cleaning system is operational. The system will:

- ✅ Automatically capture images every 60 seconds
- ✅ Detect dust levels using computer vision
- ✅ Trigger cleaning when thresholds are exceeded
- ✅ Execute cleaning cycle with IR-guided navigation
- ✅ Save all captured images for analysis

Happy cleaning! 🌞
