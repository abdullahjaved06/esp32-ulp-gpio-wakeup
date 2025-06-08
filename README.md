# ESP32 ULP FSM GPIO Wake-Up

This project demonstrates how to use the **Ultra Low Power (ULP) co-processor** on the **ESP32** to detect a **rising edge signal** on **GPIO0** while the main CPU is in deep sleep. This allows extremely low-power operation for applications that only need to wake on external events.

---

## 🔧 Features

- Uses ULP FSM to monitor RTC-capable GPIO (GPIO0)
- Detects **LOW → HIGH** transitions
- Wakes up the main CPU only when needed
- Sleeps between checks (~200ms polling interval)
- Suitable for battery-powered and energy-efficient designs

---

## 📂 Project Structure

esp32_ulp_gpio_wakeup/
├── main/
│ ├── app_main.c # Main application logic
│ └── ulp/
│ ├── pulse_cnt.S # ULP GPIO polling logic
│ └── wake_up.S # ULP wake-up condition
├── sdkconfig / sdkconfig.defaults
├── CMakeLists.txt
└── README.md


---

## 🧠 How It Works

1. On boot, the main CPU initializes GPIO0 in RTC mode and loads a ULP program.
2. The ULP FSM continuously checks the GPIO state every ~200ms.
3. When a rising edge is detected (LOW → HIGH), the ULP triggers a wake-up.
4. The main CPU handles the event, reinitializes the ULP, and returns to deep sleep.

---

## ⚙️ Setup

1. Connect a digital signal (e.g. button or sensor) to **GPIO0**.
2. Run the following in your ESP-IDF environment:

```bash
idf.py menuconfig
# Enable: Component config → ULP co-processor → Enable ULP FSM support

