
#ifndef _adc_h__
#define _adc_h__

#ifdef __cplusplus
extern "C"
{
#endif

#include <avr/interrupt.h>
#include <avr/io.h>
    void init_ADC();
    uint16_t ADC_value(uint8_t source);

    ISR(ADC_vect);
#ifdef __cplusplus
} // extern "C"
#endif

#endif
