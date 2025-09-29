
# Toasty-Filament-Dryer

### Toasty — Arduino-Powered Filament Dryer Controller

### Description

**Toasty** is a DIY, Arduino Nano–based controller that converts a household **220 V AC food dehydrator** into a reliable **3D-printer filament dryer**.
It uses an **AHT21 humidity/temperature sensor**, a **20×4 I²C LCD**, **push buttons**, a **5 V active-HIGH relay**, and a **5 V buzzer**. The firmware provides **preset temperature/humidity targets and timers** for common filaments and a **Hold Mode** that prevents re-moisture after the timer ends.

---

## 🎯 Objective

* Create a **low-cost, beginner-friendly** controller for consistent filament drying.
* Keep material **below glass-transition** limits with simple safety logic.
* Offer **ready-made profiles** for common materials and allow easy customization.
* Support **continuous drying/storage** for up to **3 spools** inside a modified dehydrator.

---

## ✅ Benefits

* Reduces **stringing, bubbles, and warping** by driving out moisture.
* Extends the life of premium engineering filaments.
* Uses a **modified 220 V AC dehydrator** for the drying chamber.
* Controller runs on a **safe 5 V/2 A DC supply**, isolated from mains.
* **Hold Mode** keeps air moving after the timer to avoid re-wetting.
* Buildable with **intermediate → beginner** electronics skills.

---

## ✨ Features

* **8 built-in material profiles** (PLA, PETG/CPE/PCTG, ABS, ASA, TPU, Nylon, PC, PP).
* **20×4 I²C LCD** status UI with center-aligned error messages.
* **Four buttons** (OK / UP / DOWN / CANCEL) with **external pull-downs** (pressed = HIGH).
* **Active-HIGH relay** (no beep on relay toggles; buzzer only for UI/errors).
* **One-time sensor check** at boot—fails safe if AHT21 is missing.
* **Separate hysteresis** for temperature/humidity to reduce relay chatter.
* **Timer is user-visible only**; when it ends, system enters **Hold Mode** (relay continues as needed).

---

## 🔌 Hardware Overview

**Core:** Arduino Nano
**Sensor:** AHT21 (Adafruit AHTX0 library) @ **I²C 0x38**
**LCD:** 20×4 I²C @ **0x27**
**Power:** 5 V/2 A DC (controller) + **220 V AC** (dehydrator heater/fan through relay)

### Pin Mapping (per code)

| Function      | Pin   | Notes                                  |
| ------------- | ----- | -------------------------------------- |
| OK Button     | D3    | External pull-down; pressed = **HIGH** |
| UP Button     | D4    | External pull-down; pressed = **HIGH** |
| DOWN Button   | D5    | External pull-down; pressed = **HIGH** |
| CANCEL        | D6    | External pull-down; pressed = **HIGH** |
| Relay Out     | D7    | **Active-HIGH** (HIGH = ON)            |
| Buzzer        | D9    | UI/error beeps only                    |
| I²C (LCD+AHT) | A4/A5 | SDA = A4, SCL = A5 (Arduino Nano)      |

> **Mains note:** The relay must switch the **220 V AC** line to the dehydrator’s heater/fan. Keep low-voltage and mains wiring **well isolated**.

---

## 🧰 Bill of Materials (BOM)

* Arduino Nano (or compatible)
* AHT21 sensor module (Adafruit AHTX0 compatible)
* 20×4 I²C LCD @ 0x27
* 4 × momentary push buttons + external pull-down resistors
* 5 V active-HIGH relay module (mains-rated, with proper isolation)
* 5 V buzzer
* 5 V/2 A DC power supply (for controller board)
* Modified **220 V AC food dehydrator** (drying chamber)
* Enclosure, wiring, terminal blocks, heat-shrink, etc.

---

## 📋 Supported Filament Profile

| ID | Filament           | Set Temp (°C) | Target RH (%) | Max Run Time |
| -- | ------------------ | ------------- | ------------- | ------------ |
| 1  | PLA                | **45**        | **10**        | **6 h**      |
| 2  | PETG / CPE / PCTG  | **68**        | **10**        | **5 h**      |
| 3  | ABS                | **75**        | **10**        | **3 h**      |
| 4  | ASA                | **75**        | **10**        | **3 h**      |
| 5  | TPU                | **52**        | **10**        | **7 h**      |
| 6  | Nylon              | **80**        | **5**         | **12 h**     |
| 7  | Polycarbonate (PC) | **95**        | **5**         | **5 h**      |
| 8  | Polypropylene (PP) | **65**        | **10**        | **5 h**      |

**Hysteresis (in code):** `tempHys = 1.5 °C`, `humHys = 3 %RH`
**Ceiling rule:** Temperature **never allowed to cross** `Set Temp`.

> You can adjust these in `loadProfile(id)`.

---

