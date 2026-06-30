<p align="center">
  <img src="device images/hero-img.jpg" alt="Toasty Filament Dryer" width="250">
</p>

<h1 align="center">Toasty Filament Dryer</h1>
<h3 align="center">An Arduino powered 3D printer filament drying controller</h3>

## Description

**Toasty** is a DIY, Arduino Nano–based controller that turns a household **220 V AC food dehydrator** into a reliable **3D-printer filament dryer**.
It uses an **AHT21 temperature/humidity sensor**, a **20×4 I²C LCD**, **four push buttons**, a **5 V active-HIGH relay**, and a **5 V buzzer**. The firmware includes preset **temperature/humidity targets** for common filaments and a **Hold Mode** that prevents re-moisture after the timer ends.

---

## 🎯 Objective

* Deliver a **low-cost, beginner-friendly** controller for consistent filament drying.
* Keep material **within safe temperature** while reducing humidity.
* Provide **ready-made profiles** for popular materials and allow easy tweaks.
* Support **continuous drying/storage** for up to **3 spools** inside a modified dehydrator.

---

## Benefits

* Reduces **stringing, bubbles, and warping** by removing moisture in Hygroscopic filaments.
* Extends the life of engineering filaments.
* Reuses a **modified 220 V AC dehydrator** as the drying chamber.
* Controller runs on a **safe 5 V/2 A DC** supply, isolated from mains.
* **Hold Mode** keeps air moving after the timer to avoid re-wetting.
* Buildable with **intermediate → beginner** electronics skills.

---
  

## Limitations

- This tool is designed for **standard 3D printing filaments**. Some filament manufacturers use proprietary blends or modified materials, so the actual properties may differ from the standard material behavior. Results should be treated as estimates.


- **Hydrolysis** is a moisture-driven chemical degradation process where water breaks down the polymer chains of filaments. Unlike simple drying, hydrolysis damage is usually permanent and cannot be fully reversed.

- **Filament crazing / brittleness** occurs when filament absorbs moisture, ages, or is exposed to unsuitable environmental conditions. The material becomes stiff and fragile, and may crack or snap easily. This often happens because the filament retains stress from its original coiled shape and develops weak points when straightened during feeding into the printer.

- **Drying vs Annealing:** If the filament condition is unknown, annealing may sometimes be considered instead of only drying. However, annealing and drying solve different problems:
  - **Drying** removes absorbed moisture from the filament.
  - **Annealing** changes the internal structure of the printed part by applying controlled heat, improving properties such as strength and heat resistance.

  Choose the correct process based on the filament type and its condition.
---



## Features

* **8 material profiles** (PLA, PETG/CPE/PCTG, ABS, ASA, TPU, Nylon, PC, PP).
* **20×4 I²C LCD** for better user interactions.
* **Four buttons** (OK / UP / DOWN / CANCEL) with **external pull-downs** (pressed = HIGH).
* **Relay** control for mains 
* **Separate hysteresis** for temperature and humidity.
* **Timer on Screen**; when it ends, system enters **Hold Mode**.
* **Simple UI** with buzzer feedback for each click for UI/errors).

---

## Hardware Overview

**Core:** Arduino Nano
**Sensor:** AHT21 (Adafruit AHTX0) @ **I²C 0x38**
**LCD:** 20×4 I²C @ **0x27**
**Relay Module** 
**Power Supply** **5 V/2 A DC**
**Food dehydrator** ( heater/fan )

---


### Pin Mapping

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

## Bill of Materials (BOM)

* Arduino Nano (or compatible)
* AHT21 sensor module (Adafruit AHTX0 compatible)
* 20×4 I²C LCD @ 0x27
* 4 × momentary push buttons + external pull-down resistors
* 5 V **active-HIGH** relay module (mains-rated, proper isolation)
* 5 V buzzer
* 5 V/2 A DC power supply (controller board)
* Modified **220 V AC** food dehydrator (drying chamber)
* Enclosure, wiring, terminal blocks, heat-shrink, etc.


