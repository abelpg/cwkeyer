# CwKeyer

CW (Morse Code) Keyer application built in C++ with a Qt/QML graphical interface.  
It supports USB paddle adapters, keyboard input, straight key via serial DSR (N1MM Logger integration), audio tone generation, serial port output, and real-time CW decoding.

---

## Table of Contents

- [Architecture Overview](#architecture-overview)
- [Directory Structure](#directory-structure)
- [Modules](#modules)
  - [main.cpp](#maincpp)
  - [gui – GuiConnector](#gui--guiconnector)
  - [keyer – Keyer](#keyer--keyer)
  - [sound – Sound & CwGenerator](#sound--sound--cwgenerator)
  - [cwdecoder – CwDecoder & MorseTable](#cwdecoder--cwdecoder--morsetable)
  - [usb – UsbDevice, Device & HidDevice](#usb--usbdevice-device--hiddevice)
  - [serial – SerialComm, SerialPorts & N1MMProxy](#serial--serialcomm-serialports--n1mmproxy)
  - [keyboard – Keyboard & KeyboardListener](#keyboard--keyboard--keyboardlistener)
  - [configuration – Configuration](#configuration--configuration)
  - [utils – Interfaces, Logger & Utilities](#utils--interfaces-logger--utilities)
- [Default Parameters](#default-parameters)
- [CW Timing (PARIS Standard)](#cw-timing-paris-standard)
- [Keyer Modes](#keyer-modes)
- [Supported Input Methods](#supported-input-methods)
- [Dependencies](#dependencies)

---

## Architecture Overview

```
USB Paddle / Keyboard / N1MM Logger (serial DSR)
        │
        ▼
    UsbDevice / KeyboardListener / N1MMProxy
        │  onDit / onDah / onStraight
        ▼
      Keyer  ──────────────────────────────┐
        │ runCW / startRunCw / stopRunCw   │
     ┌──┴─────────────────┐                │
   Sound             SerialComm       CwDecoder
 (audio tone)      (serial DTR)    (text decode)
   CwGenerator                           │
 (continuous CW)                 callback → GuiConnector
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
│   └── MorseTable.h           # ITU-R Morse code lookup table + prosigns
├── gui/
│   ├── GuiConnector.h/.cpp    # Qt/QML bridge (central controller)
├── keyboard/
│   ├── Keyboard.h/.cpp        # Keyboard output (simulates keystrokes via WinAPI)
│   └── KeyboardListener.h/.cpp# Low-level Windows keyboard hook input
├── keyer/
│   ├── Keyer.h/.cpp           # Iambic/Ultimatic keyer + straight key logic
├── serial/
│   ├── SerialComm.h/.cpp      # Serial port CW output via DTR (Windows)
│   ├── SerialPorts.h/.cpp     # Serial port enumeration from Windows Registry
│   └── N1MMProxy.h/.cpp       # N1MM Logger serial input (DSR → straight key)
├── sound/
│   ├── Sound.h/.cpp           # Audio tone generator (Qt Multimedia, push mode)
│   └── CwGenerator.h/.cpp     # Continuous CW tone with attack/release envelope
├── usb/
│   ├── UsbDevice.h/.cpp       # libusb USB paddle reader
│   ├── Device.h/.cpp          # USB device model (JSON serializable)
│   ├── DeviceInterface.h/.cpp # USB interface/endpoint model
│   ├── HidDevice.h/.cpp       # HID device support (hidapi, alternative)
│   └── hidapi/ libusb/        # Third-party USB libraries
└── utils/
    ├── IDitDah.h              # Interface: dit/dah/straight input events
    ├── IKeyerCW.h             # Interface: CW output + PARIS timing formulas
    ├── Logger.h               # Logging utility with log levels
    ├── Utils.h/.cpp           # General utilities (sleepFor)
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
- Global log level set to `L_INFO`.

---

### gui – GuiConnector

**Files:** `gui/GuiConnector.h`, `gui/GuiConnector.cpp`

Central controller and Qt/QML bridge (`QObject`). Owns all subsystem components.

#### Q_PROPERTY bindings (QML-accessible)

| Property              | Type         | Default        | Description                                      |
|-----------------------|--------------|----------------|--------------------------------------------------|
| `amplitude`           | `double`     | `0.5`          | Audio tone amplitude (0.0–1.0)                   |
| `frequency`           | `double`     | `650` Hz       | Audio tone frequency                             |
| `wpm`                 | `int`        | `25`           | Keyer speed in Words Per Minute                  |
| `farnsWorth`          | `int`        | `25`           | Farnsworth timing WPM                            |
| `mode`                | `int`        | `IAMBIC_B`     | Keyer mode (1=Ultimatic, 2=A, 3=B)              |
| `enabledSound`        | `int`        | `true`         | Toggle audio output                              |
| `enabledCommOut`      | `int`        | `false`        | Toggle serial port CW output (DTR)               |
| `enabledCommIn`       | `int`        | `false`        | Toggle N1MM serial input (DSR straight key)      |
| `enabledKeyboard`     | `bool`       | `false`        | Toggle keyboard stroke output                    |
| `enabledCwDecoder`    | `bool`       | `false`        | Toggle CW decoder                                |
| `selectedAudioDevice` | `int`        | `0`            | Index of audio output device                     |
| `selectedCommPort`    | `int`        | `-1`           | Index of selected serial port for CW output      |
| `selectedCommPortIn`  | `int`        | `-1`           | Index of selected serial port for N1MM input     |
| `audioDevices`        | `QStringList`|                | List of available audio devices                  |
| `commPorts`           | `QStringList`|                | List of available serial ports                   |
| `enabledZadig`        | `bool`       |                | True when USB paddle is connected                |

#### Invokable Methods (callable from QML)

| Method               | Description                                              |
|----------------------|----------------------------------------------------------|
| `initDevice()`       | Initializes USB device; falls back to keyboard listener  |
| `detectDevice()`     | Scans for compatible USB devices (plug & detect)         |
| `connectDevice()`    | Connects to the detected USB device                      |
| `disconnectDevice()` | Disconnects the USB device                               |
| `quit()`             | Stops all subsystems and releases resources              |

#### Signals

| Signal                          | Description                                        |
|---------------------------------|----------------------------------------------------|
| `deviceUpdated(QVariant)`       | Emitted when USB device status changes             |
| `textCwDecoderUpdated(QVariant)`| Emitted when CW decoder decodes a character        |
| `amplitudeChanged`              | Amplitude property changed                         |
| `frequencyChanged`              | Frequency property changed                         |
| `wpmChanged`                    | WPM property changed                               |
| `farnsWorthChanged`             | Farnsworth property changed                        |
| `modeChanged`                   | Keyer mode changed                                 |
| `soundEnabledChanged`           | Sound enabled state changed                        |
| `enabledCommOutChanged`         | Serial output enabled state changed                |
| `enabledCommInChanged`          | N1MM serial input enabled state changed            |
| `enabledKeyboardChanged`        | Keyboard output enabled state changed              |
| `enabledCwDecoderChanged`       | CW decoder enabled state changed                   |
| `enabledZadigChanged`           | USB device connection state changed                |
| `commPortsChanged`              | Serial port list changed                           |
| `selectedCommPortChanged`       | Selected output port index changed                 |
| `selectedCommPortInChanged`     | Selected input port index changed                  |

#### Initialization sequence

1. Creates `SerialComm` (CW output via DTR).
2. Loads available audio devices and serial ports.
3. Loads persisted configuration (`configuration.json`).
4. Creates and configures `Sound`.
5. Creates `CwDecoder` with a text callback.
6. Creates `Keyer`, registers `SerialComm` and `CwDecoder` as CW outputs.
7. Creates `N1MMProxy` for N1MM Logger straight key input (serial DSR monitoring).
8. Creates `Keyboard` (keystroke output) and `UsbDevice` (USB paddle reader); registers `Keyboard` as additional dit/dah listener.
9. Creates `KeyboardListener` (fallback when no USB device present).

#### Business rules

- `farnsWorth` is always ≤ `wpm`; setting either enforces the constraint automatically.
- Decoded `=` (BT), `<SK>` and `<BK>` prosigns append a newline in the GUI.
- On `quit()`: stops Sound, disconnects USB device, stops both serial ports, then deletes owned objects.

---

### keyer – Keyer

**Files:** `keyer/Keyer.h`, `keyer/Keyer.cpp`

Implements the iambic/Ultimatic keyer logic. Implements `IDitDah` to receive paddle events.

#### Methods

| Method                        | Description                                              |
|-------------------------------|----------------------------------------------------------|
| `Keyer(IKeyerCW *soundCW)`    | Constructor; registers the primary CW output             |
| `initKeyer(int wpm, Mode mode)` | Calculates dit/dah/space timings for the given WPM     |
| `onDit(bool pressed)`         | Called when the dit paddle is pressed/released           |
| `onDah(bool pressed)`         | Called when the dah paddle is pressed/released           |
| `onStraight(bool pressed)`    | Called for straight key; calls `startRunCw`/`stopRunCw`  |
| `addKeyerCW(IKeyerCW*)`       | Registers an additional CW output (serial, decoder…)    |
| `ditTime()` / `dahTime()` / `spaceTime()` | Returns computed timing values in ms        |

#### Keyer logic

- Paddle events are enqueued and processed in a detached thread (`keyerCall`).
- After each element, the squeeze state is checked to generate the next element according to the active mode (see [Keyer Modes](#keyer-modes)).
- Straight key bypasses the iambic logic: calls `startRunCw()`/`stopRunCw()` directly on all registered `IKeyerCW` outputs.

#### Mode enum

```cpp
enum Mode {
  ULTIMATIC = 0x1,
  IAMBIC_A  = 0x2,
  IAMBIC_B  = 0x3
};
```

---

### sound – Sound & CwGenerator

#### Sound

**Files:** `sound/Sound.h`, `sound/Sound.cpp`

Generates and plays CW audio tones using **Qt Multimedia** (`QAudioSink`) in **push mode** via a `QTimer`. Implements `IKeyerCW`.

| Method                                               | Description                                             |
|------------------------------------------------------|---------------------------------------------------------|
| `init(freq, sampleRate, amplitude, attack, release)` | Initialize with the default audio device                |
| `initWithDevice(device, freq, ...)`                  | Initialize with a specific `QAudioDevice`               |
| `stop()`                                             | Stops the push timer, sink, and active buffer           |
| `runCW(item, duration)`                              | Called by `Keyer`; emits `playRequested` (iambic mode)  |
| `startRunCw()`                                       | Emits `startCwRequested`; starts continuous tone (straight key) |
| `stopRunCw()`                                        | Emits `stopCwRequested`; applies release envelope       |
| `setEnabled(bool)`                                   | Enable/disable audio output                             |

**Push-mode architecture:**
- `runCW()` pre-generates PCM buffers cached by duration (iambic/paddle mode).
- `startRunCw()` / `stopRunCw()` use `CwGenerator` for continuous tone with smooth attack/release (straight key mode).
- A `QTimer` ticks every 20 ms and writes PCM chunks to the audio sink. The sink auto-shuts down after 2 s of silence.
- Thread-safe: all audio operations run on the Qt event loop via queued signal-slot connections.

#### CwGenerator

**Files:** `sound/CwGenerator.h`, `sound/CwGenerator.cpp`

Generates PCM chunks for continuous CW tones with configurable attack and release envelopes.

| Method                       | Description                                            |
|------------------------------|--------------------------------------------------------|
| `startStream()`              | Resets sample index; starts generating tone            |
| `stopStream()`               | Marks the stop point; applies release envelope         |
| `isStopped()`                | Returns `true` when release has fully completed        |
| `generateChunk(numSamples)`  | Returns a `QByteArray` of PCM Int16 samples            |

- Attack phase: linear ramp from 0 to full amplitude over `attackSamples`.
- Release phase: linear ramp from full amplitude to 0 over `releaseSamples`.
- After release, silence is produced until `startStream()` is called again.

---

### cwdecoder – CwDecoder & MorseTable

#### CwDecoder

**Files:** `cwdecoder/CwDecoder.h/.cpp`

Real-time CW decoder. Implements `IKeyerCW` to receive keyed elements from `Keyer`.

| Method                                  | Description                                           |
|-----------------------------------------|-------------------------------------------------------|
| `CwDecoder(callback)`                   | Constructor; receives a `std::function<void(string)>` |
| `start(farnsWorth, wpm)`               | Starts the decoder; computes inter-element/letter/word thresholds |
| `stop()`                                | Stops the timeout thread                              |
| `runCW(item, duration)`                 | Receives a DIT/DAH element; accumulates the sequence  |

**Timing thresholds:**
- `interElementSpace` = `calculateDuration(INTER_ELEMENT_SPACE, farnsWorth)`
- `letterSpace`       = `calculateDuration(LETTER_SPACE, wpm)`
- `wordSpace`         = `calculateDuration(WORD_SPACE, farnsWorth)`

**Decoding flow:**
1. Each incoming element is appended to `m_currentSequence` as `.` or `-`.
2. Silence elapsed since the last element is measured; if it exceeds `letterSpace`, the sequence is flushed to `MorseTable::decode()`.
3. If silence exceeds `wordSpace`, a space character ` ` is emitted after flushing.
4. A background thread (`timeoutLoop`) checks silence every `interElementSpace/2` ms to handle trailing letters/words.
5. Thread-safe via `std::mutex`.

**Decoded callbacks:**
| Decoded value | Callback argument |
|---------------|-------------------|
| Normal char   | `std::string(1, ch)` |
| BT prosign    | `"="` with newline appended by GuiConnector |
| SK prosign    | `"<SK>"` with newline appended              |
| BK prosign    | `"<BK>"` with newline appended              |
| Error (HH)    | `"<ERROR>"`                                 |

#### MorseTable

**File:** `cwdecoder/MorseTable.h`

Static lookup table mapping dot-dash sequences to characters (ITU-R M.1677).

| Method                    | Description                                           |
|---------------------------|-------------------------------------------------------|
| `toSymbol(KeyerItem)`     | Returns `'.'` for DIT, `'-'` for DAH                 |
| `decode(string)`          | Returns the character for a sequence, or `NOT_FOUND`  |

**Special return constants:**

| Constant    | Value  | Meaning                              |
|-------------|--------|--------------------------------------|
| `NOT_FOUND` | `'\0'` | Sequence not in table                |
| `SK`        | `'\1'` | End of work prosign (`...-.-`)       |
| `BK`        | `'\2'` | Break prosign (`-...-.-`)            |
| `CW_ERROR`  | `'\3'` | Error prosign – HH (`........`)      |

Covers: **A–Z**, **0–9**, common punctuation (`.`, `,`, `?`, `!`, `/`, `(`, `)`, `&`, `:`, `;`, `=`, `+`, `-`, `"`, `@`, `_`), and prosigns SK, BK, ERROR.

---

### usb – UsbDevice, Device & HidDevice

#### UsbDevice

**Files:** `usb/UsbDevice.h/.cpp`

Reads USB HID paddle adapters via **libusb**. Dispatches dit/dah events to all registered `IDitDah` listeners.

| Method               | Description                                                      |
|----------------------|------------------------------------------------------------------|
| `UsbDevice(IDitDah*)`| Constructor; adds first dit/dah listener                         |
| `initDevice()`       | Loads last known device from config and tries to connect         |
| `detectDevice()`     | Scans all USB devices; detects newly plugged-in adapter          |
| `connectDevice()`    | Connects to the previously detected/stored device                |
| `disconnectDevice()` | Releases the USB interface and stops the reading thread          |
| `addDitDah(IDitDah*)`| Registers additional dit/dah listeners (e.g., `Keyboard`)       |
| `connected()`        | Returns `true` if a device is currently connected                |
| `intToHex<T>(T)`     | Template utility to format integers as hex strings               |

**Connection flow:**
1. Tries to load last known device from `configuration.json` key `"device-usb"`.
2. If found, starts a background thread (`taskRunnable`) that opens the device and submits interrupt transfer callbacks via `libusb_fill_interrupt_transfer`.
3. `cbInterrupt` static callback decodes buffer bytes to dit/dah events and re-submits the transfer.

**Buffer decoding:**

| Condition                                      | Event                |
|------------------------------------------------|----------------------|
| `buffer[0] == CLICK_BOTH` or bytes 2 & 3 > 0 | Both pressed         |
| `buffer[0] == CLICK_LEFT` or byte 2 > 0       | Dit pressed          |
| `buffer[0] == CLICK_RIGHT` or byte 3 > 0      | Dah pressed          |
| None of the above                              | Both released        |

#### Device

**File:** `usb/Device.h/.cpp`

Model class representing a USB device. JSON-serializable.

| Field/Method              | Description                                     |
|---------------------------|-------------------------------------------------|
| `vendorId`, `productId`   | USB vendor and product IDs                      |
| `getInterface()`          | Returns the associated `DeviceInterface`        |
| `getPath()`               | Returns the HID path (for HidDevice usage)      |
| `toJson()` / `fromJson()` | JSON serialization for `configuration.json`     |

#### DeviceInterface

**File:** `usb/DeviceInterface.h/.cpp`

Model for a USB interface/endpoint.

| Field          | Description                              |
|----------------|------------------------------------------|
| `interfaceNum` | USB interface number                     |
| `endpoint`     | Endpoint address                         |
| `packetSize`   | Max packet size in bytes                 |

JSON-serializable via `toJson()` / `fromJson()`.

#### HidDevice

**Files:** `usb/HidDevice.h/.cpp`

Alternative HID device reader using **hidapi** (instead of libusb). Same lifecycle API as `UsbDevice`.

| Method               | Description                                           |
|----------------------|-------------------------------------------------------|
| `initDevice()`       | Loads last known device from config and connects      |
| `detectDevice()`     | Enumerates HID devices; detects newly plugged-in one  |
| `connectDevice()`    | Opens the HID device via `hid_open_path`              |
| `disconnectDevice()` | Closes the HID device                                 |

> **Note:** `HidDevice` is currently not wired into `GuiConnector`; `UsbDevice` (libusb) is the active implementation.

---

### serial – SerialComm, SerialPorts & N1MMProxy

#### SerialComm

**Files:** `serial/SerialComm.h`, `serial/SerialComm.cpp`

Outputs CW keying signals over a Windows serial port via the **DTR** line. Implements `IKeyerCW`.

| Method                       | Description                                          |
|------------------------------|------------------------------------------------------|
| `SerialComm(rtsControl, dtrControl, overlapped)` | Constructor; configures line control flags |
| `start(portName)`            | Opens the COM port at 9600 baud                      |
| `stop()`                     | Closes the port, clears DTR                          |
| `started()`                  | Returns `true` if port is open                       |
| `runCW(item, duration)`      | Asserts DTR for the element duration in a thread     |
| `startRunCw()`               | Asserts DTR immediately (straight key press)         |
| `stopRunCw()`                | Deasserts DTR immediately (straight key release)     |

- Default baud rate: `9600`.
- Windows-only (`HANDLE`, `windows.h`).
- `runCW` spawns a detached thread to hold DTR for `duration` ms.
- Constructor parameters `rtsControl`, `dtrControl`, `overlapped` allow subclasses (`N1MMProxy`) to configure the port differently.

#### SerialPorts

**Files:** `serial/SerialPorts.h`, `serial/SerialPorts.cpp`

Enumerates available serial ports from the Windows Registry.

| Method          | Description                                               |
|-----------------|-----------------------------------------------------------|
| `listPorts()`   | Returns `std::vector<std::string>` of COM port names      |

- Reads from `HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\SERIALCOMM`.

#### N1MMProxy

**Files:** `serial/N1MMProxy.h`, `serial/N1MMProxy.cpp`

Integrates with **N1MM Logger** (or any application that signals CW via the serial **DSR** line). Extends `SerialComm` and implements serial input monitoring.

| Method              | Description                                                  |
|---------------------|--------------------------------------------------------------|
| `N1MMProxy(IDitDah*)`| Constructor; registers the straight key listener            |
| `start(portName)`   | Opens port in overlapped mode, starts DSR monitor thread     |
| `stop()`            | Signals monitor thread, joins it, closes port                |

**DSR monitoring flow:**
1. Opens the COM port with `FILE_FLAG_OVERLAPPED` enabled.
2. Sets `EV_DSR` as the comm event mask.
3. Background thread (`dsrMonitorLoop`) calls `WaitCommEvent` with an overlapped structure.
4. Waits on both the comm event handle and an internal stop event (`WaitForMultipleObjects`).
5. On DSR change: calls `IDitDah::onStraight(true/false)` on the registered listener.
6. On stop: cancels pending I/O via `CancelIoEx`, waits for completion, exits.

---

### keyboard – Keyboard & KeyboardListener

#### Keyboard

**Files:** `keyboard/Keyboard.h`, `keyboard/Keyboard.cpp`

Implements `IDitDah`. When enabled, sends keyboard keystrokes in response to dit/dah events (e.g., for Vail/VBand web apps). Thread-safe via Qt queued signal-slot connections.

| Method              | Description                                               |
|---------------------|-----------------------------------------------------------|
| `onDit(bool)`       | Emits `ditChanged` signal → slots run on Qt event loop    |
| `onDah(bool)`       | Emits `dahChanged` signal → slots run on Qt event loop    |
| `onStraight(bool)`  | No-op                                                     |
| `setEnabled(bool)`  | Enables/disables keystroke output                         |

**Key mapping:**

| Event | Virtual Key    | Description      |
|-------|----------------|------------------|
| Dit   | `VK_OEM_1`     | `;` / `:` key    |
| Dah   | `VK_OEM_PLUS`  | `+` / `=` key    |

Uses `KEYEVENTF_SCANCODE` flag with `SendInput`.

#### KeyboardListener

**Files:** `keyboard/KeyboardListener.h`, `keyboard/KeyboardListener.cpp`

Fallback input when no USB device is present. Installs a **Windows low-level keyboard hook** (`WH_KEYBOARD_LL`) to intercept key presses and forward them as dit/dah events to `Keyer`.

| Method             | Description                                          |
|--------------------|------------------------------------------------------|
| `setEnabled(bool)` | Installs or removes the keyboard hook                |
| `isEnabled()`      | Returns hook state                                   |

**Key mapping:**

| Key            | Virtual Key     | Event |
|----------------|-----------------|-------|
| `;` / `:`      | `VK_OEM_1`      | Dit   |
| Left Ctrl      | `VK_LCONTROL`   | Dit   |
| `+` / `=`      | `VK_OEM_PLUS`   | Dah   |
| Right Ctrl     | `VK_RCONTROL`   | Dah   |

Duplicate press events are suppressed by tracking previous state.

---

### configuration – Configuration

**Files:** `configuration/Configuration.h`, `configuration/Configuration.cpp`

Persistent JSON configuration stored in `configuration.json` in the working directory.

#### Configuration keys

| Key                     | Constant                    | Type     | Description                      |
|-------------------------|-----------------------------|----------|----------------------------------|
| `amplitude`             | `CFG_AMPLITUDE`             | `double` | Audio amplitude                  |
| `frequency`             | `CFG_FREQUENCY`             | `double` | Tone frequency                   |
| `wpm`                   | `CFG_WPM`                   | `int`    | Keyer speed                      |
| `farnsWorth`            | `CFG_FARNSWORTH`            | `int`    | Farnsworth speed                 |
| `selected_audio_device` | `CFG_SELECTED_AUDIO_DEVICE` | `int`    | Audio device index               |
| `mode`                  | `CFG_MODE`                  | `int`    | Keyer mode                       |
| `commout`               | `CFG_COMM_OUT`              | `int`    | Selected COM port index (output) |
| `commin`                | `CFG_COMM_IN`               | `int`    | Selected COM port index (input)  |
| `device-usb`            | `UsbDevice::CONFIG_NAME`    | `object` | Last known USB device            |

#### Static API

```cpp
Configuration::putValueInt(key, value);
Configuration::putValueDouble(key, value);
Configuration::putValueBool(key, value);
Configuration::putValueString(key, value);
Configuration::putObject(key, QJsonObject);

Configuration::getValueInt(key);
Configuration::getValueDouble(key);
Configuration::getValueBool(key);
Configuration::getValueString(key);
Configuration::getValue(key);        // returns QJsonObject*

Configuration::removeValue(key);
```

All read-modify-write operations use a full JSON load + rewrite cycle to preserve other keys.

---

### utils – Interfaces, Logger & Utilities

#### IDitDah (`utils/IDitDah.h`)

Pure virtual interface for components that receive paddle/key input.

```cpp
virtual void onDit(bool pressed)      = 0;
virtual void onDah(bool pressed)      = 0;
virtual void onStraight(bool pressed) = 0;
```

Implemented by: `Keyer`, `Keyboard`.

#### IKeyerCW (`utils/IKeyerCW.h`)

Pure virtual interface for components that produce CW output, plus static PARIS timing helpers.

```cpp
virtual void runCW(KeyerItem item, int duration) = 0;
virtual void startRunCw() = 0;   // straight key press
virtual void stopRunCw()  = 0;   // straight key release
```

Implemented by: `Sound`, `SerialComm`, `CwDecoder`.

**KeyerItem enum:**

| Value                 | Hex   | Description              |
|-----------------------|-------|--------------------------|
| `DIT`                 | `0x1` | Short element (dot)      |
| `DAH`                 | `0x2` | Long element (dash)      |
| `INTER_ELEMENT_SPACE` | `0x3` | Space between elements   |
| `LETTER_SPACE`        | `0x4` | Space between letters    |
| `WORD_SPACE`          | `0x5` | Space between words      |

**PARIS timing base:** `1200 ms / WPM` per dit.

#### Logger (`utils/Logger.h`)

Lightweight stream-based logger. Uses `std::cerr` for output.

```cpp
log(L_DEBUG) << "Message " << value;
log(L_INFO)  << "Info";
log(L_WARNING) << "Warning";
log(L_ERROR)   << "Error";
```

| Level     | Value |
|-----------|-------|
| `L_ERROR` | 0     |
| `L_WARNING`| 1    |
| `L_INFO`  | 2     |
| `L_DEBUG` | 3     |

Global `loglevel` variable (set in `main.cpp`) filters output: messages with level > `loglevel` are discarded at compile time via the `log()` macro. Default: `L_INFO`.

#### Utils (`utils/Utils.h/.cpp`)

| Method                      | Description                                |
|-----------------------------|--------------------------------------------|
| `Utils::sleepFor(int ms)`   | Sleeps the current thread for `ms` ms      |

Implemented via `usleep(ms * 1000)`.

---

## Default Parameters

| Parameter     | Value     |
|---------------|-----------|
| WPM           | 25        |
| Farnsworth    | 25        |
| Sample Rate   | 44 100 Hz |
| Frequency     | 650 Hz    |
| Amplitude     | 0.5       |
| Attack        | 10 ms (`0.01 s`) |
| Release       | 9 ms  (`0.009 s`) |
| Mode          | Iambic B  |

---

## CW Timing (PARIS Standard)

Based on the word **PARIS** = 50 units; one unit = one dit:

| Element             | Formula                    |
|---------------------|----------------------------|
| Dit                 | `1200 ms / WPM`            |
| Dah                 | `3 × Dit`                  |
| Inter-element space | `1 × Dit`                  |
| Letter space        | `3 × Dit`                  |
| Word space          | `7 × Dit`                  |

Examples:

| WPM | Dit   | Dah   |
|-----|-------|-------|
| 15  | 80 ms | 240 ms|
| 20  | 60 ms | 180 ms|
| 25  | 48 ms | 144 ms|
| 30  | 40 ms | 120 ms|

---

## Keyer Modes

| Mode       | Value | Behavior                                                          |
|------------|-------|-------------------------------------------------------------------|
| `ULTIMATIC`| `0x1` | Squeeze repeats the last-pressed paddle                           |
| `IAMBIC_A` | `0x2` | Squeeze alternates dit/dah; stops immediately on release          |
| `IAMBIC_B` | `0x3` | Like A but completes one more element after both paddles released |

---

## Supported Input Methods

| Method                  | Class              | Trigger                        |
|-------------------------|--------------------|--------------------------------|
| USB Paddle (libusb)     | `UsbDevice`        | Left/right USB interrupt bytes |
| Keyboard (fallback)     | `KeyboardListener` | Low-level Windows keyboard hook|
| N1MM Logger / serial    | `N1MMProxy`        | DSR line change on COM port    |

**Key mappings:**

| Input        | Dit key          | Dah key          |
|--------------|------------------|------------------|
| USB paddle   | Left click byte  | Right click byte |
| Keyboard     | `;` or L-Ctrl    | `+` or R-Ctrl    |
| Keyboard out | `VK_OEM_1` (`;`) | `VK_OEM_PLUS` (`+`) |

---

## Dependencies

| Library         | Usage                                              |
|-----------------|----------------------------------------------------|
| Qt 6            | GUI, QML, Multimedia, JSON, Audio                  |
| libusb          | USB HID paddle reading (interrupt transfers)       |
| hidapi          | HID device access (alternative, `HidDevice`)       |
| Windows API     | Serial port (DTR/DSR), keyboard hook, registry     |
