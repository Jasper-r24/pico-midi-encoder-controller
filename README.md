# MIDI Encoder Controller

[Deutsch](README.de.md)

A USB MIDI controller with four endless rotary encoders, built around a Raspberry Pi Pico. It shows up on your computer as a class-compliant USB MIDI device called "Encoder Board", so you need no driver, no serial bridge and no extra software.

I built it to control moving head parameters (pan, tilt and more) in lighting software next to an Akai APC Mini. It works with anything that accepts MIDI CC messages: lighting software, DAWs, VJ tools.

The 3D printed case, lid and knobs: [MakerWorld link]

## Features

- 4 endless encoders, each sends its own MIDI CC (20 to 23) on MIDI channel 2
- Relative MIDI values, so the controlled parameter never jumps when you start turning
- Press an encoder to toggle fine mode for that encoder:
  - Fine mode: exactly 1 MIDI value per detent
  - Normal mode: 4 to 16 MIDI values per detent, more the faster you turn
- Encoder decoding that tolerates contact bounce, works with cheap KY-040 modules
- Native USB MIDI through the Pico

## Hardware

- 1x Raspberry Pi Pico (RP2040)
- 4x KY-040 rotary encoder module (5 pins: CLK, DT, SW, +, GND)
- 1x USB cable with a micro-USB plug
- Printed case, lid and 4 knobs
- Screws, hookup wire, solder

## Wiring

Power the encoders from the Pico's 3V3(OUT) pin (pin 36). Never use VBUS or 5 V: the Pico's GPIO pins are not 5 V tolerant.

| Encoder | CLK | DT | SW | MIDI CC |
|---|---|---|---|---|
| 1 | GP0 (pin 1) | GP1 (pin 2) | GP2 (pin 4) | 20 |
| 2 | GP3 (pin 5) | GP4 (pin 6) | GP5 (pin 7) | 21 |
| 3 | GP6 (pin 9) | GP7 (pin 10) | GP8 (pin 11) | 22 |
| 4 | GP9 (pin 12) | GP10 (pin 14) | GP11 (pin 15) | 23 |

- All four "+" pins go to 3V3(OUT) (pin 36), all four GND pins to any Pico GND pin.
- Pin 37 (3V3_EN) sits between GND (pin 38) and 3V3 (pin 36). Do not connect it to anything.
- The signal pins use the Pico's internal pull-ups, no extra resistors are needed.

## Firmware

1. Install the Arduino IDE. In the preferences, add this URL under "Additional boards manager URLs":
   `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`
2. Install the board package "Raspberry Pi Pico/RP2040" (by Earle Philhower) in the boards manager.
3. Install the libraries "Adafruit TinyUSB Library" and "MIDI Library" (FortySevenEffects).
4. Select Tools > Board > "Raspberry Pi Pico" and set Tools > USB Stack to "Adafruit TinyUSB". The default stack does not work for this sketch.
5. Open `firmware/MIDI_Encoder_Controller/MIDI_Encoder_Controller.ino` and upload it. For the very first upload, hold the BOOTSEL button while plugging the Pico into USB.
6. The Pico now appears as the MIDI device "Encoder Board".

## Usage

Map each encoder to a parameter in your software and set the mapping to relative mode. If your software offers several relative modes, pick the one centered on 64: the value 65 means one step up, 63 means one step down.

Pressing an encoder toggles fine mode for that encoder only. Fine mode resets when the Pico loses power, and there is no indicator for it on the hardware.

## Tuning

All settings are constants at the top of the sketch:

| Constant | Meaning | Default |
|---|---|---|
| `STEPS_PER_DETENT` | Quadrature transitions per click. Use 2 if only every second click reacts, 8 if one click triggers several. | 4 |
| `NORMAL_BASE_BURST` | MIDI values per click in normal mode when turning slowly | 4 |
| `NORMAL_MAX_BURST` | MIDI values per click in normal mode when turning fast | 16 |
| `FAST_MS` / `SLOW_MS` | Time between clicks that counts as fast / slow | 15 / 150 |
| `midiChannel` | MIDI channel (the sketch uses 2 so it does not collide with controllers on channel 1) | 2 |

The CC numbers and pins are in the `encoders` array.

## Troubleshooting

- Wrong turning direction: swap the values 65 and 63 in the sketch.
- Still jittery: solder a 100 nF capacitor from CLK to GND and from DT to GND on the encoder module.
- The Pico does not show up as a port: hold BOOTSEL while plugging it in.
- The MIDI device does not appear: check that Tools > USB Stack is set to "Adafruit TinyUSB".

## How it works

Every loop the sketch reads CLK and DT of each encoder and looks the transition up in a quadrature table (+1, -1 or 0 for invalid changes). Contact bounce flips the counter back and forth and cancels itself out. Once the counter reaches `STEPS_PER_DETENT`, one detent counts. In fine mode that sends one relative CC message, in normal mode it sends a burst whose size depends on the time since the last detent.

## Credits

- [arduino-pico](https://github.com/earlephilhower/arduino-pico) by Earle Philhower
- [Adafruit TinyUSB Library](https://github.com/adafruit/Adafruit_TinyUSB_Arduino)
- [MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library) by FortySevenEffects
