# ESP32 Solar Panel Dust Detection & Cleaning System

Automated solar panel dust detection and cleaning system using computer vision and IoT. The system continuously monitors solar panel cleanliness using an ESP32-CAM, processes images with OpenCV for dust detection, and triggers automated cleaning when necessary.

## 🎯 System Overview

The system operates in a fully automated cycle:

1. **ESP32-CAM** captures solar panel images every 60 seconds
2. **FastAPI Server** (running on PC) receives and processes images using OpenCV
3. **Dust Detection Algorithm** analyzes images for dust percentage and contrast
4. **ESP32 Cleaner** receives cleaning commands and controls the motor
5. **Automated Cleaning** cycle with IR sensor-based edge detection

## 🔧 Hardware Components

### Controllers
- **ESP32-CAM (AI Thinker)** - Image capture and transmission
- **ESP32 WROOM** - Motor control and web server

### Peripherals
- **TB6612 Motor Driver** - Bidirectional motor control
- **DC Motor** - Cleaning mechanism
- **IR Sensors (x2)** - Front and back edge detection (surface/air detection)

### Power Requirements
- ESP32-CAM: 5V (via FTDI or dedicated supply)
- ESP32 WROOM: 5V (USB or external)
- Motor Driver VM: 5-12V (motor voltage)
- Motor Driver VCC: 3.3V or 5V (logic voltage)

## 💻 Software Stack

### Backend
- **FastAPI** - Asynchronous web framework
- **OpenCV** - Image processing and dust detection
- **Uvicorn** - ASGI server
- **Python 3.10+** - Runtime environment

### Embedded
- **ESP32 Arduino Core 3.x** - Framework
- **ArduinoJson v7** - JSON parsing
- **ESP32 WebServer** - HTTP endpoints
- **Blynk IoT** - Remote control (optional)
- **ESP32 Camera Library** - OV2640 interface

## 📁 Project Structure

```
.
├── app.py                           # FastAPI server with dust detection
├── requirements.txt                 # Python dependencies
├── ESP32_CAM_Capture/
│   └── ESP32_CAM_Capture.ino       # ESP32-CAM firmware
├── ESP32_Cleaner/
│   └── ESP32_Cleaner.ino           # ESP32 motor controller firmware
├── README.md                        # Project overview (this file)
└── UPLOAD_INSTRUCTIONS.md          # Detailed setup guide
```

## 🚀 Quick Start

### Prerequisites
- Arduino IDE with ESP32 board support (v3.0.0+)
- Python 3.10+ with pip
- WiFi network (2.4GHz)

### Setup Steps

1. **Clone the repository**
   ```bash
   git clone https://github.com/rogerdemello/Automatic-Solar-Panel-Cleaning-System.git
   cd Automatic-Solar-Panel-Cleaning-System
   ```

2. **Install Python dependencies**
   ```bash
   python -m venv .venv
   .venv/Scripts/activate  # On Windows
   pip install -r requirements.txt
   ```

3. **Configure WiFi and IPs**
   - Update WiFi credentials in both `.ino` files
   - Update server URL in `ESP32_CAM_Capture.ino` (line 16)
   - Update cleaner IP in `app.py` (line 16)

4. **Upload firmware**
   - Upload `ESP32_CAM_Capture.ino` to ESP32-CAM
   - Upload `ESP32_Cleaner.ino` to ESP32 WROOM

5. **Start the server**
   ```bash
   uvicorn app:app --host 0.0.0.0 --port 8000
   ```

See [UPLOAD_INSTRUCTIONS.md](UPLOAD_INSTRUCTIONS.md) for detailed setup.

## ⚙️ Configuration

### Dust Detection Thresholds
```python
DUST_THRESHOLD = 20.0        # Percentage of dusty pixels
CONTRAST_THRESHOLD = 40.0    # Image contrast value
```

Cleaning triggers when: `dust_percentage > 20%` **OR** `contrast < 40`

### Network Configuration
Update these based on your network setup:

