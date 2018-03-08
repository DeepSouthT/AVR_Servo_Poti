/*******************************
 * Servo_Poti_20180214.c
 *
 * Created: 14.02.2018 19:00:19
 * Author : DeepSouthT
 *
 * Used:
 *    IC > ATtiny13
 *    IDE > Atmel Studio 7
 *    Programmer > Atmel STK500
 *    Micro servo TowerPro SG92R (separate 4.5V)
 *    Potentiometer
 * 
 * Description:
 *     Find the complete rotation angle of the servo.
 *     36Hz PWM wave is used for the servo.
 *     Potentiometer is used to change the width of the PWM wave.
 *******************************/ 

#include <avr/io.h>
#include <stdbool.h>
#include <stdint-gcc.h>

#define F_CPU 9600000
#define SERVO PB1

uint8_t adc_filter_output =0;

void setup_adc (void)
{
	// Set the ADC input to PB2/ADC1
	// See ATtiny13 data sheet, Table 13-4
	ADMUX |= (1 << MUX0);
	
	// Only 8-bit ADC precision needed (10-bit available)
	// ADC result left adjusted
	// For 8-bit ADC precision, only need to read the ADCH
	ADMUX |= (1 << ADLAR);
	
	// Set the ADC prescaler to 8 and enable the ADC
	// See ATtiny13 data sheet, Table 14-4
	ADCSRA |= (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN);
}

uint8_t read_adc (void)
{
	// Start the conversion
	ADCSRA |= (1 << ADSC);
	
	// Wait for it to finish
	while (ADCSRA & (1 << ADSC));
	
	// ADC result with 8-bit precision
	return ADCH;
}

uint8_t filter_adc_reading(uint8_t input)
{	
	// Note that the first value is zero !!
	adc_filter_output =(((adc_filter_output*3)+input)/4);
	
	return adc_filter_output;
}

void setup_pwm (void)
{
	// System clock 9.6 MHz
	// Set Timer0 prescaler to 1024
	// At 9.6 MHz this is 9.6 MHz/1024 = 9375 Hz (or 0.11 ms)
	// This is an 8-bit counter, so it counts from 0 to 255
	// PWM frequency = 256*0.11ms = ~28,16 ms or ~36 Hz
	
	// PWM frequency = (Fclk_io/(N(1+TOP)))
	// N: Prescaler, TOP: Counter top
	
	// Prescaler 1024
	// See ATtiny13 data sheet, Table 11-9
	TCCR0B |= (1<<CS00)|(1<<CS02);
	
	// Set to 'Fast PWM' mode
	// Clear OC0B output on compare match, upwards counting.
	TCCR0A |= (1<<WGM01)|(1<<WGM00)|(1<<COM0A1)|(1<<COM0B1)|(1<<COM0B0);
}

void pwm_width (uint8_t val)
{
	// See ATtiny13 data sheet, Figure 11-6
	OCR0B = val;
}

int main (void)
{
	// Parameter holding the ADC reading
	uint8_t adc_reading;
	
	// SERVO pin is an output
	DDRB |= (1 << SERVO);
	
	setup_adc();
	setup_pwm();
	
	while (1) {
		// Get the ADC reading
		adc_reading = read_adc();
		//Filter the reading
		adc_reading =filter_adc_reading(adc_reading);
		// Now write it to the PWM counter
		pwm_width(adc_reading);
	}
}

