#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <LiquidCrystal_I2C.h>

/*
  mapping the hardware here so future-you knows where to poke:
  - 20x4 I2C LCD @ 0x27
  - AHT21 (Adafruit AHTX0 lib) @ 0x38
  - Four buttons with EXTERNAL pull-downs (pressed = HIGH): OK/UP/DOWN/CANCEL
  - Relay (active-HIGH) to drive heater/blower
  - Buzzer for UI/error beeps 
*/

// button pins
const int okPin = 3;
const int upPin = 4;
const int downPin = 5;
const int cancelPin = 6;

// relay and buzzer pins
const int relayPin = 7;  // active-HIGH
const int buzzerPin = 9;

// store button states (pressed = HIGH because of external pull-downs)
int cancelState = 0;
int upState = 0;
int downState = 0;
int okState = 0;

// relay and mode states
bool relayState = false;  // true means relay ON (HIGH)
bool holdMode = false;    // becomes true once user timer is up (no hard cutoff)

// menu/profile selection
int selectedProfile = 1;  // currently highlighted profile in menu
int activeProfile = 0;    // currently running profile (0 = none)

// flags to avoid redrawing LCD headers too often
bool runHeaderDrawn = false;
bool homeHeaderDrawn = false;

// sensor and LCD objects
Adafruit_AHTX0 aht;                  // AHT21 sensor
LiquidCrystal_I2C lcd(0x27, 20, 4);  // 20x4 LCD at addr 0x27

// profile parameters (overwritten by loadProfile)
float setTemp = 35.0;  // target temperature (°C)
float setHum = 20.0;   // target humidity (%RH)

// separate hysteresis bands so temp and humidity don’t fight each other
float tempHys = 1.0;  // °C band to avoid chatter
float humHys = 3.0;   // %RH band to avoid chatter

// runtime control (user-visible timer; not a hard cutoff)
unsigned long maxRunTime = 6UL * 60UL * 60UL * 1000UL;  // default 6 hours
unsigned long startTime = 0;

// tiny, simple beep helper (kept everywhere except relay ON/OFF)
void beep(unsigned int durationMs = 120, unsigned int repeat = 1, unsigned int gapMs = 90) {
  for (unsigned int i = 0; i < repeat; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(durationMs);
    digitalWrite(buzzerPin, LOW);
    if (i < repeat - 1) delay(gapMs);
  }
}

// map profile ID -> filament name
const char* profileName(int id) {
  if (id == 1) return "PLA";
  if (id == 2) return "PETG/CPE/PCTG";
  if (id == 3) return "ABS";
  if (id == 4) return "ASA";
  if (id == 5) return "TPU";
  if (id == 6) return "Nylon";
  if (id == 7) return "Polycarbonate";
  if (id == 8) return "PP";
  return "Unknown";
}

// load profile parameters for selected filament
void loadProfile(int id) {
  if (id == 1) {
    setTemp = 45;
    setHum = 10;
    maxRunTime = 6UL * 60 * 60 * 1000UL;
  }  // PLA typical limit ~45C
  if (id == 2) {
    setTemp = 68;
    setHum = 10;
    maxRunTime = 5UL * 60 * 60 * 1000UL;
  }
  if (id == 3) {
    setTemp = 75;
    setHum = 10;
    maxRunTime = 3UL * 60 * 60 * 1000UL;
  }
  if (id == 4) {
    setTemp = 75;
    setHum = 10;
    maxRunTime = 3UL * 60 * 60 * 1000UL;
  }
  if (id == 5) {
    setTemp = 52;
    setHum = 10;
    maxRunTime = 7UL * 60 * 60 * 1000UL;
  }
  if (id == 6) {
    setTemp = 80;
    setHum = 5;
    maxRunTime = 12UL * 60 * 60 * 1000UL;
  }
  if (id == 7) {
    setTemp = 95;
    setHum = 5;
    maxRunTime = 5UL * 60 * 60 * 1000UL;
  }
  if (id == 8) {
    setTemp = 65;
    setHum = 10;
    maxRunTime = 5UL * 60 * 60 * 1000UL;
  }
}

// one-time sensor check — in setup()
void checkSensorOnceOrDie() {
  if (!aht.begin()) {
    Serial.println("ERROR: AHT21 not found at 0x38");
    lcd.clear();
    lcd.setCursor(7, 0);
    lcd.print("ERROR:");
    lcd.setCursor(2, 1);
    lcd.print("AHT21 not found");
    while (1) {                     // lock out
      digitalWrite(relayPin, LOW);  // be safe
      beep(300, 1, 0);
      delay(300);
    }
  }
}

