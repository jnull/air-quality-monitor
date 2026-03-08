# Air Quality Monitor

ESP32-C6 · Zephyr RTOS · LVGL 8 · SGP30 · ILI9341 · OpenThread → Home Assistant

---

## Table of Contents

1. [Hardware Overview](#1-hardware-overview)  
2. [Pin Assignment & Wiring Notes](#2-pin-assignment--wiring-notes)  
3. [Project Layout](#3-project-layout)  
4. [Phase 1 — Display & Sensor](#4-phase-1--display--sensor)  
   4.1 [Prerequisites](#41-prerequisites)  
   4.2 [Workspace Setup](#42-workspace-setup)  
   4.3 [Build & Flash](#43-build--flash)  
   4.4 [Console (USB CDC-ACM)](#44-console-usb-cdc-acm)  
   4.5 [Touch Calibration](#45-touch-calibration)  
5. [Phase 2 — Thread + Home Assistant + OTA](#5-phase-2--thread--home-assistant--ota)  
   5.1 [OpenThread on ESP32-C6](#51-openthread-on-esp32-c6)  
   5.2 [Connecting to Home Assistant](#52-connecting-to-home-assistant)  
   5.3 [OTA Firmware Updates (MCUboot)](#53-ota-firmware-updates-mcuboot)  
6. [EEZ Studio UI Design](#6-eez-studio-ui-design)  
7. [Troubleshooting](#7-troubleshooting)  
8. [References](#8-references)  

---

## 1. Hardware Overview

| Component      | Details                                                   |
| -------------- | --------------------------------------------------------- |
| **MCU**        | ESP32-C6-DevKitC-1 v1.2 (ESP32-C6-WROOM-1, 8 MB flash)    |
| **Display**    | 2.8" SPI TFT, ILI9341 driver, 320×240 px, RGB565          |
| **Touch**      | XPT2046 resistive (on-board, shared SPI bus with display) |
| **Air sensor** | Adafruit SGP30 — eCO2 (ppm) + TVOC (ppb) over I2C         |

The ESP32-C6 supports Wi-Fi 6, Bluetooth 5, **IEEE 802.15.4 (Thread 1.3 / Zigbee 3.0)** — making it a first-class Thread device without any extra hardware.

---

## 2. Pin Assignment & Wiring Notes

```
ESP32-C6 GPIO  │ Display / Touch (MSP2807)      │ SGP30
───────────────┼────────────────────────────────┼──────────
GPIO6          │ SCK  (display + touch shared)  │
GPIO7          │ MOSI (display + touch shared)  │
GPIO2          │ MISO (display + touch shared)  │
GPIO8          │ DC/RS                          │
GPIO10         │ RESET                          │
GPIO11         │ LED  (backlight, 3.3V logic)   │
GPIO16         │ CS   (display chip select)     │
GPIO17         │ T_CS (touch chip select)       │
GPIO21         │ T_IRQ (touch interrupt)        │
GPIO22         │                                │ SDA
GPIO23         │                                │ SCL
3V3            │ VCC                            │ VIN
GND            │ GND                            │ GND
```

### ⚠️ Important GPIO conflicts on the DevKit board

| GPIO       | DevKit function            | This project  | Resolution                                               |
| ---------- | -------------------------- | ------------- | -------------------------------------------------------- |
| **GPIO8**  | WS2812 RGB LED             | ILI9341 DC/RS | `&led0` disabled in overlay; on-board LED non-functional |
| **GPIO16** | UART0 TX (USB-UART bridge) | ILI9341 CS    | UART0 disabled; use USB CDC-ACM for console              |
| **GPIO17** | UART0 RX (USB-UART bridge) | XPT2046 T_CS  | Same as above                                            |

**Console connection:** After flashing, connect via the **ESP32-C6 native USB port** (the USB-C labeled for the chip itself, not the USB-UART port). The device appears as a CDC-ACM serial port (e.g. `/dev/ttyACM0` on Linux, `COMx` on Windows).

---

## 3. Project Layout

```
air-quality-monitor/          ← west manifest repository (also the app)
├── west.yml                  ← workspace manifest (pins Zephyr v3.7.0)
├── CMakeLists.txt
├── Kconfig                   ← app-level Kconfig symbols
├── prj.conf                  ← Zephyr configuration
├── boards/
│   └── esp32c6_devkitc.overlay  ← hardware description (DTS overlay)
├── src/
│   ├── main.c
│   ├── ui/
│   │   ├── ui.c              ← LVGL screen, gauges, update logic
│   │   └── ui.h
│   └── sensors/
│       ├── sgp30.c           ← sensor reader thread
│       └── sgp30.h
└── README.md
```

---

## 4. Phase 1 — Display & Sensor

### 4.1 Prerequisites

- A working Zephyr v3.7.0 workspace (you have this ✓)
- `west`, `cmake ≥ 3.20`, `python ≥ 3.8`, `ninja`
- ESP32 toolchain: `xtensa-esp32-elf` is **not** needed; ESP32-C6 uses RISC-V:
  ```
  west sdk install riscv64-zephyr-elf
  ```
  Or ensure `ZEPHYR_SDK_INSTALL_DIR` points to a Zephyr SDK that includes
  the RISC-V toolchain (SDK ≥ 0.16).
- `esptool.py` for flashing (installed automatically by the Zephyr SDK).

### 4.2 Workspace Setup

If you are adding this app to an **existing** workspace:

```bash
# From your workspace root, clone this repo alongside zephyr/
git clone https://github.com/your-org/air-quality-monitor

# No 'west update' needed if Zephyr is already fetched at v3.7.0.
# If you need to update:
west update
```

If you are creating a **new** standalone workspace:

```bash
west init -m https://github.com/your-org/air-quality-monitor --mr main my-workspace
cd my-workspace
west update
```

### 4.3 Build & Flash

```bash
# From workspace root:
west build -b esp32c6_devkitc air-quality-monitor \
  --build-dir build/air-quality-monitor

# Flash via native USB (put board in download mode: hold BOOT, press RESET):
west flash --build-dir build/air-quality-monitor
```

> **Tip — Zephyr ≥ 3.7 board target:** If your Zephyr version requires the
> qualified target syntax, use `-b esp32c6_devkitc/esp32c6` instead.

#### Build with menuconfig (optional)

```bash
west build -b esp32c6_devkitc air-quality-monitor -t menuconfig
```

Key symbols to look for:
- `APP_SGP30_WARMUP_S` — sensor warmup time (default 15 s)
- `APP_SENSOR_POLL_MS` — polling interval (must stay ≥ 1000)
- `APP_UI_REFRESH_MS`  — LVGL tick rate (default 100 ms = 10 FPS)

### 4.4 Console (USB CDC-ACM)

After flashing:

```bash
# Linux / macOS
screen /dev/ttyACM0 115200
# or
minicom -D /dev/ttyACM0 -b 115200

# Windows — use PuTTY or Tera Term on the COMx port that appears
```

You should see output like:
```
[00:00:00.012] <inf> main: === Air Quality Monitor booting ===
[00:00:02.015] <inf> main: USB CDC-ACM console ready
[00:00:02.016] <inf> main: Display ready: ili9341@0
[00:00:02.017] <inf> main: Display backlight ON
[00:00:02.050] <inf> main: UI ready
[00:00:02.051] <inf> sgp30_reader: SGP30 found: sgp30@58
[00:00:02.052] <inf> sgp30_reader: SGP30 reader thread started. Warmup: 15 s
[00:00:02.053] <inf> main: Entering main loop
```

### 4.5 Touch Calibration

The XPT2046 driver uses raw ADC min/max values that depend on the physical
panel. The overlay defaults are reasonable starting values. To calibrate:

1. Enable `CONFIG_XPT2046_CALIBRATE=y` in `prj.conf` (if available in your
   Zephyr version) or write a small calibration sketch.
2. Touch the four screen corners and record the raw X/Y ADC values reported
   in the log.
3. Update `min-x`, `max-x`, `min-y`, `max-y` in the overlay accordingly.

---

## 5. Phase 2 — Thread + Home Assistant + OTA

### 5.1 OpenThread on ESP32-C6

The ESP32-C6 contains an IEEE 802.15.4 radio alongside its Wi-Fi/BT radio.
Zephyr supports this via the `ieee802154_esp32` driver.

#### Kconfig additions (`prj.conf`)

Uncomment the Phase 2 block in `prj.conf`:

```kconfig
CONFIG_NET_L2_IEEE802154=y
CONFIG_IEEE802154_ESP32=y
CONFIG_OPENTHREAD=y
CONFIG_OPENTHREAD_THREAD_VERSION_1_3=y
CONFIG_OPENTHREAD_FTD=y        # Full Thread Device
CONFIG_OPENTHREAD_SHELL=y      # gives 'ot' shell commands
CONFIG_NET_SOCKETS=y
CONFIG_NET_UDP=y
CONFIG_COAP=y
```

#### Commissioning the device onto a Thread network

1. Deploy an **OpenThread Border Router (OTBR)** — this can be:
   - A Raspberry Pi running the [OTBR docker image](https://openthread.io/guides/border-router/docker)
   - A Home Assistant OS add-on ("OpenThread Border Router" + "Silicon Labs Multiprotocol")
   - A Thread-capable smart home hub (e.g. HomePod mini, Google Nest Hub)

2. On the ESP32-C6 shell:
   ```
   ot factoryreset
   ot dataset init new
   ot dataset commit active
   ot ifconfig up
   ot thread start
   ot state        # → detached → child → router
   ```

3. Use the OTBR web interface to form or join the network, then commission
   the ESP32-C6 using the Active Dataset from the border router.

### 5.2 Connecting to Home Assistant

#### Architecture

```
SGP30 → ESP32-C6 ──Thread──► OTBR ──IPv6/UDP──► Home Assistant
                                                  (CoAP or MQTT)
```

Home Assistant communicates with Thread devices via **Matter** or directly
via **CoAP** using the `rest_command` / `notify` integrations, depending on
your setup.

#### Option A: MQTT over Thread (simplest)

1. Install the **Mosquitto MQTT broker** add-on in Home Assistant.
2. The OTBR provides NAT64 / IPv6 routing so the ESP32-C6 can reach the
   broker at its IPv6 address.
3. Add to `prj.conf`:
   ```kconfig
   CONFIG_MQTT_LIB=y
   CONFIG_MQTT_CLEAN_SESSION=y
   ```
4. In `src/connectivity/thread_mqtt.c` (scaffold — add in Phase 2):
   ```c
   /* Publish every 30 s */
   mqtt_publish(&client, &param);
   ```
5. Home Assistant `configuration.yaml`:
   ```yaml
   mqtt:
     sensor:
       - name: "Living Room eCO2"
         state_topic: "aqm/eco2"
         unit_of_measurement: "ppm"
         device_class: carbon_dioxide
       - name: "Living Room TVOC"
         state_topic: "aqm/tvoc"
         unit_of_measurement: "ppb"
   ```

#### Option B: CoAP (native Thread, no broker)

Zephyr's CoAP library can publish resources directly. The OTBR forwards
the packets to Home Assistant via its REST API.  This is lower overhead but
requires the `rest_command` HA integration.

#### Option C: Matter over Thread (future-proof)

Matter is the industry standard for smart home interoperability. The
ESP32-C6 is Matter-capable. Zephyr does not yet have first-class Matter
support, so this would require the ESP-IDF Matter SDK instead — consider it
a longer-term migration path.

### 5.3 OTA Firmware Updates (MCUboot)

#### Overview

Zephyr supports MCUboot-based A/B (swap) firmware updates. The ESP32-C6's
8 MB flash is sufficient for two firmware slots plus the bootloader.

#### Partition map (`boards/esp32c6_devkitc.overlay` addition)

```dts
/ {
    chosen {
        zephyr,code-partition = &slot0_partition;
    };
};

&flash0 {
    partitions {
        compatible = "fixed-partitions";
        #address-cells = <1>;
        #size-cells = <1>;

        boot_partition: partition@0 {
            label = "mcuboot";
            reg = <0x00000000 0x00010000>;  /* 64 KB */
        };
        slot0_partition: partition@10000 {
            label = "image-0";
            reg = <0x00010000 0x00360000>;  /* ~3.4 MB */
        };
        slot1_partition: partition@370000 {
            label = "image-1";
            reg = <0x00370000 0x00360000>;  /* ~3.4 MB */
        };
        scratch_partition: partition@6d0000 {
            label = "image-scratch";
            reg = <0x006d0000 0x00010000>;  /* 64 KB */
        };
        storage_partition: partition@6e0000 {
            label = "storage";
            reg = <0x006e0000 0x00020000>;  /* 128 KB */
        };
    };
};
```

#### Build MCUboot for ESP32-C6

```bash
# From workspace root (zephyr/ must be present):
west build -b esp32c6_devkitc zephyr/bootloader/mcuboot/boot/zephyr \
  --build-dir build/mcuboot \
  -- -DCONFIG_BOOT_SIGNATURE_TYPE_RSA=y

west flash --build-dir build/mcuboot
```

#### Build app with MCUboot support

Uncomment in `prj.conf`:
```kconfig
CONFIG_BOOTLOADER_MCUBOOT=y
CONFIG_MCUBOOT_IMG_MANAGER=y
CONFIG_IMG_MANAGER=y
CONFIG_FLASH=y
CONFIG_FLASH_MAP=y
CONFIG_STREAM_FLASH=y
```

#### Trigger OTA update

Using `mcumgr` over the Thread network (SMP server):
```bash
# Install mcumgr CLI tool
go install github.com/apache/mynewt-mcumgr-cli/mcumgr@latest

# Upload new firmware
mcumgr --conntype udp --connstring "[<device-ipv6-addr>]:1337" \
  image upload build/air-quality-monitor/zephyr/zephyr.signed.bin

# Confirm and swap
mcumgr --conntype udp --connstring "[<device-ipv6-addr>]:1337" \
  image confirm
```

---

## 6. EEZ Studio UI Design

[EEZ Studio](https://www.envox.eu/eez-studio/) is a free, open-source GUI
designer that can export production-ready LVGL C code. This lets you
redesign the screen layout visually without touching C directly.

### Workflow

#### Step 1 — Install EEZ Studio

Download from [github.com/eez-open/studio](https://github.com/eez-open/studio/releases).
Available for Windows, macOS, and Linux (AppImage).

#### Step 2 — Create a new LVGL project

1. File → New Project → **LVGL** (not "EEZ-GUI").
2. Set **Display width: 320**, **Display height: 240**.
3. Select LVGL version **8.3** (matching what Zephyr 3.7 ships).
4. Save the project as `ui/eez-project/air_quality_monitor.eez-project`.

#### Step 3 — Recreate the screen

Reproduce the two-gauge layout:
- Add a `lv_meter` widget for eCO2 (left half, x=3, y=30, w=154, h=182).
- Add a `lv_meter` widget for TVOC (right half, x=163, y=30).
- Give each meter three colour arc indicators (green/amber/red).
- Add `lv_label` widgets for the value and quality strings.
- Add a header bar (`lv_obj`) with a title label.
- Add a footer `lv_label` for the uptime/update counter.

Name your widgets clearly (e.g. `eco2_meter`, `tvoc_needle`) because EEZ
Studio uses those names in the generated C variable declarations.

#### Step 4 — Export generated C code

File → Export → **LVGL C code**.

EEZ Studio generates:
```
ui/
├── ui.c          ← calls lv_init(), creates screen objects
├── ui.h
└── screens/
    └── screen_main.c
```

#### Step 5 — Integrate generated code

1. Replace `src/ui/ui.c` and `src/ui/ui.h` with the generated files.
2. Update `CMakeLists.txt` to include any new `screens/*.c` files.
3. Keep the **interface contract** from the original `ui.h`:
   - `ui_init()` — called once from `main.c`.
   - `ui_update(eco2_ppm, tvoc_ppb, valid)` — update gauge values.
   - `ui_tick()` — call `lv_timer_handler()`.

   In the generated code, implement `ui_update()` by calling:
   ```c
   lv_meter_set_indicator_value(eco2_meter, eco2_needle, eco2_to_needle(eco2_ppm));
   lv_label_set_text_fmt(eco2_value_lbl, "%u ppm", eco2_ppm);
   // … etc
   ```

4. Re-build and flash — the visual result should be identical but you can
   now iterate on the layout in EEZ Studio without rewriting C code.

#### Tips

- EEZ Studio can show a **live preview** of LVGL 8 widgets without a
  physical device — very useful for rapid iteration.
- Use EEZ Studio's **style editor** to change colours, fonts, and spacing
  globally without hunting through C code.
- When you change the screen in EEZ Studio, only re-export → replace the
  `screens/` files; your hand-written `sgp30.c` and `main.c` are untouched.

---

## 7. Troubleshooting

### Display stays black

1. Check the backlight: is GPIO11 driving the LED pin high?  
   Add `gpio_pin_set(gpio, 11, 1)` early in `main.c` and test.
2. Verify SPI clock polarity/phase. ILI9341 uses **CPOL=0, CPHA=0** (SPI
   mode 0). Check if `spi-cpol` or `spi-cpha` appear in your Zephyr version's
   ILI9341 binding and set them accordingly in the overlay.
3. Check the RESET pin: GPIO10 should pulse low briefly during init. If the
   display driver skips the hardware reset, add a manual `gpio_pin_set` pulse.

### Display shows garbled colours

The ILI9341 can be byte-swapped. `CONFIG_LV_COLOR_16_SWAP=y` is set in
`prj.conf`. If colours still look wrong, try toggling this option.

### SGP30 not found (I2C error)

1. Verify SDA/SCL wiring on GPIO22/GPIO23.
2. Add `CONFIG_I2C_LOG_LEVEL_DBG=y` and check the log for `nack` or
   `timeout`.
3. The SGP30's I2C address is fixed at **0x58** — no pull-up conflicts with
   other devices at this address.
4. Ensure 10 kΩ pull-up resistors are on SDA and SCL (the Adafruit breakout
   has them on-board).

### Pinctrl macros not found (`SPIM2_MOSI_GPIO7` undefined)

The `SPIM2_*` and `I2C0_*` macros must be present in:
`<zephyr/dt-bindings/pinctrl/esp32c6-pinctrl.h>`

If they are missing in your Zephyr version, define them manually using
the raw `ESP32_PINMUX` macro. Example:

```c
/* From hal_espressif esp32c6_pinmap.h:
 *   SPIM2_MOSI signal_out = 0x15  (check your version!)
 *   GPIO7 = 7
 */
#define SPIM2_MOSI_GPIO7  ESP32_PINMUX(7, ESP_I2SO_SD, SPIM2_MOSI)
```

Check `modules/hal/espressif/components/soc/esp32c6/include/soc/gpio_sig_map.h`
for the correct signal IDs.

### `west flash` fails

Make sure the board is in **Download mode**: hold the BOOT button, press
RESET, release RESET, then release BOOT. The chip stays in download mode
until a successful flash or power cycle.

---

## 8. References

- [ESP32-C6-DevKitC-1 User Guide](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32c6/esp32-c6-devkitc-1/user_guide.html)
- [ILI9341 MSP2807 Wiki](https://www.lcdwiki.com/2.8inch_SPI_Module_ILI9341_SKU:MSP2807)
- [Adafruit SGP30 Product Page](https://www.adafruit.com/product/3709)
- [Zephyr ILI9xxx driver](https://docs.zephyrproject.org/latest/dts/api/bindings/display/ilitek,ili9341.html)
- [Zephyr SGP30 sensor](https://docs.zephyrproject.org/latest/dts/api/bindings/sensor/sensirion,sgp30.html)
- [Zephyr LVGL integration](https://docs.zephyrproject.org/latest/services/display/lvgl.html)
- [OpenThread on Zephyr](https://docs.zephyrproject.org/latest/connectivity/networking/thread.html)
- [EEZ Studio](https://github.com/eez-open/studio)
- [MCUboot for Zephyr](https://docs.zephyrproject.org/latest/services/device_mgmt/dfu.html)