/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include "ADC.h"
#include "IfxEvadc_Adc.h"

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/

static IfxEvadc_Adc         g_evadc;                                   /* EVADC module handle variable                     */
static IfxEvadc_Adc_Group   g_adcGroup;                                /* EVADC group handle variable                      */
static IfxEvadc_Adc_Channel g_adcChannel[3];                           /* EVADC channel handle variable                    */
static Ifx_EVADC_G_RES      g_adc_result[3];                               /* Variable to store measurement results            */
/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
/*********************************************************************************************************************/
/* The initialization of the port pins to control the LEDs is done by this function */

/* Function to initialize the EVADC module with default parameters */
void _ADC_InitEVADCModule(void)
{
    /* Create configuration */
    IfxEvadc_Adc_Config adcConfig;
    IfxEvadc_Adc_initModuleConfig(&adcConfig, &MODULE_EVADC);

    /* Initialize module */
    IfxEvadc_Adc_initModule(&g_evadc, &adcConfig);
}

/* Function to initialize the EVADC group with default parameters */
void _ADC_InitEVADCGroup(void)
{
    /* Create and initialize group configuration with default values */
    IfxEvadc_Adc_GroupConfig adcGroupConfig;
    IfxEvadc_Adc_initGroupConfig(&adcGroupConfig, &g_evadc);

    /* Setting user configuration using group 0 */
    adcGroupConfig.groupId = POT1_VADC_PIN.groupId;

    adcGroupConfig.master = POT1_VADC_PIN.groupId;

    /* Enable queued source */
    adcGroupConfig.arbiter.requestSlotQueue0Enabled = TRUE;

    /* Enable all gates in "always" mode (no edge detection) */
    adcGroupConfig.queueRequest[0].triggerConfig.gatingMode = IfxEvadc_GatingMode_always;

    /* Initialize the group */
    IfxEvadc_Adc_initGroup(&g_adcGroup, &adcGroupConfig);
}

/* Function to initialize the EVADC channel */
void _ADC_InitEVADCChannels(void)
{
    // Channel AN10
    /* Create channel configuration */
    IfxEvadc_Adc_ChannelConfig adcChannelConfig;

    /* Initialize the configuration with default values */
    IfxEvadc_Adc_initChannelConfig(&adcChannelConfig, &g_adcGroup);

    /* Select the channel ID and the respective result register */
    adcChannelConfig.channelId = POT1_VADC_PIN.channelId;
    adcChannelConfig.resultRegister = (IfxEvadc_ChannelResult)(POT1_VADC_PIN.channelId);

    /* Initialize the channel */
    IfxEvadc_Adc_initChannel(&g_adcChannel[0], &adcChannelConfig);

    // Channel AN11
    IfxEvadc_Adc_initChannelConfig(&adcChannelConfig, &g_adcGroup);
    adcChannelConfig.channelId = POT2_VADC_PIN.channelId;
    adcChannelConfig.resultRegister = (IfxEvadc_ChannelResult)(POT2_VADC_PIN.channelId);
    IfxEvadc_Adc_initChannel(&g_adcChannel[1], &adcChannelConfig);

    // Channel AN12
    IfxEvadc_Adc_initChannelConfig(&adcChannelConfig, &g_adcGroup);
    adcChannelConfig.channelId = POT3_VADC_PIN.channelId;
    adcChannelConfig.resultRegister = (IfxEvadc_ChannelResult)(POT3_VADC_PIN.channelId);
    IfxEvadc_Adc_initChannel(&g_adcChannel[2], &adcChannelConfig);
}

void ADC_Init(void)
{
    _ADC_InitEVADCModule();      /* Initialize the EVADC module  */
    _ADC_InitEVADCGroup();       /* Initialize the EVADC group   */
    _ADC_InitEVADCChannels();    /* Initialize the channel       */
    ADC_Start();    /* Fill the queue and start it  */
}

/* Function to add the channel to a queue */
void ADC_Start(void)
{
    int i;
    for (i=0 ; i<3 ; i++)
    {
        /* Add channel to queue with refill option enabled */
        IfxEvadc_Adc_addToQueue(&g_adcChannel[i],IfxEvadc_RequestSource_queue0, IFXEVADC_QUEUE_REFILL);
    }

    /* Start the queue */
    IfxEvadc_Adc_startQueue(&g_adcGroup, IfxEvadc_RequestSource_queue0);
}

/* Function to read the EVADC used channel */
void ADC_Read(void)
{
    int i;
    for (i = 0; i < 3; ++i)
    {
        // read directly
        Ifx_EVADC_G_RES conversionResult = IfxEvadc_Adc_getResult(&g_adcChannel[i]);

        // save latest results
        g_adc_result[i] = conversionResult;
    }
}
/**
 * @brief Retrieves the converted voltage value for a specified ADC channel.
 * @details This function performs validation checks on the channel index and conversion result,
 *          then converts the raw ADC value to a voltage (in volts) using a 3.3V reference voltage
 *          and 12-bit resolution (0-4095).
 * @param channelIndex Index of the ADC channel to read (valid range: 0 = AN10, 1 = AN11, 2 = AN12)
 * @return Voltage value in volts (float):
 *         - Valid range: 0.0V to 3.3V (corresponding to raw ADC values 0 to 4095)
 *         - Returns -1.0f if:
 *             - channelIndex is out of range (≥ 3)
 *             - The conversion result is invalid (Valid Flag [VF] not set)
 * @note Must call _ADC_ReadEVADC() prior to this function to ensure the latest conversion results are available.
 */
float ADC_GetVoltage(uint8 channelIndex)
{
    // Define ADC reference parameters: 3.3V reference voltage and 12-bit resolution (0-4095)
    const float referenceVoltage = 3.3f;  // ADC reference voltage (3.3V)
    const uint32 adcResolution = 4095;  // Maximum raw value for 12-bit ADC (2^12 - 1)

    // Validate channel index (only 0, 1, or 2 are allowed)
    if (channelIndex >= 3)
    {
        return -1.0f;  // Return error value for out-of-range channel index
    }

    // Retrieve the raw conversion result from the internal buffer
    Ifx_EVADC_G_RES rawResult = g_adc_result[channelIndex];

    // Check if the raw result is valid (Valid Flag [VF] must be set to 1)
    if (rawResult.B.VF != 1)
    {
        return -1.0f;  // Return error value for invalid conversion result
    }

    // Convert raw ADC value to voltage using the formula: V = (raw * V_ref) / max_resolution
    return (rawResult.B.RESULT * referenceVoltage) / adcResolution;
}
