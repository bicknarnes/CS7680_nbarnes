#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <math.h>
#include <pigpio.h>
#include <unistd.h>
#include <signal.h>

// Hardware / wave generation specs:
#define SAMPLE_RATE 44100       // Sampling rate in Hz
#define NUM_BUTTONS 13          // Number of buttons
#define SPEED 1000000		// define speed for SPI transfer

// GPIO / channel number constants:
#define VOLUME 0.5              // Volume (0.0 to 1.0)
#define VOL_POT 0		// channel number for volume potentiometer
#define TREM 4			// GPIO pin for tremolo button
#define TREM_POT 1		// channel number for tremolo potentiometer
#define TREM_LIGHT 18		// GPIO pin for tremolo on/off light
#define POWER 27		// GPIO pin for power button
#define PITCH_BEND 17		// GPIO pin for pitch bend button
#define PITCH_POT 2		// channel number for tone potentiometer
#define PITCH_LIGHT 14		// GPIO pin for pitch on/off light
#define PITCH 0.5		// default value for pitch bending

// define 2D array of octaves & their respective keys' frequencies:
const double freqs[5][13] = {
	//2:
	{65.4, 69.3, 73.4, 77.8, 82.4, 87.3, 92.5, 98.0, 103.8, 110.0, 116.5, 123.5, 130.8},
	//3:
	{130.8, 138.6, 146.8, 155.5, 164.8, 174.6, 185.0, 196.0, 207.65, 220.0, 233.1, 246.9, 261.6},
	//4:
	{261.6, 277.2, 293.7, 311.1, 329.6, 349.2, 370.0, 392.0, 415.3, 440.0, 466.16, 493.88, 523.25},
	//5:
	{523.25, 554.4, 587.3, 622.25, 659.25, 698.46, 740.0, 784.0, 830.6, 880.0, 932.3, 987.8, 1046.5},
	//6:
	{1046.5, 1108.7, 1174.7, 1244.5, 1318.5, 1396.9, 1480.0, 1568.0, 1661.2, 1760.0, 1864.7, 1975.5, 2093.0}
};

// GPIO pins for the buttons
const int keyPins[NUM_BUTTONS] = {23, 24, 25, 12, 16, 20, 21, 5, 6, 13, 19, 26, 22};

// signal handling to terminate program:
volatile sig_atomic_t signal_received = 0;
void sigint_handler(int signal) {
	signal_received = signal;
}

// Function to read an analog value from the potentiometers through the MCP3008
// (for 3 potentiometers on the first 3 channels, use 0, 1, or 2 as inputs)
int readPot(int channel) {
    unsigned char buf[3];
    buf[0] = 1;  // Start bit
    buf[1] = (8 + channel) << 4;  // Channel selection
    buf[2] = 0;  // Don't care, must be 0

    unsigned char response[3];
    int spiHandle = spiOpen(0, SPEED, 0);  // Open SPI connection
    if (spiHandle < 0) {
        printf("Failed to open SPI\n");
        return -1;
    }

    spiXfer(spiHandle, buf, response, 3);  // Send data and receive the response
    spiClose(spiHandle);  // Close the SPI connection

    // Combine the 10-bit value from the MCP3008
    int value = ((response[1] & 3) << 8) + response[2];
    return value;  // Returns a value between 0 and 1023
}

// Function to read a value from a potentiometer and convert it to a double within the range of 0 to 1 
// (or, since it might be more useful in practice, 0.1 to 0.9)
double potHandler(int potNum) {
	// get a value between 1 and 1024:
	int reading = readPot(potNum) + 1;
	double val = 1.0 - ((double)reading / 1024.0);
	if(val<0.01) val=0.01;
	if(val>0.99) val=0.99;
	return val;
}

// Function to check whether an octave change has been requested
// returns 0 if not, 1 if octave down, 2 if octave up
int checkOctaveChange() {
	//printf("YOU HAVE NOW ENTERED checkOctaveChange\n");
	// if bottom three keys are pressed, indicate to decrease octave
	if(gpioRead(keyPins[0])==0 && gpioRead(keyPins[1])==0 && gpioRead(keyPins[2])==0) {
		return 1;
	}
	// if top three keys are pressed, indicate to increase octave
	if(gpioRead(keyPins[10])==0 && gpioRead(keyPins[11])==0 && gpioRead(keyPins[12])==0) {
		return 2;
	}
	return 0;
}

