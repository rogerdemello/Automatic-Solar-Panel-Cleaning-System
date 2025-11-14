# app.py

from fastapi import FastAPI, UploadFile, File
from fastapi.responses import JSONResponse
import numpy as np
import cv2
import requests
from datetime import datetime
import os

app = FastAPI()

# Configuration
DUST_THRESHOLD = 10.0  # Dust percentage threshold - average dusty panel
CONTRAST_THRESHOLD = 50.0  # Contrast threshold - low contrast indicates dust
ESP32_CLEANER_IP = "192.168.1.100"  # Replace with your ESP32 Cleaner IP address
SAVE_IMAGES = True  # Save received images for debugging
IMAGE_SAVE_PATH = "received_images"

if SAVE_IMAGES and not os.path.exists(IMAGE_SAVE_PATH):
    os.makedirs(IMAGE_SAVE_PATH)

def process_image_from_bytes(image_bytes, dust_threshold=DUST_THRESHOLD):
    np_arr = np.frombuffer(image_bytes, np.uint8)
    image = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)

    if image is None:
        return {"error": "Failed to decode image. Make sure it's a valid image file."}

    gray_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    blurred = cv2.GaussianBlur(gray_image, (5, 5), 0) 
    _, thresh_image = cv2.threshold(blurred, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)

    dust_pixels = np.sum(thresh_image == 255)
    total_pixels = gray_image.shape[0] * gray_image.shape[1]
    dust_percentage = (dust_pixels / total_pixels) * 100
    contrast = gray_image.std()

    # Cast numpy.bool_ to Python bool
    cleaning_required = bool(dust_percentage > dust_threshold or contrast < CONTRAST_THRESHOLD)
    
    print(f"[DEBUG] Dust: {dust_percentage:.2f}% (threshold: {dust_threshold}%)")
    print(f"[DEBUG] Contrast: {contrast:.2f} (threshold: {CONTRAST_THRESHOLD})")
    print(f"[DEBUG] Cleaning required: {cleaning_required}")

    return {
        "dust_percentage": round(float(dust_percentage), 2),
        "contrast": round(float(contrast), 2),
        "cleaning_required": cleaning_required,
        "image": image  # Return image for saving
    }

def send_cleaning_command(esp_ip, should_clean):
    """Send cleaning command to ESP32 cleaner"""
    try:
        url = f"http://{esp_ip}/clean"
        payload = {"command": "start" if should_clean else "stop"}
        print(f"[DEBUG] Sending cleaning command to {url}")
        print(f"[DEBUG] Payload: {payload}")
        response = requests.post(url, json=payload, timeout=5)
        print(f"[DEBUG] Response: {response.status_code} - {response.text}")
        return {"success": True, "status_code": response.status_code}
    except Exception as e:
        print(f"[ERROR] Failed to send cleaning command: {e}")
        return {"success": False, "error": str(e)}
        return {"success": False, "error": str(e)}

@app.post("/check-dust")
async def check_dust(file: UploadFile = File(...)):
    """Endpoint for ESP32-CAM to upload images"""
    contents = await file.read()
    result = process_image_from_bytes(contents)

    if "error" in result:
        return JSONResponse(status_code=400, content={"error": result["error"]})

    # Save image if enabled
    if SAVE_IMAGES and "image" in result:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"{IMAGE_SAVE_PATH}/solar_panel_{timestamp}.jpg"
        cv2.imwrite(filename, result["image"])
        result["saved_image"] = filename
    
    # Remove image from response
    if "image" in result:
        del result["image"]
    
    # Send cleaning command to ESP32 if cleaning is required
    if result["cleaning_required"]:
        clean_result = send_cleaning_command(ESP32_CLEANER_IP, True)
        result["cleaning_command_sent"] = clean_result
    
    return result

@app.post("/upload-image")
async def upload_image(file: UploadFile = File(...)):
    """Alternative endpoint that returns simple yes/no for cleaning"""
    contents = await file.read()
    result = process_image_from_bytes(contents)

    if "error" in result:
        return JSONResponse(status_code=400, content={"error": result["error"]})

    # Save image if enabled
    if SAVE_IMAGES and "image" in result:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"{IMAGE_SAVE_PATH}/solar_panel_{timestamp}.jpg"
        cv2.imwrite(filename, result["image"])
    
    # Remove image from response
    if "image" in result:
        del result["image"]
    
    # Send cleaning command to ESP32 if cleaning is required
    cleaning_sent = False
    if result["cleaning_required"]:
        clean_result = send_cleaning_command(ESP32_CLEANER_IP, True)
        cleaning_sent = clean_result.get("success", False)
    
    return {
        "cleaning_required": result["cleaning_required"],
        "dust_percentage": result["dust_percentage"],
        "contrast": result["contrast"],
        "cleaning_command_sent": cleaning_sent
    }

@app.get("/")
async def root():
    return {
        "message": "Solar Panel Dust Detection API",
        "endpoints": {
            "/check-dust": "POST - Upload image for dust detection",
            "/upload-image": "POST - Upload image (simplified response)",
            "/status": "GET - Server status"
        }
    }

@app.get("/status")
async def status():
    return {
        "status": "running",
        "dust_threshold": DUST_THRESHOLD,
        "contrast_threshold": CONTRAST_THRESHOLD,
        "esp32_cleaner_ip": ESP32_CLEANER_IP
    }