// Lab9HMain.cpp
// Runs on MSPM0G3507
// Lab 9 ECE319H
// Anh-Vu Nguyen, Rishi Bala
// Last Modified: January 12, 2026

#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/SlidePot.h"
#include "../inc/DAC.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"
#include "Graphics.h"
#include "Puck.h"
#include "Button.h"
#include "../inc/ADC1.h"
#include "../inc/ADC1.c"
#include "ScoreDisplay.h"
extern "C" void __disable_irq(void);
extern "C" void __enable_irq(void);
extern "C" void TIMG12_IRQHandler(void);

extern volatile uint32_t ShootFlag;
extern volatile uint32_t ResetFlag;
volatile uint32_t SemaphoreFlag = 0;
volatile float aim = 0.0f;
volatile float power = 0.0f;
volatile float start = 0.0f;

volatile uint8_t redScore;
volatile uint8_t blueScore;

void PLL_Init(void){
  Clock_Init80MHz(0);
}

Puck pArray[6] = {
  Puck(60,165,"Red",0), Puck(60,165,"Blue",1),
  Puck(60,165,"Red",2), Puck(60,165,"Blue",3),
  Puck(60,165,"Red",4), Puck(60,165,"Blue",5)
};

uint32_t M=1;
uint32_t Random32(void){ M = 1664525*M+1013904223; return M; }
uint32_t Random(uint32_t n){ return (Random32()>>16)%n; }

SlidePot Sensor(1500,0);

// physics + ADC only
// no game state, no flag clearing
void TIMG12_IRQHandler(void){
  if((TIMG12->CPU_INT.IIDX) == 1){
    GPIOB->DOUTTGL31_0 = GREEN;
    // 1) sample joystick ADC

    float rawAim, rawStart;
    uint8_t vertError;
    ADCin(&rawStart, &rawAim, &power, &vertError);
    aim   = rawAim * 2.0f - 1.0f;
    rawStart = sqrtf(rawStart);
    float newStart = rawStart * 2.0f - 1.0f;
    start = start * 0.5f + newStart * 0.5f; //estimated moving average formula

    // 2) move sprites
    for(int i = 0; i < 6; i++){
      if(pArray[i].isMoving)
        pArray[i].process_Movement();
    }

    // 3) signal main — NO LCD OUTPUT IN INTERRUPT SERVICE ROUTINES
    SemaphoreFlag = 1;
    GPIOB->DOUTTGL31_0 = GREEN;
  }
}

uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}

typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
// const char *Phrases[3][4]={
//   {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
//   {Goodbye_Englifsh,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
//   {Language_English,Language_Spanish,Language_Portuguese,Language_French}
// };

typedef enum { LANGUAGE_SELECT, AIMING, SHOOTING, SCORING, GAME_OVER } GameState_t;
uint8_t language = 0;  // 0=English, 1=French

static bool allStopped(void){
  for(int i = 0; i < 6; i++)
    if(pArray[i].isMoving) return false;
  return true;
}

// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
int main(void){
  __disable_irq();
  PLL_Init();
  LaunchPad_Init();
  graphics_init();
  Sensor.Init();
  ADCinit();
  Button_Init();
  LED_Init();
  Sound_Init();
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26);
  TimerG12_IntArm(2666667, 3);  // 80MHz/2666667 = 30fps
  __enable_irq();
  scoreDisplay_init();
  drawLanguageScreen(language);

  GameState_t state = LANGUAGE_SELECT;
  uint8_t activePuck = 0;  // 0-5
  float prevAim = 0.0f, prevPower = 0.0f;

  while(1){
    while(!SemaphoreFlag){};
    SemaphoreFlag = 0;

    if(ResetFlag){
      ResetFlag = 0;
      ShootFlag = 0;
      if(state == LANGUAGE_SELECT){
        language ^= 1;
        drawLanguageScreen(language);
      } else {
        if(state == AIMING)
          drawArrow(pArray[activePuck].x, pArray[activePuck].y, prevAim, prevPower, true);
        pArray[0] = Puck(60,165,"Red",0);
        pArray[1] = Puck(60,165,"Blue",1);
        pArray[2] = Puck(60,165,"Red",2);
        pArray[3] = Puck(60,165,"Blue",3);
        pArray[4] = Puck(60,165,"Red",4);
        pArray[5] = Puck(60,165,"Blue",5);
        activePuck = 0;
        prevAim = 0.0f; prevPower = 0.0f;
        redScore = 0; blueScore = 0;
        updateScoreDisplay(0, 0);
        drawLanguageScreen(language);
        state = LANGUAGE_SELECT;
      }
    }

    switch(state){

      case LANGUAGE_SELECT:
        if(ShootFlag){
          ShootFlag = 0;
          drawBackground();
          state = AIMING;
        }
        break;

      case AIMING: {
        // erase arrow at current puck position before sliding
        drawArrow(pArray[activePuck].x, pArray[activePuck].y, prevAim, prevPower, true);

        // slide active puck left/right with start pot
        {
          float newX = (start + 1.0f) * 0.5f * 113.0f;
          if(newX < 0.0f) newX = 0.0f;
          if(newX > 113.0f) newX = 113.0f;
          pArray[activePuck].oldX = pArray[activePuck].x;
          pArray[activePuck].oldY = pArray[activePuck].y;
          pArray[activePuck].x = newX;
          pArray[activePuck].y = 125.0f;
        }

        float clampedAim = aim;
        if(clampedAim >  0.90f) clampedAim =  0.90f;
        if(clampedAim < -0.90f) clampedAim = -0.90f;

        // draw arrow at new puck position
        drawArrow(pArray[activePuck].x, pArray[activePuck].y, clampedAim, power, false);
        prevAim   = clampedAim;
        prevPower = power;

        if(ShootFlag){
          ShootFlag = 0;
          if(power > 0.1) {
            // erase arrow, switch state
            drawArrow(pArray[activePuck].x, pArray[activePuck].y, prevAim, prevPower, true);
            float angle = prevAim * (3.14159f / 2.0f);
            pArray[activePuck].set_Velocity(sinf(angle) * prevPower * 12.0f, -cosf(angle) * prevPower * 7.0f);
            prevAim = 0.0f; prevPower = 0.0f;
            Sound_Shoot();
            state = SHOOTING;
          }
        }
        break;
      }

      case SHOOTING:
        if(allStopped()){
          state = SCORING;
        }
        break;

      case SCORING:
        Sound_Collision();
        calculateScore((uint8_t*)&redScore, (uint8_t*)&blueScore, pArray);
        updateScoreDisplay(redScore, blueScore);
        activePuck++;
        if(activePuck >= 6){
          Sound_Winner();
          drawBackground();
          drawWinnerBanner(redScore, blueScore, language);
          state = GAME_OVER;
        } else {
          ShootFlag = 0;
          state = AIMING;
        }
        break;

      case GAME_OVER:
        // waiting for reset 
        break;
    }

    // render pucks every frame
    if(state != GAME_OVER && state != LANGUAGE_SELECT){
      for(int i = 0; i < 6; i++){
        renderPuck(pArray[i]);
      }
    }

    if(state == AIMING){
      pArray[activePuck].oldX = pArray[activePuck].x;
      pArray[activePuck].oldY = pArray[activePuck].y;
    }
  }
}