## ⚙️ How It Works (High level)

1. **Boot & Sensor Check:** On startup, code initializes the AHT21 and LCD. If the sensor is missing, it shows a **ERROR → “AHT21 not found”**, beeps, and **locks out** with the relay forced OFF.
2. **Home Screen:** Shows “Toasty Filament Dryer” and live **T/H** readings.
3. **Profile Menu:** Press **OK** to open the profile selector (1–8). Navigate with **UP/DOWN**, **OK** to start, **CANCEL** to exit.
4. **Run:** Control loop tries to reach and maintain **Set Temp** and **Target RH**, using **separate hysteresis** and a **hard temperature ceiling**.
5. **Timer:** A **user-visible countdown** runs (does **not** hard-stop). Remaining time and relay state are shown.
6. **Hold Mode:** When the timer reaches zero, the system enters **Done (Hold)**; **relay continues to operate** as needed to prevent re-moisture (no automatic cutoff).
7. **Stop Anytime:** Press **CANCEL** to stop, relay turns OFF, and the system returns to Home.

---

## 🔄 Control Structure (matches the code)

* **Inputs:** AHT21 temperature/humidity, four buttons with external pull-downs.
* **Outputs:** Relay (active-HIGH), Buzzer, 20×4 LCD.
* **Decision Logic:**

  * **Ceiling:** If `t ≥ setTemp` → **force relay OFF**.
  * **Turn ON** when `(t < setTemp - tempHys) OR (h > setHum + humHys)` **and** ceiling not hit.
  * **Turn OFF** when `(t ≥ setTemp - tempHys AND h ≤ setHum + humHys)` **or** ceiling hit.
* **Timer:** `maxRunTime` per profile; on expiry → **Hold Mode** (no hard cutoff).
* **Beep policy:** No beeps on relay ON/OFF; only for UI events and errors.

---

## 🖥️ Software (this repo)

**Language:** Arduino C++
**Libraries:**

```text
Wire
Adafruit_AHTX0
LiquidCrystal_I2C
```

### Build & Flash

1. Install libraries via Arduino Library Manager.
2. Select **Arduino Nano** (ATmega328P) in the IDE.
3. Open the provided `.ino` file.
4. Compile & Upload.

### Customization

* Edit `loadProfile(id)` to tune **Set Temp**, **Target RH**, and **Max Run Time**.
* Adjust `tempHys` / `humHys` to change responsiveness vs. relay chatter.
* UI text is in helper functions (`showHomeHeader`, `showRunHeader`, etc.).
* Error messages are pre-centered for a 20×4 LCD.

---

## 📐 Circuit Schematic

> Add your schematic image (PNG/JPG) here and reference nets for: **5 V/2 A DC**, **Relay IN**, **LCD I²C @ 0x27**, **AHT21 I²C @ 0x38**, **Buttons with external pull-downs**, **Buzzer**.
> Clearly separate **low-voltage** and **220 V AC** sections.

---

## 📸 Images

> Add photos of the assembled controller, dehydrator integration, LCD screens (Home, Menu, Run, Done/Hold), and wiring.

---

## 🧪 Operation Tips

* **PLA safety:** Code sets **45 °C** ceiling; you can lower it if your dryer overshoots.
* **Nylon/PC:** Higher temps; ensure your dehydrator and enclosure can handle it.
* **Three-spool storage:** Keep dryer in **Hold Mode** between prints to avoid re-wetting.

---

## 🛠️ Troubleshooting

* **AHT21 not found:** Check I²C wiring (SDA=A4, SCL=A5), addresses, power, and pull-ups on the module.
* **Relay always ON/OFF:** Verify **active-HIGH** wiring and that the firmware pin (D7) matches your board.
* **LCD garbage/blank:** Confirm I²C address (**0x27**) and 20×4 constructor `(0x27, 20, 4)`.
* **Buttons inverted:** You **must** use **external pull-downs**; pressed must read **HIGH**.

---

## ⚠️ Safety Note (Mains)

* The dehydrator’s heater/fan is **220 V AC**. Disconnect power before wiring.
* Enclose all mains parts; insulate and strain-relieve cables.
* Use a relay module **rated** for your load with proper isolation/clearances.
* Keep low-voltage and mains wiring **physically separated**.
* If unsure, consult a qualified person.

---


## Reporting Issues & Contributions

Feel free to report <b>[issues](https://github.com/Abhijeetbyte/Toasty-Filament-Dryer/issues/new)</b> and <b>contribute</b> to this repository


## 📜 License & Credits


## License

Copyright © 2025 Abhijeet kumar. All rights reserved.

Licensed under the [CC0-1.0 License](LICENSE).

* Hardware/firmware by **Toasty** project. Sensor/library credit: **Adafruit AHTX0**, **LiquidCrystal_I2C** maintainers.

---





