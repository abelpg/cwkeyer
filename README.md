# CwKeyer

CW (Morse Code) Keyer application built in C++ with a Qt/QML graphical interface.  
It supports USB paddle adapters, keyboard input, audio tone generation, serial port output, and real-time CW decoding.

---

## Table of Contents

- [Architecture Overview](#architecture-overview)
- [Directory Structure](#directory-structure)
- [Modules](#modules)
  - [main.cpp](#maincpp)
  - [gui – GuiConnector](#gui--guiconnector)
  - [keyer – Keyer](#keyer--keyer)
  - [sound – Sound](#sound--sound)
  - [cwdecoder – CwDecoder & MorseTable](#cwdecoder--cwdecoder--morsetable)
  - [usb – UsbDevice & Device](#usb--usbdevice--device)
  - [serial – SerialComm](#serial--serialcomm)
  - [keyboard – Keyboard & KeyboardListener](#keyboard--keyboard--keyboardlistener)
  - [configuration – Configuration](#configuration--configuration)
  - [utils – Interfaces & Utilities](#utils--interfaces--utilities)
- [Default Parameters](#default-parameters)
- [CW Timing (PARIS Standard)](#cw-timing-paris-standard)
- [Keyer Modes](#keyer-modes)
- [Supported Devices](#supported-devices)
- [Dependencies](#dependencies)

---

## Architecture Overview

```
USB Paddle / Keyboard
        │
        ▼
    UsbDevice / KeyboardListener
        │  onDit / onDah
        ▼
      Keyer  ──────────────────────────────┐
        │ runCW(item, duration)            │
     ┌──┴───────────────┐                  │
   Sound           SerialComm         CwDecoder
 (audio tone)   (serial port)      (text decode)
                                          │
                                  callback → GuiConnector
                                          │
                                       QML UI
```

`GuiConnector` is the central Qt/QML bridge. It owns all components and exposes properties and invokable methods to the QML layer.

---

## Directory Structure

```
App/
├── main.cpp
├── autogen/
│   └── environment.h          # Auto-generated Qt environment setup
├── configuration/
│   ├── Configuration.h/.cpp   # JSON-based persistent configuration
├── cwdecoder/
│   ├── CwDecoder.h/.cpp       # Real-time CW decoder
│   └── MorseTable.h           # ITU-R Morse code lookup table
├── gui/
│   ├── GuiConnector.h/.cpp    # Qt/QML bridge (central controller)
├── keyboard/
│   ├── Keyboard.h/.cpp        # Keyboard output (simulates keystrokes)
│   └── KeyboardListener.h/.cpp# Low-level Windows keyboard hook input
├── keyer/
│   ├── Keyer.h/.cpp           # Iambic/Ultimatic keyer logic
├── serial/
│   ├── SerialComm.h/.cpp      # Serial port CW output (Windows)
├── sound/
│   ├── Sound.h/.cpp           # Audio tone generator (Qt Multimedia)
├── usb/
│   ├── UsbDevice.h/.cpp       # libusb USB paddle reader
│   ├── Device.h/.cpp          # USB device model
│   ├── DeviceInterface.h/.cpp # USB interface/endpoint model
│   ├── HidDevice.h/.cpp       # HID device support
│   └── hidapi/ libusb/        # Third-party USB libraries
└── utils/
    ├── IDitDah.h              # Interface: dit/dah input events
    ├── IKeyerCW.h             # Interface: CW output + timing formulas
    ├── Utils.h/.cpp           # General utilities (sleep, etc.)
```

---

## Modules

### main.cpp

Application entry point.

- Initializes the Qt environment via `set_qt_environment()`.
- Creates a `QApplication` and `QQmlApplicationEngine`.
- Instantiates `GuiConnector` and exposes it to QML as `guiConnector`.
- Loads the main QML file.
- Connects QML signals:
  - `deviceUpdated(QVariant)` → QML `deviceUpdated(QVariant)` slot
  - `textCwDecoderUpdated(QVariant)` → QML `textCwDecoderUpdated(QVariant)` slot
- Calls `guiConnector.initDevice()` after QML object creation.
- Connects `aboutToQuit` to `GuiConnector::quit()` for clean shutdown.

---

### gui – GuiConnector

**Files:** `gui/GuiConnector.h`, `gui/GuiConnector.cpp`

Central controller and Qt/QML bridge (`QObject`). Owns all subsystem components.

#### Q_PROPERTY bindings (QML-accessible)

| Property              | Type         | Default    | Description                         |
|-----------------------|--------------|------------|-------------------------------------|
| `amplitude`           | `double`     | `0.5`      | Audio tone amplitude (0.0–1.0)      |
| `frequency`           | `double`     | `650` Hz   | Audio tone frequency                |
| `wpm`                 | `int`        | `25`       | Keyer speed in Words Per Minute     |
| `farnsWorth`          | `int`        | `25`       | Farnsworth timing WPM               |
| `mode`                | `int`        | `IAMBIC_B` | Keyer mode (0=Ultimatic, 1=A, 2=B)  |
| `enabledSound`        | `int`        | `true`     | Toggle audio output                 |
| `enabledCommOut`      | `int`        | `false`    | Toggle serial port output           |
| `enabledKeyboard`     | `bool`       | `false`    | Toggle keyboard stroke output       |
| `enabledCwDecoder`    | `bool`       | `false`    | Toggle CW decoder                   |
| `selectedAudioDevice` | `int`        | `0`        | Index of audio output device        |
| `selectedCommPort`    | `int`        | `-1`       | Index of selected serial port       |
| `audioDevices`        | `QStringList`|            | List of available audio devices     |
| `commPorts`           | `QStringList`|            | List of available serial ports      |
| `enabledZadig`        | `bool`       |            | True when USB paddle is connected   |

#### Invokable Methods (callable from QML)

| Method               | Description                                              |
|----------------------|----------------------------------------------------------|
| `initDevice()`       | Initializes USB device; falls back to keyboard listener  |
| `detectDevice()`     | Scans for compatible USB devices                         |
| `connectDevice()`    | Connects to the detected USB device                      |
| `disconnectDevice()` | Disconnects the USB device                               |
| `quit()`             | Stops all subsystems and releases resources              |

#### Signals

| Signal                           | Description                                  |
|----------------------------------|----------------------------------------------|
| `deviceUpdated(QVariant)`        | Emitted when USB device status changes       |
| `textCwDecoderUpdated(QVariant)` | Emitted when CW decoder decodes a character  |

#### Initialization sequence

1. Creates `SerialComm` and loads available COM ports.
2. Loads audio device list.
3. Loads persisted configuration (`configuration.json`).
4. Creates and configures `Sound`.
5. Creates `CwDecoder` with a text callback.
6. Creates `Keyer`, registers `SerialComm` and `CwDecoder` as CW outputs.
7. Creates `Keyboard` (keystroke output) and `UsbDevice` (USB paddle reader).
8. Creates `KeyboardListener` (fallback when no USB device present).

---

### keyer – Keyer

**Files:** `keyer/Keyer.h`, `keyer/Keyer.cpp`

Implements the iambic/Ultimatic keyer logic. Implements `IDitDah` to receive paddle events.

#### Methods

| Method                           | Description                                           |
|----------------------------------|-------------------------------------------------------|
| `Keyer(IKeyerCW *soundCW)`       | Constructor; registers the primary CW output          |
| `initKeyer(int wpm, Mode mode)`  | Calculates dit/dah/space timings for the given WPM    |
| `onDit(bool pressed)`            | Called when the dit paddle is pressed/released        |
| `onDah(bool pressed)`            | Called when the dah paddle is pressed/released        |
| `addKeyerCW(IKeyerCW*)`          | Registers an additional CW output (serial, decoder…)  |
| `ditTime()` / `dahTime()` / `spaceTime()` | Returns computed timing values in ms        |

#### Keyer logic

- Paddle events are enqueued and processed in a detached thread (`keyerCall`).
- After each element, the squeeze state is checked to generate the next element according to the active mode.

---

### sound – Sound

**Files:** `sound/Sound.h`, `sound/Sound.cpp`

Generates and plays CW audio tones using **Qt Multimedia** (`QAudioSink`). Implements `IKeyerCW`.

#### Methods

| Method                                                | Description                                        |
|-------------------------------------------------------|----------------------------------------------------|
| `init(freq, sampleRate, amplitude, attack, release)`  | Initialize with the default audio device           |
| `initWithDevice(device, freq, ...)`                   | Initialize with a specific `QAudioDevice`          |
| `stop()`                                              | Stop and release the audio sink                    |
| `runCW(item, duration)`                               | Called by `Keyer`; plays a tone of given duration  |
| `setEnabled(bool)`                                    | Enable/disable audio output                        |

- Pre-generates PCM audio buffers and caches them by duration for efficiency.
- Uses attack/release envelopes to eliminate clicks.
- Default: 44 100 Hz sample rate, 650 Hz tone, 0.5 amplitude, 10 ms attack/release.

---

### cwdecoder – CwDecoder & MorseTable

**Files:** `cwdecoder/CwDecoder.h/.cpp`, `cwdecoder/MorseTable.h`

#### CwDecoder

Real-time CW decoder. Implements `IKeyerCW` to receive keyed elements from `Keyer`.

| Method                    | Description                                           |
|---------------------------|-------------------------------------------------------|
| `CwDecoder(callback)`     | Constructor; receives a `std::function<void(string)>` |
| `start(farnsWorth, wpm)`  | Starts the decoder, computing space thresholds        |
| `stop()`                  | Stops the timeout thread                              |
| `runCW(item, duration)`   | Receives a DIT/DAH element; accumulates the sequence  |

- Internally builds a dot-dash sequence string per letter.
- A background timeout thread (`timeoutLoop`) flushes a completed letter or word when silence exceeds the letter/word space threshold.
- On decoding `=` (BT prosign), a newline is appended.
- Thread-safe via `std::mutex`.

#### MorseTable

Static lookup table mapping dot-dash sequences to characters (ITU-R M.1677).

| Method              | Description                                      |
|---------------------|--------------------------------------------------|
| `toSymbol(item)`    | Returns `'.'` for DIT, `'-'` for DAH            |
| `decode(sequence)`  | Returns the character for a sequence, or `'\0'` |

Covers: **A–Z**, **0–9**, and common punctuation (`.`, `,`, `?`, `!`, `/`, `(`, `)`, `&`, `:`, `;`, `=`, `+`, `-`, `"`, `@`, `_`).

---

### usb – UsbDevice & Device

**Files:** `usb/UsbDevice.h/.cpp`, `usb/Device.h/.cpp`, `usb/DeviceInterface.h/.cpp`

#### UsbDevice

Reads USB HID paddle adapters via **libusb**. Dispatches dit/dah events to `Keyer`.

| Method                | Description                                                 |
|-----------------------|-------------------------------------------------------------|
| `initDevice()`        | Loads last known device from config and tries to connect    |
| `detectDevice()`      | Scans all USB devices for a compatible adapter              |
| `connectDevice()`     | Connects to the previously detected device                  |
| `disconnectDevice()`  | Releases the USB interface and stops the reading thread     |
| `addDitDah(IDitDah*)` | Registers additional dit/dah listeners (e.g., `Keyboard`)  |
| `connected()`         | Returns `true` if a device is currently connected           |
| `intToHex<T>(T)`      | Utility to format integers as hex strings                   |

- Reads USB interrupt transfers in a detached thread (`taskRunnable`).
- Decodes left/right click bytes (`CLICK_LEFT=0x01`, `CLICK_RIGHT=0x02`) to dit/dah events.

#### Device

Model class representing a USB device.

| Field / Method           | Description                                   |
|--------------------------|-----------------------------------------------|
| `vendorId`, `productId`  | USB vendor and product IDs                    |
| `getInterface()`         | Returns the associated `DeviceInterface`      |
| `toJson()` / `fromJson()`| JSON serialization for configuration storage  |

Tested devices:
- Vail Adapter: VID `0x413d` / PID `0x2107`, interface 0, endpoint `0x81`
- Left/right click: same VID/PID, interface 1, endpoint `0x82`

---

### serial – SerialComm

**Files:** `serial/SerialComm.h`, `serial/SerialComm.cpp`

Outputs CW keying signals over a Windows serial port (RTS line). Implements `IKeyerCW`.

| Method                  | Description                                       |
|-------------------------|---------------------------------------------------|
| `start(portName)`       | Opens the COM port at 9600 baud                   |
| `stop()`                | Closes the port                                   |
| `started()`             | Returns `true` if port is open                    |
| `listPorts()`           | Returns a list of available COM port names        |
| `runCW(item, duration)` | Asserts/deasserts RTS for the element duration    |

- Default baud rate: `9600`.
- Windows-only (`HANDLE`, `windows.h`).

---

### keyboard – Keyboard & KeyboardListener

#### Keyboard

**Files:** `keyboard/Keyboard.h`, `keyboard/Keyboard.cpp`

Implements `IDitDah`. When enabled, sends keyboard keystrokes (e.g., for Vail/VBand web apps) in response to dit/dah events from `UsbDevice`.

| Method             | Description                              |
|--------------------|------------------------------------------|
| `onDit(pressed)`   | Sends a dit keystroke via Windows API    |
| `onDah(pressed)`   | Sends a dah keystroke via Windows API    |
| `setEnabled(bool)` | Enables/disables keystroke output        |

#### KeyboardListener

**Files:** `keyboard/KeyboardListener.h`, `keyboard/KeyboardListener.cpp`

Fallback input when no USB device is present. Installs a **Windows low-level keyboard hook** to intercept key presses and forward them as dit/dah events to `Keyer`.

| Method             | Description                                           |
|--------------------|-------------------------------------------------------|
| `setEnabled(bool)` | Installs or removes the keyboard hook                 |
| `isEnabled()`      | Returns hook state                                    |

---

### configuration – Configuration

**Files:** `configuration/Configuration.h`, `configuration/Configuration.cpp`

Persistent JSON configuration stored in `configuration.json`.

#### Configuration keys

| Key                     | Constant                    | Type     | Description              |
|-------------------------|-----------------------------|----------|--------------------------|
| `amplitude`             | `CFG_AMPLITUDE`             | `double` | Audio amplitude          |
| `frequency`             | `CFG_FREQUENCY`             | `double` | Tone frequency           |
| `wpm`                   | `CFG_WPM`                   | `int`    | Keyer speed              |
| `farnsWorth`            | `CFG_FARNSWORTH`            | `int`    | Farnsworth speed         |
| `selected_audio_device` | `CFG_SELECTED_AUDIO_DEVICE` | `int`    | Audio device index       |
| `mode`                  | `CFG_MODE`                  | `int`    | Keyer mode               |
| `commout`               | `CFG_COMM_OUT`              | `int`    | Selected COM port index  |

#### Static API

```cpp
Configuration::putValueInt(key, value);
Configuration::putValueDouble(key, value);
Configuration::getValueInt(key);
Configuration::getValueDouble(key);
// also: Bool, String, Object variants
```

---

### utils – Interfaces & Utilities

#### IDitDah (`utils/IDitDah.h`)

Pure virtual interface for components that receive paddle input.

```cpp
virtual void onDit(bool pressed) = 0;
virtual void onDah(bool pressed) = 0;
```

Implemented by: `Keyer`, `Keyboard`.

#### IKeyerCW (`utils/IKeyerCW.h`)

Pure virtual interface for components that produce CW output, plus static timing helpers.

```cpp
virtual void runCW(KeyerItem item, int duration) = 0;
```

Implemented by: `Sound`, `SerialComm`, `CwDecoder`.

**KeyerItem enum:**

| Value                 | Hex   | Description             |
|-----------------------|-------|-------------------------|
| `DIT`                 | `0x1` | Short element (dot)     |
| `DAH`                 | `0x2` | Long element (dash)     |
| `INTER_ELEMENT_SPACE` | `0x3` | Space between elements  |
| `LETTER_SPACE`        | `0x4` | Space between letters   |
| `WORD_SPACE`          | `0x5` | Space between words     |

#### Utils (`utils/Utils.h/.cpp`)

General utilities, including `Utils::sleepFor(ms)` for precise thread sleeping.

---

## Default Parameters

| Parameter   | Value     |
|-------------|-----------|
| WPM         | 25        |
| Farnsworth  | 25        |
| Sample Rate | 44 100 Hz |
| Frequency   | 650 Hz    |
| Amplitude   | 0.5       |
| Attack      | 10 ms     |
| Release     | 10 ms     |
| Mode        | Iambic B  |

---

## CW Timing (PARIS Standard)

Based on the word **PARIS** = 50 units:

| Element              | Duration          |
|----------------------|-------------------|
| Dit                  | `1200 ms / WPM`   |
| Dah                  | `3 × Dit`         |
| Inter-element space  | `1 × Dit`         |
| Letter space         | `3 × Dit`         |
| Word space           | `7 × Dit`         |

Examples:

| WPM | Dit   | Dah    |
|-----|-------|--------|
| 15  | 80 ms | 240 ms |
| 20  | 60 ms | 180 ms |
| 25  | 48 ms | 144 ms |
| 30  | 40 ms | 120 ms |

---

## Keyer Modes

| Mode       | Value | Behavior                                                           |
|------------|-------|--------------------------------------------------------------------|
| `ULTIMATIC`| `0x1` | Squeeze repeats the last-pressed paddle                            |
| `IAMBIC_A` | `0x2` | Squeeze alternates dit/dah; stops immediately on release           |
| `IAMBIC_B` | `0x3` | Like A but completes one more element after both paddles released  |

---

## Supported Devices

| Device               | VID    | PID    | Interface | Endpoint |
|----------------------|--------|--------|-----------|----------|
| Vail Adapter         | 0x413d | 0x2107 | 0         | 0x81     |
| Vail (click output)  | 0x413d | 0x2107 | 1         | 0x82     |

If no USB device is found, the application automatically falls back to `KeyboardListener` for input.

---

## Dependencies

| Library      | Usage                       |
|--------------|-----------------------------|
| Qt 6         | GUI, QML, Multimedia, JSON  |
| libusb       | USB HID paddle reading      |
| hidapi       | HID device access           |
| Windows API  | Serial port, keyboard hook  |

