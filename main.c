#include "lpc24xx.h"

void setupADC(void);
int readADC(void);

int main(void) {
	int result;
	
	setupADC();

	// Hello
	//Loop forever, reading ADC channel AD0.2
	while (1) {
		result = readADC();
	}		
}

// Initialize any required registers for the ADC functionality here
void setupADC(void) {
	//Implement this function
}

// Function to sample and return ADC channel AD0.2
// NOTE: You must setup the entire ADC Control Register before EVERY sample.
int readADC(void) {
	//Implement this function
}
