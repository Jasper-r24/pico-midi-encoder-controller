# MIDI Encoder Controller

[English](README.md)

Ein USB-MIDI-Controller mit vier Endlos-Drehgebern, gebaut mit einem Raspberry Pi Pico. Er meldet sich am Rechner als class-compliant USB-MIDI-Gerät "Encoder Board", du brauchst also keinen Treiber, keine serielle Brücke und keine Zusatzsoftware.

Gebaut habe ich ihn, um Moving-Head-Parameter (Pan, Tilt und mehr) in Lichtsoftware neben einem Akai APC Mini zu steuern. Er funktioniert mit allem, was MIDI-CC-Nachrichten annimmt: Lichtsoftware, DAWs, VJ-Tools.

Das 3D-gedruckte Gehäuse, Deckel und Räder: [MakerWorld-Link]

## Funktionen

- 4 Endlos-Encoder, jeder sendet seinen eigenen MIDI-CC (20 bis 23) auf MIDI-Kanal 2
- Relative MIDI-Werte, der gesteuerte Parameter springt beim Losdrehen also nie
- Encoder drücken schaltet den Fine-Modus für diesen Encoder um:
  - Fine-Modus: genau 1 MIDI-Wert pro Rastklick
  - Normalmodus: 4 bis 16 MIDI-Werte pro Rastklick, je schneller du drehst, desto mehr
- Prellfester Encoder-Decoder, funktioniert auch mit günstigen KY-040-Modulen
- Natives USB-MIDI über den Pico

## Hardware

- 1x Raspberry Pi Pico (RP2040)
- 4x KY-040 Drehgeber-Modul (5 Pins: CLK, DT, SW, +, GND)
- 1x USB-Kabel mit Micro-USB-Stecker
- Gedrucktes Gehäuse, Deckel und 4 Räder
- Schrauben, Schaltdraht, Lötzinn

## Verkabelung

Die Encoder aus dem 3V3(OUT)-Pin des Pico (Pin 36) versorgen. Niemals VBUS oder 5 V nehmen: Die GPIO-Pins des Pico sind nicht 5-V-tolerant.

| Encoder | CLK | DT | SW | MIDI-CC |
|---|---|---|---|---|
| 1 | GP0 (Pin 1) | GP1 (Pin 2) | GP2 (Pin 4) | 20 |
| 2 | GP3 (Pin 5) | GP4 (Pin 6) | GP5 (Pin 7) | 21 |
| 3 | GP6 (Pin 9) | GP7 (Pin 10) | GP8 (Pin 11) | 22 |
| 4 | GP9 (Pin 12) | GP10 (Pin 14) | GP11 (Pin 15) | 23 |

- Alle vier "+" an 3V3(OUT) (Pin 36), alle vier GND an einen beliebigen GND-Pin des Pico.
- Pin 37 (3V3_EN) liegt zwischen GND (Pin 38) und 3V3 (Pin 36). Dort nichts anschließen.
- Die Signalpins nutzen die internen Pull-ups des Pico, zusätzliche Widerstände sind nicht nötig.

## Firmware

1. Arduino IDE installieren. In den Voreinstellungen unter "Zusätzliche Boardverwalter-URLs" diese URL eintragen:
   `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`
2. Im Boardverwalter das Paket "Raspberry Pi Pico/RP2040" (von Earle Philhower) installieren.
3. Die Bibliotheken "Adafruit TinyUSB Library" und "MIDI Library" (FortySevenEffects) installieren.
4. Werkzeuge > Board > "Raspberry Pi Pico" wählen und Werkzeuge > USB Stack auf "Adafruit TinyUSB" stellen. Der Standard-Stack funktioniert mit diesem Sketch nicht.
5. `firmware/MIDI_Encoder_Controller/MIDI_Encoder_Controller.ino` öffnen und hochladen. Beim allerersten Upload die BOOTSEL-Taste gedrückt halten, während der Pico per USB angeschlossen wird.
6. Der Pico erscheint jetzt als MIDI-Gerät "Encoder Board".

## Benutzung

Jeden Encoder in deiner Software einem Parameter zuweisen und die Zuweisung auf Relativ stellen. Wenn die Software mehrere Relativ-Modi anbietet, nimm den mit Mittelwert 64: Der Wert 65 bedeutet einen Schritt hoch, 63 einen Schritt runter.

Encoder drücken schaltet den Fine-Modus nur für diesen Encoder um. Der Fine-Modus wird zurückgesetzt, wenn der Pico stromlos wird, und es gibt keine Anzeige dafür an der Hardware.

## Anpassen

Alle Einstellungen sind Konstanten oben im Sketch:

| Konstante | Bedeutung | Standard |
|---|---|---|
| `STEPS_PER_DETENT` | Quadratur-Zustandswechsel pro Klick. 2 nehmen, wenn nur jeder zweite Klick reagiert, 8, wenn ein Klick mehrfach auslöst. | 4 |
| `NORMAL_BASE_BURST` | MIDI-Werte pro Klick im Normalmodus bei langsamem Drehen | 4 |
| `NORMAL_MAX_BURST` | MIDI-Werte pro Klick im Normalmodus bei schnellem Drehen | 16 |
| `FAST_MS` / `SLOW_MS` | Zeit zwischen Klicks, die als schnell / langsam gilt | 15 / 150 |
| `midiChannel` | MIDI-Kanal (der Sketch nutzt 2, damit es keine Überschneidung mit Controllern auf Kanal 1 gibt) | 2 |

Die CC-Nummern und Pins stehen im Array `encoders`.

## Fehlersuche

- Falsche Drehrichtung: die Werte 65 und 63 im Sketch tauschen.
- Immer noch unruhig: einen 100-nF-Kondensator von CLK nach GND und von DT nach GND am Encoder-Modul anlöten.
- Der Pico taucht nicht als Port auf: beim Einstecken BOOTSEL gedrückt halten.
- Das MIDI-Gerät erscheint nicht: prüfen, ob Werkzeuge > USB Stack auf "Adafruit TinyUSB" steht.

## So funktioniert es

In jedem Durchlauf liest der Sketch CLK und DT jedes Encoders und schlägt den Übergang in einer Quadratur-Tabelle nach (+1, -1 oder 0 für ungültige Wechsel). Kontaktprellen lässt den Zähler hin und her springen und hebt sich selbst auf. Sobald der Zähler `STEPS_PER_DETENT` erreicht, zählt ein Rastklick. Im Fine-Modus wird dafür eine relative CC-Nachricht gesendet, im Normalmodus ein Schwung, dessen Größe von der Zeit seit dem letzten Rastklick abhängt.

## Danksagung

- [arduino-pico](https://github.com/earlephilhower/arduino-pico) von Earle Philhower
- [Adafruit TinyUSB Library](https://github.com/adafruit/Adafruit_TinyUSB_Arduino)
- [MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library) von FortySevenEffects