* 🔧 **Modification Instructions:**  
  Watch the tutorial video by **Christopher Lum** for guidance on physical modification in the food dehydrator setup.  

  >  [YouTube – Christopher Lum: Turning a Food Dehydrator into a Filament Dryer](https://www.youtube.com/watch?v=nidkPN12M4I)) 

---

##  Supported Filament Profiles

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

**Hysteresis (defaults):** `tempHys = 1.5 °C`, `humHys = 3 %RH`

> Tune inside `loadProfile(id)` or via the globals near the top of the sketch.

---

## ⚙️ How It Works

Hot air increases the evaporation of water from the filament. Moving air carries the water vapor away, keeping the surrounding air from becoming saturated. This maintains a low water vapor partial pressure (or low relative humidity around the filament), allowing more moisture to diffuse out of the filament.

1. **Boot & Sensor Check** — Initializes AHT21 and LCD. If the sensor is missing, it shows a centered **ERROR: AHT21 not found**, beeps, and locks out with the relay OFF(NO state ). Prevents turning on the Food Dehydrater for Safety.
2. **Home Screen** — Shows “Toasty Filament Dryer” and live **T/H** readings.
3. **Profile Menu** — Press **OK** to open the selector (1–8). Navigate with **UP/DOWN**, **OK** to start, **CANCEL** to exit.
4. **Run** — Control loop maintains setpoints using independent hysteresis for temperature and humidity.
5. **Timer** — A user-visible countdown runs (it does not hard-stop the run).
6. **Hold Mode** — When the timer ends, the display shows **Done (Hold)** and the relay continues as required to prevent re-moisture.
7. **Stop Anytime** — Press **CANCEL** to stop; relay turns OFF and the system returns to Home.

---

##  Control Structure

* **Relay ON** when
  `t ≤ setTemp − tempHys` **AND** `h ≥ setHum + humHys`

* **Relay OFF** when
  `t ≥ setTemp` **OR** `h ≤ setHum`

* Otherwise (between bands), the relay **holds its last state**

---

##  Software

**Language:** Arduino C++
**Libraries:**

```
Wire
Adafruit_AHTX0
LiquidCrystal_I2C
```

### Build & Flash

1. Install libraries via Arduino Library Manager.
2. Select **Arduino Nano (ATmega328P)** in the IDE.
3. Open the provided `main.ino` file.
4. Compile & Upload.

### Customization

* Edit `loadProfile(id)` to tune **Set Temp**, **Target RH**, and **Max Run Time**.
* Adjust `tempHys` / `humHys` for responsiveness vs. stability.
* UI strings live in `showHomeHeader`, `showRunHeader`, etc.
* Error messages are pre-centered for a **20×4** LCD.

---

##  Circuit Schematic

<img width="570" height="400" alt="image" src="https://github.com/user-attachments/assets/ad3ba50e-b9e1-4646-bb30-19890a5fa1a0" />


##  Images

<img width="420" height="400" alt="image" src="https://github.com/user-attachments/assets/51e9b04e-c7ac-436f-8f6b-b34c47683573" />
<img width="450" height="370" alt="image" src="https://github.com/user-attachments/assets/a28cde18-25f6-4d38-bafe-fa18de0a5f75" />




---

##  Operation Tips

* **PLA:** Default **Set Temp = 45 °C**. If your unit overshoots, reduce the setpoint or improve airflow.
* **Nylon/PC:** High temps—confirm your dehydrator/enclosure can handle them safely.
* **Multi-spool storage:** Keep **Hold Mode** active between prints to prevent re-wetting.

---

##  Troubleshooting

* **AHT21 not found:** Check I²C wiring (SDA=A4, SCL=A5), address, power, and module pull-ups.
* **Relay always ON/OFF:** Verify **active-HIGH** wiring, pin **D7**, and thresholds.
* **LCD blank/garbled:** Confirm I²C address **0x27** and constructor `(0x27, 20, 4)`.
* **Buttons inverted:** Buttons must use **external pull-downs**; pressed = **HIGH**.

---

##  Safety Note (Mains)

* The dehydrator’s heater/fan is **220 V AC**. Disconnect power before wiring.
* Enclose all mains parts; insulate and strain-relieve cables.
* Use a relay module **rated** for your load with proper isolation/clearances.
* Keep low-voltage and mains wiring **physically separated**.
* Only load one filament type at a time—the type selected on the screen (e.g., High Temp or Low Temp). Mixing filament types can damage low-temperature filament due to excess heat.
* If unsure, consult a qualified person.


---

## File Download

###  Circuit Diagram / Schematics  

📁 **File Path:** [`Open`](Circuit_Schematic.pdf)  

##  Arduino Code  

📁 **File Path:** [`Open`](main.ino)

---



## Reporting Issues & Contributions

Open **issues** and send **PRs** to improve docs, Temp profiles, and wiring notes.

And feel free to give a **Star** to the repo...

---

##  License & Credits

Copyright © 2025 Abhijeet Kumar.
Licensed under **CC0-1.0** [`LICENSE`](LICENSE)

Hardware/firmware by **Toasty** project. Library credits: **Adafruit AHTX0**, **LiquidCrystal_I2C**.


