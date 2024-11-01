#include <pigpio.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h> // For usleep function

// Settings
#define IN_RC 18       // Input pin (BCM numbering)
#define OUT_LOW 20     // Low-light output pin
#define OUT_HIGH 21    // High-light output pin
#define OUT_STATE 16   // Program state output pin

// define thresholds for low/moderate/high light conditions
#define HIGH_LIMIT 80000
#define MID_LIMIT 1250000
#define LOW_LIMIT 17500000

// Function prototypes
int RCtime(int RCpin);
void ledOut(int state);
char photocellParse(int reading);

// Signal handling:
volatile sig_atomic_t signal_received = 0;
void sigint_handler(int signal) {
	signal_received = signal;
}

int main(void) {
    // Initialize pigpio library
    if (gpioInitialise() == PI_INIT_FAILED) {
        printf("ERROR: Failed to initialize the GPIO interface.\n");
        return 1;
    }

    // Set up output pins
    gpioSetMode(OUT_LOW, PI_OUTPUT);
    gpioSetMode(OUT_HIGH, PI_OUTPUT);
    gpioSetMode(OUT_STATE, PI_OUTPUT);

    signal(SIGINT, sigint_handler);
    printf("Press CTRL-C to exit.\n");

    // Main loop
    while (!signal_received) {
        gpioWrite(OUT_STATE, PI_HIGH); // Program running indicator
        int reading = RCtime(IN_RC);
	//printf("Reading: %d\n", reading);
        printf("%c", photocellParse(reading));
        fflush(stdout);
        // Optional delay for pacing
        usleep(100000); // 100 milliseconds
    }
    gpioSetMode(OUT_LOW, PI_INPUT);
    gpioSetMode(OUT_HIGH, PI_INPUT);
    gpioSetMode(OUT_STATE, PI_INPUT);
    gpioTerminate();
    printf("\n");
    return 0;
}

// Function to measure RC timing
int RCtime(int RCpin) {
    int reading = 0;

    gpioSetMode(RCpin, PI_OUTPUT);
    gpioWrite(RCpin, 0);
    gpioDelay(100000); // 100 milliseconds

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

// Function to set LED output states
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

// Function to parse the reading and output appropriate character
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

