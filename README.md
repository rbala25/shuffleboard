# Shuffleboard
 
60 FPS Multiplayer shuffleboard built on bare-metal C/C++ for the TM4C123 — no OS, no shortcuts. ECE319H Final Project.
 
---
 
## Highlights
 
**Comparator + FET Input Detection** — Puck sensing uses a custom analog comparator circuit driving an N-channel FET, with hardware debouncing and firmware-level input validation. Fast, noise-resistant, and doesn't rely on polling.
 
**Custom DAC + Audio** — Sound effects generated through an R-2R resistor ladder DAC feeding a discrete speaker amplifier — no audio IC, just resistors and math.
 
**SPI Display Pipeline** — ST7735 TFT screen handles all game state rendering and animations via SPI. Score output runs in parallel on 7-segment displays driven by 74HC595 shift registers.
 
**Mixed-Signal PCB** — Custom board with split digital/analog ground planes, decoupling for stable SPI, and an integrated comparator/FET sensing front-end alongside the DAC output stage.
 
---
 
## Stack
 
| Layer | Tech |
|---|---|
| MCU | TM4C123GXL (ARM Cortex-M4) |
| Language | C/C++, bare-metal |
| Display | ST7735 TFT over SPI |
| Score Display | 7-segment + 74HC595 shift registers |
| Audio | R-2R DAC + speaker amp |
| Input | Analog comparator + N-FET + hardware debounce |
 
---
 
Developed for **ECE319H — Introduction to Embedded Systems (Honors)** at UT Austin.
 