| Component | File | Line | Parameter |
|-----------|------|------|-----------|
| WiFi SSID | Both `.ino` files | 13 | `ssid` |
| WiFi Password | Both `.ino` files | 14 | `password` |
| Server URL | `ESP32_CAM_Capture.ino` | 16 | `serverUrl` |
| Cleaner IP | `app.py` | 16 | `ESP32_CLEANER_IP` |

### Motor Control
```cpp
dutyCycle = 255              // PWM duty cycle (0-255)
IR_SENSOR_FORWARD = 14       // Front IR sensor
IR_SENSOR_BACKWARD = 27      // Back IR sensor
```

**IR Sensor Logic:**
- `HIGH` = Air (no surface detected)
- `LOW` = Surface (panel detected)

## 🔄 System Flow

```
┌─────────────────┐
│  ESP32-CAM      │  Captures image every 60 seconds
└────────┬────────┘
         │ HTTP POST (multipart/form-data)
         ▼
┌─────────────────┐
│  FastAPI Server │  Processes with OpenCV
│  (PC/Laptop)    │  - Grayscale conversion
└────────┬────────┘  - Gaussian blur
         │            - Otsu thresholding
         │            - Dust calculation
         ▼
┌─────────────────┐
│ Dust Detection  │  dust > 20% OR contrast < 40
└────────┬────────┘
         │ If cleaning required
         │ HTTP POST /clean {"command":"start"}
         ▼
┌─────────────────┐
│ ESP32 Cleaner   │  Web server receives command
│ (Motor Control) │  - State: IDLE → MOVING_FORWARD
└────────┬────────┘  - Motor activates
         │
         ▼
┌─────────────────┐
│ Cleaning Cycle  │  Forward → IR detects edge → Backward
│ (with IR)       │  Stops when back at starting position
└─────────────────┘
```

## 📡 API Endpoints

### Server (FastAPI)
- `POST /upload-image` - Receive image from ESP32-CAM
- `GET /status` - Server status and configuration
- `GET /` - API information

### ESP32 Cleaner (WebServer)
- `POST /clean` - Control cleaning motor
  ```json
  {"command": "start"}  // Start cleaning
  {"command": "stop"}   // Stop cleaning
  ```

## 🧪 Testing

### Test Image Upload
```bash
# Check server logs for received images
# Images saved to: received_images/
```

### Test Cleaning Command
```bash
curl -X POST http://<CLEANER_IP>/clean \
  -H "Content-Type: application/json" \
  -d '{"command":"start"}'
```

## 🛠️ Troubleshooting

### ESP32-CAM Issues
- **Camera init failed**: Check camera module connection, try different board variant
- **WiFi connection timeout**: Verify 2.4GHz network, check credentials
- **Upload failed**: Connect GPIO0 to GND during upload, press RESET when "Connecting..."

### Server Issues
- **Connection refused**: Check server is running, verify firewall settings
- **Module not found**: Install requirements: `pip install -r requirements.txt`
- **Port already in use**: Change port or kill process: `netstat -ano | findstr :8000`

### Motor Issues
- **Motor not moving**: Check VM power supply, verify STBY pin HIGH, test dutyCycle=255
- **Wrong direction**: Swap BIN1/BIN2 pins or change motor polarity
- **Erratic movement**: Verify common ground between ESP32 and motor driver

### Network Issues
- **IP address changed**: Update `serverUrl` in ESP32-CAM and `ESP32_CLEANER_IP` in app.py
- **Connection timeout**: Check all devices on same subnet, verify IPs with `ipconfig`

## 📊 Performance

- **Image Capture**: 60-second intervals
- **Processing Time**: ~100-300ms per image
- **Network Latency**: ~50-200ms (local WiFi)
- **Cleaning Cycle**: Variable (depends on panel length and IR sensors)

## 🔐 Security Notes

- System runs on local network (no internet required)
- No authentication on ESP32 endpoints (add if deploying publicly)
- WiFi credentials stored in plaintext in firmware

## 📝 License

This project is for educational purposes (Major Project 2025).

## 👨‍💻 Author

Created by Roger Demello for Major Project 2025

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues.

## 📮 Support

For issues and questions, please open an issue on GitHub.
