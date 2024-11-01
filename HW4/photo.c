#include <pigpio.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h> // For usleep function

// Define GPIO pins:
#define IN_RC 18       // Input pin (BCM numbering)
#define OUT_LOW 20     // Low-light output pin
#define OUT_HIGH 21    // High-light output pin
#define OUT_STATE 16   // Program state output pin

// Define thresholds for low/moderate/high light conditions:
#define HIGH_LIMIT 80000
#define MID_LIMIT 1250000
#define LOW_LIMIT 17500000

// Function to measure photocell/capacitor timing:
int RCtime(int RCpin) {
    int reading = 0;

    gpioSetMode(RCpin, PI_OUTPUT);
    gpioWrite(RCpin, 0);
    // Delay 100 milliseconds:
    gpioDelay(100000);

    gpioSetMode(RCpin, PI_INPUT);
    // This loop takes about 1 microsecond per cycle
    while (gpioRead(RCpin) == 0) {
        reading++;
	if(reading > LOW_LIMIT) {
		break;
	}
    }
    return reading;
}

// Function for LED output:
void ledOut(int state) {
    switch (state) {
        case 0:
            gpioWrite(OUT_LOW, PI_HIGH);
            gpioWrite(OUT_HIGH, PI_HIGH);
            break;
        case 1:
            gpioWrite(OUT_LOW, PI_LOW);
            gpioWrite(OUT_HIGH, PI_HIGH);
            break;
        case 2:
            gpioWrite(OUT_LOW, PI_HIGH);
            gpioWrite(OUT_HIGH, PI_LOW);
            break;
        case 3:
            gpioWrite(OUT_LOW, PI_LOW);
            gpioWrite(OUT_HIGH, PI_LOW);
            break;
        default:
            break;
    }
}

// Function to process reading and output information to user:
char photocellParse(int reading) {
    char out;
    if (reading <= HIGH_LIMIT) {
        out = '0';
        ledOut(0);
    } else if (reading <= MID_LIMIT) {
        out = 'o';
        ledOut(1);
    } else if (reading <= LOW_LIMIT) {
        out = '.';
        ledOut(2);
    } else {
        out = ' ';
        ledOut(3);
    }
    return out;
}

// Signal handling:
volatile sig_atomic_t signal_received = 0;
void sigint_handler(int signal) {
	signal_received = signal;
}

int main(void) {
    // Initialize gpio:
    if (gpioInitialise() == PI_INIT_FAILED) {
        printf("ERROR: Failed to initialize the GPIO interface.\n");
        return 1;
    }

    // Set modes for output pins:
    gpioSetMode(OUT_LOW, PI_OUTPUT);
    gpioSetMode(OUT_HIGH, PI_OUTPUT);
    gpioSetMode(OUT_STATE, PI_OUTPUT);

    signal(SIGINT, sigint_handler);
    printf("Press CTRL-C to exit.\n");

    // Main loop
    while (!signal_received) {
	// Run out_state light:
        gpioWrite(OUT_STATE, PI_HIGH);
	// Get reading:
        int reading = RCtime(IN_RC);
        printf("%c", photocellParse(reading));
        fflush(stdout);
        // Delay 100 milliseconds:
        usleep(100000);
    }
    // Put pins safely back into input state:
    gpioSetMode(OUT_LOW, PI_INPUT);
    gpioSetMode(OUT_HIGH, PI_INPUT);
    gpioSetMode(OUT_STATE, PI_INPUT);
    // Terminate GPIO interface:
    gpioTerminate();
    printf("\n");
    return 0;
}