// --- LCD display helpers ---

void showHomeHeader() {
  lcd.clear();
  lcd.setCursor(7, 0);
  lcd.print("Toasty");
  lcd.setCursor(3, 1);
  lcd.print("Filament Dryer");
  lcd.setCursor(1, 3);
  lcd.print("Press OK for menu");
}

void showHomeTH() {
  sensors_event_t humEvt, tmpEvt;
  aht.getEvent(&humEvt, &tmpEvt);
  float t = tmpEvt.temperature;
  float h = humEvt.relative_humidity;
  lcd.setCursor(2, 2);
  lcd.print("T:");
  lcd.print(t, 1);
  lcd.print((char)223);
  lcd.print("C  ");
  lcd.print("H:");
  lcd.print(h, 0);
  lcd.print("%  ");
}

void showProfile(int id) {
  loadProfile(id);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Profile: ");
  lcd.print(profileName(id));
  lcd.setCursor(0, 1);
  lcd.print("Set T:");
  lcd.print(setTemp, 1);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(14, 1);
  lcd.print("H:");
  lcd.print(setHum, 0);
  lcd.print("%");
  lcd.setCursor(0, 2);
  lcd.print("Time:");
  lcd.print(maxRunTime / 3600000UL);
  lcd.print("h");
}

void showRunHeader(int id) {
  loadProfile(id);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Running: ");
  lcd.print(profileName(id));
  lcd.setCursor(0, 1);
  lcd.print("Set T:");
  lcd.print(setTemp, 1);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(14, 1);
  lcd.print("H:");
  lcd.print(setHum, 0);
  lcd.print("%");
}

void showTempHumRow(float currentT, float currentH, bool relayIsOn, unsigned long remainingMs) {
  unsigned long hh = remainingMs / 3600000UL;
  unsigned long mm = (remainingMs % 3600000UL) / 60000UL;

  lcd.setCursor(0, 2);
  lcd.print("T:");
  lcd.print(currentT, 1);
  lcd.print((char)223);
  lcd.print("C  ");
  lcd.print("H:");
  lcd.print(currentH, 0);
  lcd.print("%   ");

  lcd.setCursor(0, 3);
  lcd.print("Left ");
  if (hh < 10) lcd.print("0");
  lcd.print(hh);
  lcd.print(":");
  if (mm < 10) lcd.print("0");
  lcd.print(mm);
  lcd.print(" Relay:");
  lcd.print(relayIsOn ? "ON " : "OFF");
}

// run active drying profile
void runProfile(int id) {
  if (!runHeaderDrawn) {
    showRunHeader(id);
    runHeaderDrawn = true;
  }

  // read sensor (no re-begin here)
  sensors_event_t humEvt, tmpEvt;
  aht.getEvent(&humEvt, &tmpEvt);
  float t = tmpEvt.temperature;
  float h = humEvt.relative_humidity;

  if (isnan(t) || isnan(h)) {
    Serial.println("Sensor reading invalid -> forcing relay OFF.");
    digitalWrite(relayPin, LOW);
    relayState = false;
    lcd.setCursor(0, 3);
    lcd.print("Sensor ERR           ");
    beep(400);
    delay(400);
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" C");
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %RH");


  

  // --------- SIMPLE, SEPARATE HYSTERESIS CONTROL ----------
  // Relay OFF if: (t >= setTemp)  OR  (h <= setHum)          // safety-first
  // Relay ON  if: (t <= setTemp - tempHys) AND (h >= setHum + humHys)

  if ((t >= setTemp) || (h <= setHum)) {
    relayState = false;
    digitalWrite(relayPin, LOW);  // active-HIGH -> LOW = OFF
    Serial.println("Relay OFF (temp >= setpoint OR humidity <= target)");
  } else if ((t <= (setTemp - tempHys)) && (h >= (setHum + humHys))) {
    relayState = true;
    digitalWrite(relayPin, HIGH);  // active-HIGH -> HIGH = ON
    Serial.println("Relay ON (temp below band AND humidity above band)");
  }





  unsigned long elapsed = millis() - startTime;
  unsigned long remaining = (elapsed >= maxRunTime) ? 0UL : (maxRunTime - elapsed);

  showTempHumRow(t, h, relayState, remaining);

  // user-visible timer only (don’t kill relay); just enter HOLD once
  bool done = (elapsed >= maxRunTime);
  if (done && !holdMode) {
    holdMode = true;
    lcd.setCursor(0, 0);
    lcd.print("Done (Hold): ");
    lcd.print(profileName(id));
    beep(150, 2, 120);  // beep unique
    Serial.println("Profile timer done -> HOLD (relay continues as needed)");
  }
}

