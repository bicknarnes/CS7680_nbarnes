#include <signal.h>
#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

const int GreenLED = 22;
const int YellowLED = 26;
const int RedLED = 17;
const int PWM_pin = 21;

#define OFF 0
#define LOW 85
#define MEDIUM 171
#define HIGH 255

volatile sig_atomic_t signal_received = 0;

void sigint_handler(int signal){
	signal_received=signal;
}

// function to handle setting LED according to motor speed
void set_LED(int speed) {
	switch(speed) {
		case OFF:
			// turn LEDs off
			gpioWrite(GreenLED, PI_LOW);
			gpioWrite(YellowLED, PI_LOW);
			gpioWrite(RedLED, PI_LOW);
			break;
		case LOW:
			// turn on yellow LED, others off
			gpioWrite(GreenLED, PI_LOW);
			gpioWrite(RedLED, PI_LOW);
			gpioWrite(YellowLED, PI_HIGH);
			break;
		case MEDIUM:
			// turn on green LED, others off
			gpioWrite(YellowLED, PI_LOW);
			gpioWrite(RedLED, PI_LOW);
			gpioWrite(GreenLED, PI_HIGH);
			break;
		case HIGH:
			// turn on red LED, others off
			gpioWrite(GreenLED, PI_LOW);
			gpioWrite(YellowLED, PI_LOW);
			gpioWrite(RedLED, PI_HIGH);
			break;
	}
}

int set_intensity(char input) {
	switch(input) {
		case 'o':
			//intensity = OFF;
			printf("Motor off.\n");
			return OFF;
			break;
		case 'l':
			//intensity = LOW;
			printf("Motor on low speed.\n");
			return LOW;
			break;
		case 'm':
			//intensity = MEDIUM;
			printf("Motor on medium speed.\n");
			return MEDIUM;
			break;
		case 'h':
			//intensity = HIGH;
			printf("Motor on high speed.\n");
			return HIGH;
			break;
		default:
			printf("Invalid input. Please try again.\n");
			break;
	}

}

int main (void) {
    if(gpioInitialise()==PI_INIT_FAILED){
	printf("ERROR: Failed to initialize the GPIO interface.");
	return 1;
    }
    signal(SIGINT, sigint_handler);
    // initialize variable to control intensity for PWM
    int intensity;
    // set PWM pin as output
    gpioSetMode(PWM_pin, PI_OUTPUT);
    gpioSetPWMrange(PWM_pin, 255);   
    gpioSetMode(RedLED, PI_OUTPUT);
    gpioSetMode(GreenLED, PI_OUTPUT);
    gpioSetMode(YellowLED, PI_OUTPUT);

    printf("Enter CTRL-C to exit.\n");
    printf("\n");
    printf("Welcome to the motor control program :)\n\nEnter 'o' to turn the motor off, 'l' for low speed setting, 'm' for medium speed setting, or 'h' for high speed setting.\nPress CTRL-C to exit the program.\n\n");

    while (!signal_received) {
	char user_input = getchar();
	if(signal_received) {break;}
	getchar();

	// Set PWM intensity based on user input
	intensity = set_intensity(user_input);
	gpioPWM(PWM_pin, intensity);
	set_LED(intensity);
	gpioDelay(10000);
	
    }

    // turn off LEDs & motor
    gpioPWM(PWM_pin, 0);
    set_LED(OFF);

    // set in-use GPIO pins to input for safe exit of program
    gpioSetMode(PWM_pin, PI_INPUT);
    gpioSetMode(RedLED, PI_INPUT);
    gpioSetMode(YellowLED, PI_INPUT);
    gpioSetMode(GreenLED, PI_INPUT);
    gpioTerminate();
    return 0;
}

