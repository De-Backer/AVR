
#ifndef _adc_h__
#define _adc_h__

#ifdef __cplusplus
extern "C"
{
#endif

#include <avr/interrupt.h>
#include <avr/io.h>
    void init_ADC();
    ISR(ADC_vect);
#ifdef __cplusplus
} // extern "C"
#endif

#endif
