# Offline Event Check-in Terminal

A lightweight offline QR/barcode validation system built in C++ for fast and reliable event gate check-ins on Windows devices.

---

## Features
- Offline QR validation  
- Instant success/failure feedback  
- Fullscreen terminal interface  
- Persistent attendee counting  
- USB QR scanner compatible  
- Audio and visual confirmation system  

---

## How It Works
1. The operator scans a QR code using a USB scanner.  
2. The scanner sends the encoded string to the application.  
3. The application validates the scanned value against the configured secure token.  
4. If valid:
   - attendee is admitted  
   - count increases  
   - green success screen and confirmation beep appear  
5. If invalid:
   - red rejection screen and warning beep appear  

---

## QR Format

Example QR content:

WHEATON INTERNATIONAL SCHOOL -EVENT

---

## Files

| File | Purpose |
|------|---------|
| checkin.exe | Main application |
| count.txt | Stores admitted attendee count |

---

## Requirements
- Windows PC or laptop  
- USB QR/barcode scanner  

---

## Notes
- Works completely offline  
- Automatically saves attendee count after every successful scan  
- Designed for fast event entry processing with minimal operator interaction  
