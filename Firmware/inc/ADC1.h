/* ADC1.h
 * Students put your names here
 * Modified: put the date here
 * 12-bit ADC input on ADC1 channel 5, PB18
 */

/**
 * @file      ADC1.h
 * @brief     12-bit ADC input
 * @details   12-bit ADC input on ADC1 channel 5, PB18<br>
 * @version   ECE319K Spring 2025
 * @author    Your names go here
 * @copyright Lab solution, do not post,
 * @warning   AS-IS
 * @note      For more information see  http://users.ece.utexas.edu/~valvano/
 * @date      July 19, 2025
  <table>
<caption id="Lab7pins">Pins for the 12-bit ADC</caption>
<tr><th>Pin <th>GPIO<th>Hardware
<tr><td>PB18 <td>input<td>connected to slide-pot sensor
</table>
  ******************************************************************************/
#ifndef __ADC1_H__
#define __ADC1_H__
#include <stdint.h>

/**
 * Initialize 12-bit ADC
 *    - PB18 analog input
 * @param none
 * @return none
 * @brief  Initialize ADC
 * @note LaunchPad_Init has been called; this program should not reset Port B
 */
void ADCinit(void);
// write code to initialize ADC1 channel 5, PB18
// Your measurement will be connected to PB18
// 12-bit mode, 0 to 3.3V, right justified
// software trigger, no averaging

/**
 * Sample ADC1 channels 4, 5, 6 in sequence
 * @param ch4 pointer to store channel 4 result (PB17, 0-4095)
 * @param ch5 pointer to store channel 5 result (PB18, 0-4095)
 * @param ch6 pointer to store channel 6 result (PB19, 0-4095)
 * @brief  Reads all three ADC1 channels in one software-triggered sequence
 */
void ADCin(volatile float *ch4, volatile float *ch5, volatile float *ch6, uint8_t *vertError);


/**
 * Convert raw 12-bit ADC to fixed point distance
 * @param input 12-bit ADC raw sample
 * @return distance as 0.001cm fixed point number
 * @brief  Input from PB18
 */
// uint32_t Convert(uint32_t input);
//    determine constants k1 k2 to fit Position=(k1*Data + k2)>>12


/**
 * Convert raw 12-bit ADC to distance
 * @param input 12-bit ADC raw sample
 * @return distance as a floating-point number in cm
 * @brief  Input from PB18
 */
// float FloatConvert(uint32_t input);
// do not use this function
// it is added just to show you how SLOW floating point in on a Cortex M0+

#endif // __ADC1_H__
