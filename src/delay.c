//Crude millisecond delay function.

void mdelay (unsigned int ms) {                     
  unsigned int i, j;
  unsigned int max = 0xFFF;
  
  for (i = 0; i < ms; i++) {
		//Delay approx 1ms
		for (j = 0; j < max; j++);
	}
}
