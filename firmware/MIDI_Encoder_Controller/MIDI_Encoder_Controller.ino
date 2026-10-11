/*
  MIDI Encoder Controller - 4 endless encoders as a native USB MIDI device
  Board: Raspberry Pi Pico (RP2040), arduino-pico core (Earle Philhower)

  Arduino IDE setup:
    Tools > Board      = "Raspberry Pi Pico"
    Tools > USB Stack  = "Adafruit TinyUSB"
    Libraries          = "Adafruit TinyUSB Library", "MIDI Library" (FortySevenEffects)

  Wiring (KY-040 modules). Power the "+" pin from 3V3(OUT) (Pico pin 36), NEVER from VBUS/5V.
    Encoder 1: CLK=GP0  DT=GP1   SW=GP2   -> MIDI CC 20
    Encoder 2: CLK=GP3  DT=GP4   SW=GP5   -> MIDI CC 21
    Encoder 3: CLK=GP6  DT=GP7   SW=GP8   -> MIDI CC 22
    Encoder 4: CLK=GP9  DT=GP10  SW=GP11  -> MIDI CC 23

  Behaviour:
    - Relative MIDI CC, centered on 64: 65 = one step right, 63 = one step left.
      Set the target parameter in your software to "relative" mode.
    - Pressing an encoder toggles FINE mode for that encoder:
        fine   -> exactly 1 MIDI value per detent
        normal -> 4..16 MIDI values per detent, more the faster you turn
*/

#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

// State changes per detent: 4 = typical KY-040, use 2 if only every second click reacts
const int STEPS_PER_DETENT = 4;

// Quadrature lookup table. Index = (oldState << 2) | newState, state = (CLK << 1) | DT.
// Counts every valid transition, so contact bounce cancels itself out.
const int8_t encTable[16] = {0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0};

struct Encoder {
  uint8_t pinClk, pinDt, pinSw;
  uint8_t ccNumber;
  uint8_t oldAB = 3;
  int counter = 0;
  bool lastSw = true;
  unsigned long lastSwChange = 0;
  bool fineMode = false;
  unsigned long lastTurnTime = 0;
};

Encoder encoders[4] = {
  { 0,  1,  2, 20},
  { 3,  4,  5, 21},
  { 6,  7,  8, 22},
  { 9, 10, 11, 23}
};

// MIDI channel 2, so it does not collide with controllers that default to channel 1
const byte midiChannel = 2;

// Normal mode: MIDI values per detent, depending on turning speed
const int NORMAL_BASE_BURST = 4;     // slow turning
const int NORMAL_MAX_BURST  = 16;    // fast turning
const unsigned long FAST_MS = 15;    // time between detents at or below this = full speed-up
const unsigned long SLOW_MS = 150;   // time between detents at or above this = base value

void handleEncoder(Encoder &enc) {
  uint8_t newAB = (digitalRead(enc.pinClk) << 1) | digitalRead(enc.pinDt);
  enc.counter += encTable[(enc.oldAB << 2) | newAB];
  enc.oldAB = newAB;

  int dir = 0;
  if (enc.counter >= STEPS_PER_DETENT)       { dir = 1;  enc.counter = 0; }
  else if (enc.counter <= -STEPS_PER_DETENT) { dir = -1; enc.counter = 0; }

  if (dir != 0) {
    byte value = (dir > 0) ? 65 : 63;   // if the direction is reversed, swap 65 and 63

    int burst;
    if (enc.fineMode) {
      burst = 1;
    } else {
      unsigned long delta = millis() - enc.lastTurnTime;
      if (delta > SLOW_MS) delta = SLOW_MS;
      if (delta < FAST_MS) delta = FAST_MS;
      burst = map((long)delta, (long)FAST_MS, (long)SLOW_MS, NORMAL_MAX_BURST, NORMAL_BASE_BURST);
    }

    for (int i = 0; i < burst; i++) {
      MIDI.sendControlChange(enc.ccNumber, value, midiChannel);
    }
    enc.lastTurnTime = millis();
  }

  // Encoder push button: toggle fine mode (30 ms debounce)
  bool swState = digitalRead(enc.pinSw);
  if (swState != enc.lastSw && millis() - enc.lastSwChange > 30) {
    enc.lastSwChange = millis();
    enc.lastSw = swState;
    if (swState == LOW) enc.fineMode = !enc.fineMode;
  }
}

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(encoders[i].pinClk, INPUT_PULLUP);
    pinMode(encoders[i].pinDt, INPUT_PULLUP);
    pinMode(encoders[i].pinSw, INPUT_PULLUP);
    encoders[i].oldAB = (digitalRead(encoders[i].pinClk) << 1) | digitalRead(encoders[i].pinDt);
  }

  usb_midi.setStringDescriptor("Encoder Board");
  MIDI.begin(MIDI_CHANNEL_OMNI);
  while (!TinyUSBDevice.mounted()) delay(1);
}

void loop() {
  for (int i = 0; i < 4; i++) {
    handleEncoder(encoders[i]);
  }
  MIDI.read();
}
