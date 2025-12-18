#include <stdio.h>

int main(void)
{
	uint16_t n=16;	// Target resolution
	uint16_t b=11;	// Actual hardware resolution
	uint16_t val, pwm, pwm_out, error;
	uint16_t mask = (1 << (n-b)) - 1;
	
	error = 0
	pwm = 11000;	// Desired PWM to full n bit resolution
	error = 0;		// Assume no error
	while (1) {
		val = pwm + error;	// Add error from previous
		pwm_out = (val >> (n-b));	// Scale output to actual b bits of resolution
		print("Outputting: $%04X, error=%d", pwm_out, error)
		error = val & mask;
	}
	
	 
	 
	assert (n >= b)
	
}
