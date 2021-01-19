#include "../include/adc.h"

void init_ADC()
{
    /* DIDR0 – Digital Input Disable Register 0 */
    DIDR0 = 0xff;

    /* ADC Multiplexer Selection Register */
    /* AREF, Internal Vref turned 2.56V */
    ADMUX = (0x01 << REFS1) | (0x01 << REFS0);

    /* MUX4:0: Analog Channel and Gain Selection Bits => ADC0 */
    /* ADLAR ADC Left Adjust Result */
    ADMUX |= (0x01 << ADLAR);

    /* ADC Control and Status Register A */
    /* ADPS2:0: ADC Prescaler Select Bits 156.25KHz sample rate @ 20MHz */
    ADCSRA = (0x01 << ADPS2) | (0x01 << ADPS1) | (0x01 << ADPS0);
    /* ADEN: ADC Enable */
    ADCSRA |= (0x01 << ADEN);
    /* ADIE: Interrupt Enable */
    ADCSRA |= (0x01 << ADIE);
}

/* interrupt */
ISR(ADC_vect)
{
    /* Sleep clear */
    SMCR &= ~(0x01 << SE);
}
