#include "ScoreDisplay.h"
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/LaunchPad.h"
#include "Graphics.h"
#include "images/images.h"

//  SPI is MSB-first: bit7 sent first → reaches QH; bit0 sent last → reaches QA
//  Wiring: QA→A, QB→B, QC→C, QD→D, QE→E, QF→F, QG→G, QH→DP
//  So: bit0=A, bit1=B, bit2=C, bit3=D, bit4=E, bit5=F, bit6=G, bit7=DP
//
//      AAA
//     F   B
//     F   B
//      GGG
//     E   C
//     E   C
//      DDD  [DP]
//
static const uint8_t SEG7[10] = {
    0x3F,  // 0: A B C D E F
    0x06,  // 1:   B C
    0x5B,  // 2: A B   D E   G
    0x4F,  // 3: A B C D     G
    0x66,  // 4:   B C     F G
    0x6D,  // 5: A   C D   F G
    0x7D,  // 6: A   C D E F G
    0x07,  // 7: A B C
    0x7F,  // 8: A B C D E F G
    0x6F,  // 9: A B C D   F G
};

static uint8_t s_red  = 0;
static uint8_t s_blue = 0;

static void spi_send_byte(uint8_t data) {
    while (!(SEG_SPI_BASE->STAT & SPI_STAT_TNF_MASK)); //wait for tx fifo not full
    SEG_SPI_BASE->TXDATA = data;
    while (SEG_SPI_BASE->STAT & SPI_STAT_BUSY_MASK);
}

static void latch_pulse(void) {
    GPIOB->DOUTSET31_0 = SEG_LATCH_PIN;
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); // 5 NOPs = ~62.5ns, covers 40ns min at 3.3V
    GPIOB->DOUTCLR31_0 = SEG_LATCH_PIN;
}

// Push both score bytes to the daisy-chained 595s
// U8 data first then U6 data
static void push_to_595(uint8_t redByte, uint8_t blueByte) {
    spi_send_byte(blueByte);  
    spi_send_byte(redByte);  
    latch_pulse();
}

void scoreDisplay_init(void) {
    // PB16 latch pin - configure IOMUX as GPIO, then set as output, start low
    IOMUX->SECCFG.PINCM[PB16INDEX] = 0x00000081;
    GPIOB->DOE31_0  |=  SEG_LATCH_PIN;
    GPIOB->DOUTCLR31_0 = SEG_LATCH_PIN;

    push_to_595(SEG7[0], SEG7[0]);
    updateScoreDisplay(0, 0);
}

static void drawBigString(int16_t x, int16_t y, const char *str, uint16_t fg, uint16_t bg) {
    for (int i; *str; str++, x += 12)
        ST7735_DrawCharS(x, y, *str, fg, bg, 2);
}

static void drawSmallString(int16_t x, int16_t y, const char *str, uint16_t fg, uint16_t bg) {
    for (int i; *str; str++, x += 6)
        ST7735_DrawCharS(x, y, *str, fg, bg, 1);
}

void drawWinnerBanner(uint8_t redScore, uint8_t blueScore, uint8_t lang) {
    uint16_t bannerColor;
    const char *line1;
    int16_t textX;

    if (lang == 1) {  
        if (redScore > blueScore) {
            bannerColor = SCORE_COLOR_RED;
            line1 = "ROUGE WIN!";  
            textX = 4;
        } else if (blueScore > redScore) {
            bannerColor = SCORE_COLOR_BLUE;
            line1 = "BLEU WIN! "; 
            textX = 4;
        } else {
            bannerColor = ST7735_Color565(140, 60, 200);
            line1 = "EGALITE!";
            textX = 16;
        }
    } else {  // english
        if (redScore > blueScore) {
            bannerColor = SCORE_COLOR_RED;
            line1 = "RED WINS!";
            textX = 10;
        } else if (blueScore > redScore) {
            bannerColor = SCORE_COLOR_BLUE;
            line1 = "BLUE WINS!";
            textX = 4;
        } else {
            bannerColor = ST7735_Color565(140, 60, 200);
            line1 = "  TIE!  ";
            textX = 4;
        }
    }

    uint16_t white = ST7735_Color565(255, 255, 255);
    ST7735_FillRect(4, 56, 120, 54, bannerColor);
    drawBigString(textX, 63, line1, white, bannerColor);
    const char *sub = (lang == 1) ? "Appuyez Reset" : "Press Reset";
    drawSmallString(lang == 1 ? 15 : 31, 91, sub, white, bannerColor);
}

void updateScoreDisplay(uint8_t redScore, uint8_t blueScore) {
    if (redScore  > 9) redScore  = 9;
    if (blueScore > 9) blueScore = 9;

    s_red  = redScore;
    s_blue = blueScore;

    push_to_595(SEG7[redScore], SEG7[blueScore]);

    // draw over background pixels so there's no black box
    char rBuf[2] = { (char)('0' + redScore),  '\0' };
    char bBuf[2] = { (char)('0' + blueScore), '\0' };

    int16_t rX = SCORE_LCD_X_RED,  rY = SCORE_LCD_Y;
    int16_t bX = SCORE_LCD_X_BLUE, bY = SCORE_LCD_Y;

    ST7735_DrawCharS(rX, rY, rBuf[0], SCORE_COLOR_RED,  (uint16_t)getBackgroundPixel(rX, rY), 2);
    ST7735_DrawCharS(bX, bY, bBuf[0], SCORE_COLOR_BLUE, (uint16_t)getBackgroundPixel(bX, bY), 2);
}

static uint8_t colorToPoints(uint16_t c) {
    if (c == 0x14CF) return 3;  //green
    if (c == 0x23FF) return 2;  // orange 
    if (c == 0xA380) return 1;  // blue 
    return 0;
}

static uint8_t zonePoints(int16_t x, int16_t y) {
    uint16_t c = (uint16_t)getBackgroundPixel(x, y);
    if (c != 0xFFFF) return colorToPoints(c);
    // white = boundary
    // check neighbors and take highest score
    uint8_t best = 0;
    const int8_t dx[] = {1,-1,0,0};
    const int8_t dy[] = {0,0,1,-1};
    for (int i = 0; i < 4; i++) {
        uint16_t n = (uint16_t)getBackgroundPixel(x+dx[i], y+dy[i]);
        uint8_t pts = colorToPoints(n);
        if (pts > best) best = pts;
    }
    return best;
}

void calculateScore(uint8_t* redScore, uint8_t* blueScore, Puck* arr) {
    uint8_t newRed = 0, newBlue = 0;
    for (int i = 0; i < 6; i++) {
        Puck& p = arr[i];
        if(p.x > 127 || p.y > 159) {
          continue;
        }
        int16_t cx = (int16_t)p.x + 7;
        int16_t cy = (int16_t)p.y + 7;
        uint8_t pts = zonePoints(cx, cy);
        if (p.color == "Red") {
            newRed  += pts;
        } else {
            newBlue += pts;
        }
    }
    *redScore  = newRed;
    *blueScore = newBlue;
}