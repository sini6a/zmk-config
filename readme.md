# ZMK Config Repository

This repository contains custom **ZMK (Zephyr Mechanical Keyboard)** configuration files for custom built wireless keyboards.

---

## 📌 Boards

Currently supported board(s):

- **Pocketboard Rev. 2.0**  
  - MCU: nRF52840  
  - Features: BLE wireless, battery monitor, LED indicator

---

## ✅ Purpose

This repo holds:
- Board definitions and `*.overlay` files
- Custom keymaps (`keymap.keymap`)
- Build configuration (`prj.conf`)
- Power management tweaks (battery, low-power modes)
- Any custom drivers or modules if needed

---

## 🚀 Build & Flash

Example build:
```bash
west build -b pocketboard_rev2 -s zmk/app
```

Flash:

```bash
west flash --runner nrfutil
```