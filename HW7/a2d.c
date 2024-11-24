#include <stdio.h>
#include <stdlib.h>
#include <pigpio.h>
#include <signal.h>
#include <unistd.h>
/*
        gcc -pthread -o a2d a2d.c -lpigpio -lrt
        a2d [num_of_loops] [speed_of_transmission]
                defaults to 1000000 in both cases
*/

const int RedLED = 21;
volatile sig_atomic_t signal_received = 0;

void sigint_handler(int signal){
    signal_received = 1;
}

int main(int argc, char *argv[]){
    if (gpioInitialise()==PI_INIT_FAILED){
        printf("ERROR: Failed to initialize GPIO interface...\n");
        return 1;
    }
    int i, h, v1, v2, loops, speed, light, freq;
    double start, diff, sps;
    unsigned char buf[3];
    
    gpioSetMode(RedLED, PI_OUTPUT);
    signal(SIGINT, sigint_handler);
    gpioSetPWMrange(RedLED, 255);
    
    //while(!signal_received){ 
    if (argc > 1) loops = atoi(argv[1]);
    else loops = 1000000;
    if (argc > 2) speed = atoi(argv[2]);
    else speed = 1000000;

    h = spiOpen(0, speed, 0);
    if (h < 0) return 2;

    while(!signal_received) {
        start = time_time();
        for (i=0; i<loops; i++){
	          // read pot 1
            buf[0] = 1;
            buf[1] = 0x80;
            buf[2] = 0;
	
            spiXfer(h, buf, buf, 3);
            v1 = ((buf[1]&3)<<8) | buf[2];
            light = v1/4;

	          // read pot 2
	          buf[1] = 0x96;
	          buf[2] = 0;
	          spiXfer(h, buf, buf, 3);
	          v2 = ((buf[1]&3)<<8) | buf[2];
	          freq = (v2*2000);
	          if(freq>1000000) freq=1000000;

	          // Flash light according to brightness and frequency:
	          if (freq < 950000) {       // exclude the high end due to inconsistent response from potentiometer
		            gpioPWM(RedLED, 0); 
	              usleep((1000000-freq));
	          }
	          if (freq > 10000){         // exclude the low end due to inconsistent response from potentiometer
	    	        gpioPWM(RedLED, light);
	    	        usleep(100000);
	          }
          
            // exit loop if CTRL-C
            if (signal_received) break;

        }
        diff = time_time() - start;
    
        fprintf(stderr, "sps=%.1f @ %d bps (%d/%.1f)\n", 
            (double)loops / diff, speed, loops, diff);
        
    }
    // clean up and terminate gpio
    spiClose(h);
    gpioWrite(RedLED, PI_LOW);
    gpioSetMode(RedLED, PI_INPUT);
    gpioTerminate();
    return 0; 
}