// Function to return the new octave when an octave change is requested:
int changeOctave(int currentOctave) {
	//printf("YOU HAVE NOW ENTERED changeOctave\n");
	// if down octave requested, try to decrease octave
	if(checkOctaveChange()==1 && currentOctave>0) {
		sleep(1);
		return currentOctave-1;
	}
	// if up octave requested, try to increase octave
	if(checkOctaveChange()==2 && currentOctave<4) {
		sleep(1);
		return currentOctave+1;
	}
	return currentOctave;
}

// Function to handle the button which turns tone potentiometer on/off:
int togglePot(int on, int button) {
	if(on==1 && gpioRead(button)==0) {
		sleep(1);
		return 0;
	}
	if(on==0 && gpioRead(button)==0) {
		sleep(1);
		return 1;
	}
	else return on;
}

// Function to generate and output tones
void playTones(snd_pcm_t *pcm_handle, int frames, int channels, int octave, int pitch, int trem) {
    int bufferSize = frames * channels;
    float buffer[bufferSize];
    double phases[NUM_BUTTONS] = {0.0};
    double phase_increments[NUM_BUTTONS];
    int currentOctave = octave;
    int pitchOn = pitch;
    int tremOn = trem;
    double pitchFactor = PITCH;
    double volume = VOLUME;
    double frequency;
    double tremFactor = 10.0;
    double tremolo = 1.0;
    double tremIncrement;
    double norm = 1.0;

    while (!signal_received) {	
	    // initialize variable to check whether button(s) are pressed
      	    int any_button_pressed = 0;
      
	    // check for octave change, pitch, flange requests:
	    currentOctave = changeOctave(currentOctave);
	    pitchOn = togglePot(pitchOn, PITCH_BEND);
	    tremOn = togglePot(tremOn, TREM);
      
	    // illuminate appropriate LEDs to indicate whether pitch/flange pots are ON:
	    if(pitchOn) gpioWrite(PITCH_LIGHT, PI_HIGH);
	    else gpioWrite(PITCH_LIGHT, PI_LOW);
	    if(tremOn) gpioWrite(TREM_LIGHT, PI_HIGH);
	    else gpioWrite(TREM_LIGHT, PI_LOW);

	    static float prevValue = 0.0;
      
	    // if pitch bend is on, get value:
	    if(pitchOn){
		    // get pitch value
		    pitchFactor = potHandler(PITCH_POT);
	    } else pitchFactor = 0.5;
	    // get volume:
	    volume = potHandler(VOL_POT);
	    // if tremolo is on, get values:
	    if (tremOn) {
		    double tremVal = potHandler(TREM_POT);
		    tremFactor = 0.5 + tremVal * 9.5;
		    tremIncrement = 2 * M_PI * tremFactor / SAMPLE_RATE;
	    }
	    static double tremPhase = 0.0;
      	    for (int j = 0; j < frames; j++) {
	            buffer[j] = 0.0; // Reset buffer for this frame
		    //printf("J: %d\n", j);
	            for (int i = 0; i < NUM_BUTTONS; i++) {
	                if (gpioRead(keyPins[i]) == 0) { // Button is pressed
	
			        // if tremolo is on, calculate value at each index:
			        if(tremOn) {
				        tremolo = 0.5 * (1 + sin(tremPhase));
			        } else tremolo = 1.0;
			    
	 	   	        frequency = freqs[currentOctave][i];
			        phase_increments[i] = 2.0 * M_PI * (frequency*2.0*pitchFactor) / SAMPLE_RATE;
	                  	any_button_pressed = 1;
	                  	buffer[j] += volume * tremolo * sin(phases[i]);
	                 	phases[i] += phase_increments[i];
	                  	if (phases[i] >= 2.0 * M_PI) phases[i] -= 2.0 * M_PI;
			        if(tremOn) {
			    	        tremPhase += tremIncrement;
			    	        if(tremPhase >= 2.0*M_PI) tremPhase -= 2.0*M_PI;
			        }
	                }
	            }
		    
	            // Prevent clipping by normalizing if the sum exceeds ±1.0 (norm)
	            if (buffer[j] > norm) {
	        	buffer[j] = norm;
	            } else if (buffer[j] < -norm) {
	                buffer[j] = -norm;
	            }
	        
		    // LOW PASS FILTER
		    float alpha = 0.2; // filter coefficient
		    buffer[j] = alpha * buffer[j] + (1.0 - alpha) * prevValue;
		    prevValue = buffer[j];
	    }

	    // if no button is pressed, check for power off request and/or wait
            if (!any_button_pressed) {
	        if(gpioRead(POWER)==0) break;
                gpioDelay(10000); // No button pressed, polling delay
                continue;
            }

      	    // Write the buffer to the PCM device
      	    int pcm = snd_pcm_writei(pcm_handle, buffer, frames);
      	    if (pcm == -EPIPE) {
          	snd_pcm_prepare(pcm_handle);
      	    } else if (pcm < 0) {
          	fprintf(stderr, "ERROR: Cannot write to PCM device (%s)\n", snd_strerror(pcm));
            }
	    // if power button is pressed, exit this function to safely terminate the program:
	    if(gpioRead(POWER)==0) break;
    }  
}

