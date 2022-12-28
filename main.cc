#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "clk.h"
#include "task.h"
#include "timer.h"
#include "usb.h"
#include "render.h"
#include "debug.h"

// ## Digital inputs
// PWRIN PE6 - DC input jack NC detect switch
#define PINP_PWRIN PORTE
#define PINB_PWRIN PORTE6
// HWB# PE2 - Boot button
#define PINP_BOOTBTN PORTE
#define PINB_BOOTBTN PORTE2

// ## Digital outputs
// PREREGEN# PF7
#define PINP_PREREGENn PORTF
#define PINB_PREREGENn PORTF7
// TESTLOAD PF5
#define PINP_TESTLOAD PORTF
#define PINB_TESTLOAD PORTF5
// LED PB7 - Diagnostic LED - also OC0A PWM
#define PINP_DIAGLED PORTB
#define PINB_DIAGLED PORTB7

// ## Digital I/O SPI header interface
// SPISCLK PB1
#define PINP_SPISCLK PORTB
#define PINB_SPISCLK PORTB1
// SPIMOSI PB2
#define PINP_SPIMOSI PORTB
#define PINB_SPIMOSI PORTB2
// SPIMISO PB3
#define PINP_SPIMISO PORTB
#define PINB_SPIMISO PORTB3

// ## PWM outputs
// VSETPWM PC7/OC4A 9-bit 64MHz 125kHz PWM
#define PINP_VSETPWM PORTC
#define PINB_VSETPWM PORTC7
// ISETPWM PC6/OC3A 9-bit 16MHz 31.25kHz PWM
#define PINP_ISETPWM PORTC
#define PINB_ISETPWM PORTC6
// IOFSPWM PB6/OC4B 9-bit 64MHz 125kHz PWM
#define PINP_IOFSPWM PORTB
#define PINB_IOFSPWM PORTB6
// POSTREGPWM PD7/OCR4D 9-bit 64MHz 125kHz PWM
#define PINP_POSTREGPWM PORTD
#define PINB_PORTREGPWM PORTD7

// ## ADC inputs
// VINSENSE PF4/ADC4
#define PINA_VINSENSE ADC10D
// AMPVAL PF1/ADC1
// VPREREGSENSEHI PF0/ADC0
// VPOSTREGOUT PD7/ADC10

//
// Timer

enum input_index : uint8_t {
	input_vpreregsensehi,
	input_ampval,
	input_vinsense,
	input_vpostregout
};

//
// Debug LEDs

// 
// PWM

static void configure_usbpower_pwm()
{
#ifdef __AVR_ATmega32U4__

	//
	// Initialize Timer0 (LED PWM)

	// Fast-PWM, inverted output to sink current during duty cycle
	// TCCR0A = (1U << COM0A0) | (1U << COM0A1) | 
	// 	(1U << WGM00) | (1U << WGM01);
	// TCCR0B = (1U << WGM02);
	
	//
	// Initialize Timer4

	// Fast PWM, Clear on compare match, set on 0 on A and B and D
	TCCR4A = (1U << COM4A1) | (1U << COM4B1) | (1U << PWM4A) | (1U << PWM4B);
	TCCR4C = (1U << COM4D1) | (1U << PWM4D);

	TCCR4B = (1U << PWM4X);

	// 64MHz timer (96MHz / 1.5 = 64MHz)
	PLLFRQ |= (1U << PLLTM1);
#endif
}

//
// ADC

// For analog input pins, the digital input buffer should be disabled 
// at all times. An analog signal level close to VCC/2 on an input pin
// can cause significant current even in active mode. Digital input
// buffers can be disabled by writing to the Digital Input Disable
// Registers (DIDR1 and DIDR0)

static const uint8_t channel_to_mux[] PROGMEM = {
	0b000000,	// [ 0] ADC0 (PREREGSENSEHI)
	0b000001,	// [ 1] ADC1 (AMPVAL)
	0xFF,			// [ 2] unavailable
	0xFF,			// [ 3] unavailable
	0b000100,	// [ 4] ADC4 (VINSENSE)
	0b000101,	// [ 5] ADC5
	0b000110,	// [ 6] ADC6
	0b000111,	// [ 7] ADC7
	0b100000,	// [ 8] ADC8
	0b100001,	// [ 9] ADC9
	0b100010,	// [10] ADC10 (VPOSTREGOUT)
	0b100011,	// [11] ADC11
	0b100100,	// [12] ADC12
	0b100101,	// [13] ADC13
	0b100111,	// [14] Temperature sensor
	0b011110,	// [15] 1.1V
	0b011111	// [16] GND
};

static constexpr uint8_t const channel_temp = 14;
static constexpr uint8_t const channel_1v1 = 15;
static constexpr uint8_t const channel_GND = 16;
static constexpr uint8_t const channel_table_length = 
	sizeof(channel_to_mux) / sizeof(*channel_to_mux);

static constexpr uint8_t const channel_vin_sense = 4;
static constexpr uint8_t const channel_ampval = 1;
static constexpr uint8_t const channel_vpreregsensehi = 0;
static constexpr uint8_t const channel_vpostregout = 10;

