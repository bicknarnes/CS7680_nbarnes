#include <stdio.h>
#include <stdlib.h>
#include <pigpio.h>
#include <unistd.h>
#include <signal.h>

/*
gcc -pthread -o temp temp.c -lpigpio -lrt
temp [num_of_loops] [speed_of_transmission]
defaults to 1000000 in both cases
*/

// Define LED pins:
const int blueLED = 12;
const int redLED = 21;

// Set up signal handling:
volatile sig_atomic_t signal_received = 0;
void sigint_handler(int signal) {
	signal_received=signal;
}

// Main program:
int main(int argc, char *argv[]) {
	if(gpioInitialise() == PI_INIT_FAILED) {
		printf("ERROR: Failed to initialize GPIO interface.\n");
		return 1;
	}

	// Set LED pins to output:
	gpioSetMode(blueLED, PI_OUTPUT);
	gpioSetMode(redLED, PI_OUTPUT);
	
	// Initialize variables to read from 3008 chip:
	int i;
	int h;
	int v=0;
	int loops;
	int speed;
	double start, diff, sps;
	unsigned char buf[3];
	if (argc > 1) loops = atoi(argv[1]);
	else loops = 1000000;
	if (argc > 2) speed = atoi(argv[2]);
	else speed = 1000000;
	if (gpioInitialise() < 0) return 1;
	h = spiOpen(0, speed, 0);
	if (h < 0) return 2;
	start = time_time();
	for (i=0; i<loops; i++) {
		buf[0] = 1;
		buf[1] = 128;
		buf[2] = 0;
		spiXfer(h, buf, buf, 3);
		v = ((buf[1]&3)<<8) | buf[2];
		// light LEDs based on low/high temperatures
		if(v < 350) {
			gpioWrite(redLED, PI_LOW);
			gpioWrite(blueLED, PI_HIGH);
		} else {
			gpioWrite(redLED, PI_HIGH);
			gpioWrite(blueLED, PI_LOW);
		}
		sleep(1);

	}
	diff = time_time() - start;
	fprintf(stderr, "sps=%.1f @ %d bps (%d/%.1f)\n", (double)loops / diff, speed, loops, diff);
	spiClose(h);
	// Ensure LEDs are off:
	gpioWrite(redLED, PI_LOW);
	gpioWrite(blueLED, PI_LOW);
	// Set LED pins to input before terminating GPIO
	gpioSetMode(blueLED, PI_INPUT);
	gpioSetMode(redLED, PI_INPUT);
	// terminate GPIO
	gpioTerminate();
	return 0;
}
