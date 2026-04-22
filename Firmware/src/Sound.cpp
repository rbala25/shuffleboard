// Sound.cpp
// Runs on MSPM0G3507
// 12-bit DAC sound output on PA15 at 11025 Hz via SysTick
// Rishi Bala, Anh-Vu Nguyen
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "Sound.h"
#include "shoot.txt"
#include "collision.txt"
#include "winner.txt"
#include "../inc/DAC.h"

static const uint16_t *SoundPtr = 0;
static const uint16_t *SoundEnd = 0;
static int32_t SoundGain = 1;

void Sound_Init(void){
  DAC_Init();
  SoundPtr = 0;
  SoundEnd = 0;
  SoundGain = 1;
  SysTick->CTRL = 0;
  SysTick->LOAD = (80000000/11025) - 1;
  SysTick->VAL  = 0;
  SCB->SHP[1] = (SCB->SHP[1] & ~0xC0000000) | (2U << 30);
  SysTick->CTRL = 0x00000007;
}

extern "C" void SysTick_Handler(void);
void SysTick_Handler(void){
  if(SoundPtr >= SoundEnd){
    DAC_Out(2048);
    return;
  }
  int32_t s = (int32_t)(*SoundPtr);
  int32_t out = 2048 + SoundGain * (s - 2048);
  if(out > 4095) out = 4095;
  if(out < 0) out = 0;
  DAC_Out((uint32_t)out);
  SoundPtr++;
}

void Sound_Start(const uint16_t *pt, uint32_t count, int32_t gain){
  SoundGain = gain;
  SoundPtr = pt;
  SoundEnd = pt + count;
}

void Sound_Shoot(void){
  Sound_Start(shoot, sizeof(shoot)/sizeof(shoot[0]), 1);
}

void Sound_Collision(void){
  Sound_Start(collision, sizeof(collision)/sizeof(collision[0]), 1);
}

void Sound_Winner(void){
  Sound_Start(winner, sizeof(winner)/sizeof(winner[0]), 1);
}