// Must be called with interrupts disabled
static void select_adc_channel(uint8_t channel)
{
	insist(!(SREG & (1U << SREG_I)));
	insist(channel < channel_table_length);

	uint8_t muxval = pgm_read_byte(&channel_to_mux[channel]);

	// REFS0=0,REFS1=1: AVCC with external capacitor on AREF pin
	ADMUX = (1U<<REFS1) | channel;
	// Keep high speed mode and set 5th mux bit in other register
#ifdef __AVR_ATmega32U4__
	ADCSRB = 
		(1U<<ADHSM) | 
		((muxval & 0b10000) ? MUX5 : 0);
#endif
}

static uint8_t const adc_channels[4] PROGMEM = { 0, 1, 4, 10 };
static uint16_t adc_values[4];
static uint8_t adc_input;

static void adc_start_conversion()
{
	debug_leds_toggle_led_divisor(1, 1, 32768);

	// Enable ADC, start conversion, enable ADC interrupt
	ADCSRA = (1U<<ADEN) | (1U<<ADSC) | (1U<<ADIE);
}

static void configure_usbpower_adc()
{	
#ifdef __AVR_ATmega32U4__
	// Disable digital input buffer on ADC inputs
	DIDR0 = (1U<<ADC0D) | (1U<<ADC1D) | (1U<<ADC4D);
	DIDR2 = (1U<<ADC10D);
#endif

	// Configure ADC
	select_adc_channel(0);
	adc_start_conversion();
}

static uint8_t select_next_adc_input(uint8_t from)
{
	uint8_t channel_count = sizeof(adc_channels) / sizeof(*adc_channels);
	return from + 1 < channel_count ? from + 1 : 0;
}

// ADC conversion complete IRQ
ISR(ADC_vect)
{
	uint16_t reading = ADC;
	uint8_t input = adc_input;
	adc_values[input] = reading;	
	input = select_next_adc_input(input);
	adc_input = input;
	
	uint8_t next_channel = pgm_read_byte(adc_channels + input);
	select_adc_channel(next_channel);
	
	adc_start_conversion();
}

// 1000 = 1V
static uint16_t set_mV;

// 1000 = 1A
static uint16_t set_mA;

// Calibration
static uint16_t cal_null_iofs;
static char calibrate_stk[128];

static void set_vsetpwm(uint16_t level)
{
#ifdef __AVR_ATmega32U4__
	TC4H = (level >> 8) & 0b111;
	OCR4A = level & 0xFF;
#endif
}

static void set_testload(bool on)
{
#ifdef __AVR_ATmega32U4__
	PINP_TESTLOAD = (PINP_TESTLOAD & 
		~(1U << PINB_TESTLOAD)) | 
		((uint8_t)on << PINB_TESTLOAD);
#endif
}

static void set_isetpwm(uint8_t level)
{
#ifdef __AVR_ATmega32U4__
	OCR3A = level;
#endif
}

static void set_iofspwm(uint16_t level)
{
#ifdef __AVR_ATmega32U4__
	TC4H = (level >> 8) & 0b111;
	OCR4B = level & 0xFF;
#endif
}

static void set_postregpwm(uint16_t level)
{
#ifdef __AVR_ATmega32U4__
	TC4H = (level >> 8) & 0b111;
	OCR4D = level & 0xFF;
#endif
}

static constexpr uint8_t curve_size = 8;
static uint8_t curve_vset[curve_size];

// Task function
static void *calibrate(void *)
{
#if 1
uint8_t val = 0;
	while (true) {
		debug_leds_toggle_led(0);
		timer_wait_for_ms(250);

		if ((val += 25) == 250) {
			val = 0;
		}
		set_postregpwm(val);
		
		debug_leds_toggle_led(0);
		timer_wait_for_ms(750);
	}
#else
	// First, set voltage and current PWM to zero
	set_iofspwm(0);
	set_vsetpwm(0);
	set_postregpwm(0);
	
	// Wait 20ms to settle
	timer_wait_for_ms(20);

	// Turn on test load
	set_testload(true);

	// Wait 20ms to settle
	timer_wait_for_ms(20);

	// Read baseline voltage
	uint16_t baseline = adc_values[input_vpostregout];

	// 20ms per iteration, 9 iterations, 180ms
	uint16_t srch_min = 0;
	uint16_t srch_max = 511;
	do {
		// Try midpoint of range
		uint16_t srch_mid = (srch_max - srch_min) >> 1;

		// Set current offset
		set_iofspwm(srch_mid);

		// Wait for it to completely settle
		timer_wait_for_ms(20);

		// Check output voltage
		uint16_t sample = adc_values[input_vpostregout];

		// If the setting is too high, the output voltage will droop
		if (sample < baseline && baseline - sample > 20)
			srch_max = srch_mid;
		else
			srch_min = srch_mid + 1;
	} while (srch_min < srch_max);

	set_testload(false);
	cal_null_iofs = srch_min;

	//
	// Find preregulator response curve

	// Set unlimited current
	set_isetpwm(0xFF);

	for (uint8_t i = 0; i < curve_size; ++i) {
		set_vsetpwm(i * (512 / curve_size));

		timer_wait_for_ms(20);

		curve_vset[i] = adc_values[input_vpreregsensehi];
	}

	return nullptr;
#endif
}

int main()
{
	// Max throttle, immediately
	clk_divisor(0);
	debug_leds_init();
	usb_init();
	task_init();
	timer_init();
	configure_usbpower_adc();
	render_init();

	task_t calibration_task = task_create(calibrate_stk, 
		sizeof(calibrate_stk), calibrate);

	task_run_forever();
}
