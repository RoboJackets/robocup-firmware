#include "analog_in.h"

// ADC Reading constants
const int REFRESH_PERIOD = 10;
const int ADC_MAX = 4096;
const float ADC_RATIO = 3.3 / 4096;

void analogin_init(adc_pin_name pin, adc_handle* handle) {
    __HAL_RCC_ADC3_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {};
    GPIO_InitStruct.Pin = pin.pin.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    HAL_GPIO_Init(pin.pin.port, &GPIO_InitStruct);

    // TODO Possibly give init back to the user by having these just be a default by checking if field is zeroed
    handle->ADC_InitStruct.Instance                   = pin.adc;
    handle->ADC_InitStruct.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV2;
    handle->ADC_InitStruct.Init.Resolution            = ADC_RESOLUTION_12B;
    handle->ADC_InitStruct.Init.ScanConvMode          = DISABLE; /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
    handle->ADC_InitStruct.Init.ContinuousConvMode    = ENABLE; /* Continuous mode enabled to have continuous conversion */
    handle->ADC_InitStruct.Init.DiscontinuousConvMode = DISABLE; /* Parameter discarded because sequencer is disabled */
    handle->ADC_InitStruct.Init.NbrOfDiscConversion   = 0;
    handle->ADC_InitStruct.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE; /* Conversion start not triggered by external event */
    handle->ADC_InitStruct.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;
    handle->ADC_InitStruct.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    handle->ADC_InitStruct.Init.NbrOfConversion       = 1;
    handle->ADC_InitStruct.Init.DMAContinuousRequests = DISABLE;
    handle->ADC_InitStruct.Init.EOCSelection          = DISABLE;
    //initializes the ADC given the structure that defines it
    if (HAL_ADC_Init(&handle->ADC_InitStruct) != HAL_OK) {
        // TODO ERROR HANDLING
    }

    handle->ADC_ChannelConfig.Channel       = pin.channel;
    handle->ADC_ChannelConfig.Rank          = 1;
    handle->ADC_ChannelConfig.SamplingTime  = ADC_SAMPLETIME_480CYCLES;
    handle->ADC_ChannelConfig.Offset        = 0;
    //configures the channel for the ADC, given the input structure
    if (HAL_ADC_ConfigChannel(&handle->ADC_InitStruct, &handle->ADC_ChannelConfig) != HAL_OK) {
    // TODO ERROR HANDLING
    }

    HAL_ADC_Start(&handle->ADC_InitStruct);
}

//reads in a float
uint32_t analogin_read(adc_handle* handle) {
    return HAL_ADC_GetValue(&handle->ADC_InitStruct);
}

//read in unsigned 16 bit Value
uint16_t analogin_read_u16(adc_handle* handle) {
    return (uint16_t) HAL_ADC_GetValue(&handle->ADC_InitStruct);
}

float analogin_voltage(adc_handle* handle) {
    return (float)(HAL_ADC_GetValue(&handle->ADC_InitStruct)) * ADC_RATIO;
}

float analogin_percent(adc_handle* handle) {
    return (float)(HAL_ADC_GetValue(&handle->ADC_InitStruct)) / ADC_MAX;
}