int main() {

    // initialise GPIO:
    if (gpioInitialise() == PI_INIT_FAILED) {
        printf("ERROR: Failed to initialize the GPIO interface :(\n");
        return 1;
    }
    // set button pins to input with pull-up resistors:
    gpioSetMode(POWER, PI_INPUT);
    gpioSetPullUpDown(POWER, PI_PUD_UP);
    gpioSetMode(PITCH_BEND, PI_INPUT);
    gpioSetPullUpDown(PITCH_BEND, PI_PUD_UP);
    gpioSetMode(TREM, PI_INPUT);
    gpioSetPullUpDown(TREM, PI_PUD_UP);

    // set LED pins to output:
    gpioSetMode(PITCH_LIGHT, PI_OUTPUT);
    gpioSetMode(TREM_LIGHT, PI_OUTPUT);

    // Audio Output configuration:
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    int pcm, dir;
    unsigned int rate = SAMPLE_RATE;
    snd_pcm_uframes_t frames = 32; // Number of frames per period
    int channels = 1; // Mono audio

    // Configure key button pins as input with pull-up resistors
    for (int i = 0; i < NUM_BUTTONS; i++) {
        gpioSetMode(keyPins[i], PI_INPUT);
        gpioSetPullUpDown(keyPins[i], PI_PUD_UP);
    }

    // Open the PCM device for playback
    pcm = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (pcm < 0) {
        fprintf(stderr, "ERROR: Cannot open audio device (%s)\n", snd_strerror(pcm));
        gpioTerminate();
        return 1;
    }

    // Allocate hardware parameters object
    snd_pcm_hw_params_malloc(&params);

    // Fill it with default values
    snd_pcm_hw_params_any(pcm_handle, params);

    // Set the desired hardware parameters
    snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_FLOAT);
    snd_pcm_hw_params_set_channels(pcm_handle, params, channels);
    snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, &dir);
    snd_pcm_hw_params_set_period_size_near(pcm_handle, params, &frames, &dir);
    snd_pcm_hw_params_set_period_size(pcm_handle, params, frames, dir);
    snd_pcm_hw_params_set_periods(pcm_handle, params, 2, dir); // Use fewer periods for lower latency

    // Write the parameters to the driver
    pcm = snd_pcm_hw_params(pcm_handle, params);
    if (pcm < 0) {
        fprintf(stderr, "ERROR: Cannot set hardware parameters (%s)\n", snd_strerror(pcm));
        gpioTerminate();
        return 1;
    }

    printf("Welcome to the pi piano :)\n");
    // Initialize variables for starting octave, pitch bend, and tremolo states
    int currentOctave = 2;
    int pitchOn = 0;
    int tremOn = 0;

    // signal handling:
    signal(SIGINT, sigint_handler);

    // call function that handles playing notes:
    playTones(pcm_handle, frames, channels, currentOctave, pitchOn, tremOn);

    // once we leave the playTones() function, safely exit the program:
    printf("Exiting the pi piano program :)\n");
    // ensure LEDs are off when exiting program:
    gpioWrite(PITCH_LIGHT, PI_LOW);
    gpioWrite(TREM_LIGHT, PI_LOW);
    // return output pins to safe state:
    gpioSetMode(PITCH_LIGHT, PI_INPUT);
    gpioSetMode(TREM_LIGHT, PI_INPUT);
    // clean up audio output hardware:
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    snd_pcm_hw_params_free(params);
    // terminate GPIO:
    gpioTerminate(); 
    //printf("\n");
    return 0;
}
