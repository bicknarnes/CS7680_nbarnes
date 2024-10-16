#include <signal.h>
#include <stdio.h>
#include <pigpio.h>

const int RedLED = 21;
const int GreenLED = 16;
const int YellowLED = 20;

volatile sig_atomic_t signal_received = 0;

void sigint_handler(int signal) {
	signal_received = signal;
}

void blinkXseconds (int pin, int numSeconds) {
		gpioWrite(pin, PI_HIGH);
		time_sleep(numSeconds);
		gpioWrite(pin, PI_LOW);
		time_sleep(.5);
}

int main() {
	if (gpioInitialise() == PI_INIT_FAILED) {
		printf("ERROR: Failed to initialize the GPIO interface.\n");
		return 1;
	}	
	gpioSetMode(RedLED, PI_OUTPUT);
	gpioSetMode(YellowLED, PI_OUTPUT);
	gpioSetMode(GreenLED, PI_OUTPUT);
	signal(SIGINT, sigint_handler);
	printf("Press CTRL-C to exit.\n");
	while (!signal_received) {
		blinkXseconds(RedLED, 20);
		blinkXseconds(GreenLED, 18);
		blinkXseconds(YellowLED, 2);
	}
	gpioSetMode(RedLED, PI_INPUT);
	gpioSetMode(YellowLED, PI_INPUT);
	gpioSetMode(GreenLED, PI_INPUT);
	gpioTerminate();
	printf("\n");
	return 0;	
}
