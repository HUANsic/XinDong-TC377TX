/**********************************************************************************************************************
 * Author: WEIZIQIN
 * Description:
 * 
 * Version:
 *********************************************************************************************************************/

#ifndef ADC_H_
#define ADC_H_
/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include "XinDong_Config.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*-------------------------------------------------Data Structures---------------------------------------------------*/
/*********************************************************************************************************************/
 
/*********************************************************************************************************************/
/*--------------------------------------------Private Variables/Constants--------------------------------------------*/
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/

/**
 * @brief Main initialization function for the EVADC module.
 * @details This function coordinates the initialization of the EVADC module, group, channels,
 *          and starts the conversion queue. Call this function once before using ADC functions.
 */
void ADC_Init(void);

/**
 * @brief Adds channels to the conversion queue and starts continuous conversion.
 * @details Populates the EVADC queue with the 3 initialized channels (with auto-refill enabled)
 *          and triggers the start of continuous conversions. Called internally by _ADC_InitEVADC().
 */
void ADC_Start(void);

/**
 * @brief Reads and updates the latest conversion results for all channels.
 * @details Polls each channel until a valid conversion result (indicated by VF flag) is received,
 *          then stores the result in an internal static array. Call this function before
 *          retrieving results with _ADC_GetEVADCResult() or _ADC_GetEVADCVoltage().
 */
void ADC_Read(void);


/**
 * @brief Retrieves the voltage (in volts) for a specified ADC channel.
 * @param channelIndex Index of the target channel (0 = AN10, 1 = AN11, 2 = AN12).
 * @return Voltage value (float) in volts. Returns -1.0f if the channel index is invalid or
 *         the conversion result is invalid (VF = 0).
 * @note Call _ADC_ReadEVADC() first to ensure up-to-date results.
 */
float ADC_GetVoltage(uint8 channelIndex);

#endif /* ADC_H_ */
