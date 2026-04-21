#include "Graphics.h"

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "images/images.h"
#include "Puck.h"


static void drawStr(int16_t x, int16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t size) {
    for (int i; *s; s++, x += 6 * size)
        ST7735_DrawCharS(x, y, *s, fg, bg, size);
}

void drawLanguageScreen(uint8_t lang) {
    uint16_t black  = ST7735_BLACK;
    uint16_t white  = ST7735_Color565(255, 255, 255);
    uint16_t yellow = ST7735_Color565(255, 220, 0);
    uint16_t gray   = ST7735_Color565(110, 110, 110);

    ST7735_FillRect(0, 0, 128, 160, black);

    drawStr(16, 18, "LANGUAGE", white, black, 2);

    drawStr(10, 60, lang == 0 ? "> ENGLISH" : "  ENGLISH",
            lang == 0 ? yellow : gray, black, 2);

    drawStr(16, 90, lang == 1 ? "> FRENCH" : "  FRENCH",
            lang == 1 ? yellow : gray, black, 2);

    const char *ins1 = (lang == 1) ? "RESET: changer" : "RESET: toggle";
    const char *ins2 = (lang == 1) ? "SHOOT: jouer"   : "SHOOT: start";
    // center
    int16_t i1len = 0; for(const char *c = ins1; *c; c++) i1len++;
    int16_t i2len = 0; for(const char *c = ins2; *c; c++) i2len++;
    drawStr((128 - i1len * 6) / 2, 130, ins1, gray, black, 1);
    drawStr((128 - i2len * 6) / 2, 145, ins2, gray, black, 1);
}

void graphics_init(){
  ST7735_InitPrintf(INITR_BLACKTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
  ST7735_SetRotation(2);
  drawBackground();
}

//Background Functions
int16_t getBackgroundPixel(uint8_t x, uint8_t y) {
  return background[(159-y)*128+x];
}

void drawBackground(){
  for(int i=0;i<128;i++){
    for(int j=0; j<160; j++) {
      ST7735_DrawPixel(i, j, getBackgroundPixel(i, j));
    }
  }
}

#define ARROW_COLOR ST7735_Color565(255, 220, 0)
#define ARROW_MAX_LEN 45

static void arrowLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool erase) {
    int16_t dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1; //horizontal distqnce
    int16_t dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1; //vertical distance
    int16_t err = dx + dy;
    while(true) {
        if(x1 >= 0 && x1 < 128 && y1 >= 0 && y1 < 160) {
            uint16_t c = erase ? getBackgroundPixel(x1, y1) : ARROW_COLOR;
            ST7735_DrawPixel(x1, y1, c);
        }
        if(x1 == x2 && y1 == y2) break;
        int16_t e2 = 2 * err;
        if(e2 >= dy) { err += dy; x1 += sx; }
        if(e2 <= dx) { err += dx; y1 += sy; }
    }
}

void drawArrow(int16_t px, int16_t py, float aim, float power, bool erase) {
    if(power < 0.05f) return;

    int16_t cx = px + 7;
    int16_t cy = py + 7;

    float length = power * ARROW_MAX_LEN;
    float angle  = aim * (3.14159f / 2.0f);  // ±90° range (full 180°)
    float dx = sinf(angle);
    float dy = -cosf(angle);  // negative = toward top of screen

    int16_t tx = cx + (int16_t)(dx * length);
    int16_t ty = cy + (int16_t)(dy * length);

    arrowLine(cx, cy, tx, ty, erase);

    float headLen = length * 0.35f;
    float a1 = angle + 2.618f;  // ±150° from forward
    float a2 = angle - 2.618f;
    arrowLine(tx, ty, tx + (int16_t)(sinf(a1) * headLen), ty - (int16_t)(cosf(a1) * headLen), erase);
    arrowLine(tx, ty, tx + (int16_t)(sinf(a2) * headLen), ty - (int16_t)(cosf(a2) * headLen), erase);
}

void renderPuck(const Puck p){
  int16_t px = (int16_t)p.x, py = (int16_t)p.y;
  int16_t ox = (int16_t)p.oldX, oy = (int16_t)p.oldY;
  bool posChanged = (ox != px || oy != py);
  if(p.isMoving || posChanged){
    for(int i=0;i<p.h;i++){
      for(int j=0;j<p.w;j++){
        if(ox > 127 || oy > 159) {
          continue;
        }
        if((ox+j)<px || (ox+j)>(px+14) || (oy+i)<py || (oy+i)>(py+14))
          ST7735_DrawPixel(ox+j, oy+i, getBackgroundPixel(ox+j, oy+i));
      }
    }
  }
  for(int i=0;i<p.h;i++){
    for(int j=0;j<p.w;j++){
      if(px > 127 || py > 159) {
        continue;
      }
      if(p.image[i*15 + j]==0x4D84){
        ST7735_DrawPixel(px+j, py+i, getBackgroundPixel(px+j, py+i));
      }else{
        ST7735_DrawPixel(px+j, py+i, p.image[(14-i)*15 + j]);
      }
    }
  }
}