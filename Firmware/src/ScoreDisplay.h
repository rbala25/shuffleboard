#include <stdint.h>
#include "Puck.h"
#include "Graphics.h"

// Pin assignments

//  PB8  = MOSI/SER  - shared with ST7735 SPI MOSI  (hardware SPI)
//  PB9  = SCLK      - shared with ST7735 SPI CLK   (hardware SPI)
//  PB16 = LATCH     - 595 RCK, dedicated GPIO
//
//  U6 QH* (pin 9) → U8 SER (pin 14)  [daisy chain]
//  U6 drives U4 (display 1 — Red player)
//  U8 drives U5 (display 2 — Blue player)
//
//  Segment wiring (both displays identical per schematic):
//    595 QA → R_x → Display A (pin 7)
//    595 QB → R_x → Display B (pin 6)
//    595 QC → R_x → Display C (pin 4)
//    595 QD → R_x → Display D (pin 2)
//    595 QE → R_x → Display E (pin 1)
//    595 QF → R_x → Display F (pin 9)
//    595 QG → R_x → Display G (pin 10)
//    595 QH → R_x → Display DP(pin 5)
//    Display COM pins 3 & 8 → GND  (common cathode)
//
//  SPI arbitration: The ST7735 driver owns the SPI peripheral. We piggyback on it.
//    after the LCD finishes a transaction, we load our two bytes and
//    pulse only PB16 (RCLK). The 595's OE is tied to GND, so outputs
//    hold their last latched value between updates automatically.

#define SEG_SPI_BASE   SPI1  

#define SEG_LATCH_PORT GPIOB
#define SEG_LATCH_PIN  (1U << 16)

#define SCORE_LCD_X_RED   4    // pixel x, red
#define SCORE_LCD_X_BLUE  110  // pixel x, blue
#define SCORE_LCD_Y       138  // pixel y 
#define SCORE_COLOR_RED   ST7735_Color565(255, 20,  20)
#define SCORE_COLOR_BLUE  ST7735_Color565(20,  110, 255)

void scoreDisplay_init(void);
void updateScoreDisplay(uint8_t redScore, uint8_t blueScore);
void drawWinnerBanner(uint8_t redScore, uint8_t blueScore, uint8_t lang);
void calculateScore(uint8_t* redScore, uint8_t* blueScore, Puck* arr);