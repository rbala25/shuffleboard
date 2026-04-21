/* ADC1.c
 * Students put your names here
 * Modified: put the date here
 * 12-bit ADC input on ADC1 channels 4 (PB17), 5 (PB18), 6 (PB19)
 */
#include <ti/devices/msp/msp.h>
#include "../inc/Clock.h"


void ADCinit(void){
// initialize ADC1 channels 4, 5, 6 in sequence mode
// PB17 = ch4, PB18 = ch5, PB19 = ch6
// software trigger
  ADC1->ULLMEM.GPRCM.RSTCTL = 0xB1000003; // 1) reset
  ADC1->ULLMEM.GPRCM.PWREN = 0x26000001;  // 2) activate
  Clock_Delay(24);                        
  ADC1->ULLMEM.GPRCM.CLKCFG = 0xA9000000; 
  ADC1->ULLMEM.CLKFREQ = 7;               
  ADC1->ULLMEM.CTL0 = 0x03010000;      
  ADC1->ULLMEM.CTL1 = 0x00010000;      
  ADC1->ULLMEM.CTL2 = 0x02000000;        
  ADC1->ULLMEM.MEMCTL[0] = 4;            
  ADC1->ULLMEM.MEMCTL[1] = 5;           
  ADC1->ULLMEM.MEMCTL[2] = 6;           
  ADC1->ULLMEM.SCOMP0 = 0;               
  ADC1->ULLMEM.CPU_INT.IMASK = 0;     
}
void ADCin(volatile float *ch4, volatile float *ch5, volatile float *ch6, uint8_t *vertError){
  // ch4, ch6: 0.0 to 1.0
  // ch5: 0.0 to 1.0
  // vertError: 1 if joystick was pushed past midpoint (invalid), 0 otherwise
  ADC1->ULLMEM.CTL0 |= 0x01;
  ADC1->ULLMEM.CTL1 |= 0x0100;
  uint32_t volatile delay = ADC1->ULLMEM.STATUS;
  while((ADC1->ULLMEM.STATUS&0x01)==0x01){};
  int raw4 = ADC1->ULLMEM.MEMRES[0];
  raw4 = (4010-raw4);
  *ch4 = raw4 / 4095.0f;

  int raw5 = ADC1->ULLMEM.MEMRES[1];
  *ch5 = raw5 / 4095.0f;

  int32_t raw6 = ADC1->ULLMEM.MEMRES[2];
  if(raw6 > 2048) {
    raw6 = 2048;
    *vertError = 1;
  } else {
    *vertError = 0;
  }
  *ch6 = (2048 - raw6) / 2048.0f;
}

// your function to convert ADC sample to distance (0.001cm)
// use main2 to calibrate the system fill in 5 points in Calibration.xls
//    determine constants k1 k2 to fit Position=(k1*Data + k2)>>12
// uint32_t Convert(uint32_t input){
  // return 42; // replace this with a linear function
// }

// do not use this function for the final lab
// it is added just to show you how SLOW floating point in on a Cortex M0+
// float FloatConvert(uint32_t input){
//   return 0.00048828125*input -0.0001812345;
// }