// home screen handler
void showHome() {
  static unsigned long lastRefresh = 0;
  const unsigned long REFRESH_MS = 1500;
  if (!homeHeaderDrawn) {
    showHomeHeader();
    homeHeaderDrawn = true;
  }
  if ((millis() - lastRefresh) >= REFRESH_MS) {
    showHomeTH();
    lastRefresh = millis();
  }
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  pinMode(relayPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  //  all button pins are external-pull down
  pinMode(cancelPin, INPUT);
  pinMode(upPin, INPUT);
  pinMode(downPin, INPUT);
  pinMode(okPin, INPUT);

  digitalWrite(relayPin, LOW);  // make sure heater is OFF on boot
  digitalWrite(buzzerPin, LOW);

  checkSensorOnceOrDie();  // init sensor once, fail-fast if missing

  Serial.println("AHT21 initialized successfully.");
  Serial.println("Toasty - Filament Dryer");

  lcd.setCursor(7, 0);
  lcd.print("Toasty");
  lcd.setCursor(3, 1);
  lcd.print("Filament Dryer");
  lcd.setCursor(4, 3);
  lcd.print("Initializing...");
  delay(2500);

  lcd.clear();
  startTime = millis();
  homeHeaderDrawn = false;
}

void loop() {
  // read button states (pressed = HIGH with external pull-downs)
  cancelState = digitalRead(cancelPin);
  upState = digitalRead(upPin);
  downState = digitalRead(downPin);
  okState = digitalRead(okPin);

  // home screen when nothing is running
  if (activeProfile == 0) {
    showHome();
    digitalWrite(relayPin, LOW);  // idle-safe
  }

  // enter menu
  if (activeProfile == 0 && okState == HIGH) {
    beep();
    showProfile(selectedProfile);
    digitalWrite(relayPin, LOW);
    delay(150);  // small debounce/UX delay

    // stay in menu until CANCEL is pressed or OK starts the run
    while (digitalRead(cancelPin) == LOW) {
      upState = digitalRead(upPin);
      downState = digitalRead(downPin);
      okState = digitalRead(okPin);

      if (downState == HIGH) {
        selectedProfile++;
        if (selectedProfile > 8) selectedProfile = 1;
        beep(80);
        showProfile(selectedProfile);
        delay(150);
      }

      if (upState == HIGH) {
        selectedProfile--;
        if (selectedProfile < 1) selectedProfile = 8;
        beep(80);
        showProfile(selectedProfile);
        delay(150);
      }

      if (okState == HIGH) {
        activeProfile = selectedProfile;
        startTime = millis();
        holdMode = false;
        runHeaderDrawn = false;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Starting: ");
        lcd.print(profileName(activeProfile));
        beep(160, 2, 140);
        delay(700);
        break;
      }
      delay(10);  // tiny poll delay
    }

    // if user CANCELLED out of menu
    cancelState = digitalRead(cancelPin);
    if (activeProfile == 0 && cancelState == HIGH) {
      relayState = false;
      digitalWrite(relayPin, LOW);
      runHeaderDrawn = false;
      homeHeaderDrawn = false;
      lcd.clear();
      lcd.setCursor(5, 1);
      lcd.print("Exit Menu");
      beep(140, 3, 120);
      delay(600);
      lcd.clear();
    }
  }

  // if a profile is running, control the dryer
  if (activeProfile > 0) {
    runProfile(activeProfile);

    // allow user to stop anytime
    cancelState = digitalRead(cancelPin);
    if (cancelState == HIGH) {
      relayState = false;
      digitalWrite(relayPin, LOW);
      activeProfile = 0;
      holdMode = false;
      runHeaderDrawn = false;
      homeHeaderDrawn = false;
      lcd.clear();
      lcd.setCursor(6, 1);
      lcd.print("Stopped");
      beep(140, 3, 120);
      delay(600);
      lcd.clear();
    }
  }
}
