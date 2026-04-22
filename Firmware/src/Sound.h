// Sound.h
// 12-bit DAC sound output on PA15
// Rishi Bala, Anh-Vu Nguyen
#ifndef SOUND_H
#define SOUND_H
#include <stdint.h>

// Initialize 12-bit DAC and SysTick at 11025 Hz; no sound plays until Sound_Start
void Sound_Init(void);

// Set active sound buffer and re-enable output; plays once then stops
// pt - pointer to array of 12-bit PCM samples, count - number of samples
// gain - integer amplitude multiplier (1 = unity, 2 = double, etc.)
void Sound_Start(const uint16_t *pt, uint32_t count, int32_t gain);

void Sound_Shoot(void);     // puck firing sound
void Sound_Collision(void); // puck/wall collision sound
void Sound_Winner(void);    // winner sound

#endif
