# UV LED Curing Box – Control + LED Board

## 📌 System Overview
Two-board setup:

1. **Control Board**  
   - Arduino Nano (brains)  
   - Buttons (4)  
   - OLED 128x32 display (I²C)  
   - Buzzer  
   - Red & Green indicator LEDs  
   - MOSFET driver for UV LED array  
   - BJT driver for fan  
   - Thermistor input  
   - Lid switch input  

2. **LED Board**  
   - 18 × UV LEDs (400–405 nm, 3.2–3.4 V, 20 mA)  
   - Wired as 6 strings of 3 LEDs + 150 Ω resistor each  
   - Strings in parallel  
   - Powered from 12 V  

---

## 📌 Arduino Nano Pin Map

| Pin   | Function                                      |
|-------|-----------------------------------------------|
| D2    | Lid switch (NC → GND when closed, internal pull-up) |
| D3    | MOSFET gate → UV LED array (switched GND)     |
| D4    | Button: ON/OFF                                |
| D5    | Button: PREV                                  |
| D6    | Button: NEXT                                  |
| D7    | Button: SET                                   |
| D8    | Fan driver (BJT base via 1 kΩ resistor)       |
| D9    | Buzzer                                        |
| D11   | Red indicator LED (220 Ω to GND)              |
| D12   | Green indicator LED (220 Ω to GND)            |
| A0    | Thermistor divider input                      |
| A4    | OLED SDA                                      |
| A5    | OLED SCL                                      |
| VIN   | +12 V input                                   |
| 5V    | Logic + OLED + buttons pull-ups               |
| GND   | Common ground                                 |

---

## 📌 Wiring Details

### UV LED Board
- 6 × strings of 3 LEDs in series + 150 Ω resistor  
- All strings in parallel  
- +12 V rail feeds resistors → LEDs → output to Control Board  
- GND of strings connected together → goes back to MOSFET drain on Control Board  

### Fan Driver (BJT)
- NPN transistor (2N2222/BC337/etc.)  
- Fan + → +12 V  
- Fan − → Collector  
- Emitter → GND  
- Base → D8 via 1 kΩ resistor  
- Flyback diode across fan: cathode → +12 V, anode → Collector  

### MOSFET Driver (LEDs)
- N-channel MOSFET (IRLZ44N)  
- Source → GND  
- Drain → LED array GND  
- Gate → D3 (with 100 Ω resistor in series for stability)  
- Pull-down resistor 10 kΩ gate → GND  

### Buttons
- One side → GND  
- Other side → D4–D7 (internal pull-ups enabled in code)  

### Buzzer
- Active buzzer (5 V)  
- + → D9  
- − → GND  
- (Optional 100 Ω series resistor)  

### Thermistor
- 10k NTC + 10k resistor voltage divider  
- NTC between 5 V and A0  
- Resistor between A0 and GND  

### OLED
- I²C (SDA → A4, SCL → A5, +5 V, GND)  

### Indicator LEDs
- Red LED → D11 (through 220 Ω to GND)  
- Green LED → D12 (through 220 Ω to GND)  

---

## 📌 Operating Logic

### Buttons
| Button | Normal Timer Mode | Preset Edit Mode | Settings Menu |
|--------|------------------|------------------|---------------|
| ON/OFF | Start/Stop timer | N/A              | Change value of selected item |
| NEXT   | Next preset / +time (short +1s, long +30s) | Increase preset | Move down list |
| PREV   | Previous preset / -time (short -1s, long -30s) | Decrease preset | Move up list |
| SET    | Enter/exit preset edit mode | Confirm preset | Long press = enter/exit menu |

### Indicators
- **Red LED:** ON = timer running  
- **Green LED:** ON = timer finished, waiting reset  

### Buzzer Patterns
- **Silent** = no beep  
- **Single** = one short beep  
- **Double** = two medium beeps  
- **Triple** = three short beeps  
- **Long** = one long beep  
- **Chirp** = rising “chirp” tone  

### Settings Menu
- Buzzer Pattern: Silent, Single, Double, Triple, Long, Chirp  
- Lid Sensor: Enabled / Disabled  
- Temp Limit: 40–90 °C in 5 °C steps (default 60 °C)  
- Fan Mode: Always On / Timer Only / Off  
- Reset Presets: Restore defaults  

### Safety
- Lid switch: timer pauses if lid open (unless disabled in settings)  
- Thermistor: if LEDs exceed threshold → shut down, display warning  
