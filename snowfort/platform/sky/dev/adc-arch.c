/*
 * This file implements the architecture specific ADC
 * functionality.
 * ---------------------------------------------------------
 *
 * Author: Ronnie Bajwa
 */

#include "contiki.h"
#include "dev/adc-arch.h"

void adc_arch_on(void)
{
	ADC12CTL0 = ADC12ON + SHT0_12 ;//+ REFON + REF2_5V;
	//clock_delay(20000); //wait for reference to turn on (minimum 17 ms)

	ADC12CTL1 = SHP;
	//ADC12MCTL0 = INCH_11;//for sampling internal temperature sensor
}

/*
 * Selection which ADC channel to sample
 */
int adc_arch_configure(uint8_t chan_num)
{
	if(chan_num>11)
		return 0;

	ADC12MCTL0 = chan_num;
	return 1;
}

void adc_arch_off(void)
{
	//need to disable ENC first
	ADC12CTL0 &= ~ENC;
	while(ADC12CTL1 & ADC12BUSY);

	//return to default register values
	ADC12CTL0 = 0;
	ADC12CTL1=0;
	ADC12MCTL0=0;
}

unsigned short adc_arch_sample(void)
{
	ADC12CTL0 |=ENC + ADC12SC;
	while(ADC12CTL1 & ADC12BUSY);
	//printf("%d\n",ADC12MEM0);
	return ADC12MEM0;
}
