#include "fakenews.h"

/*
 *  fakenews() : Function to generate random temperature values to simulate
 *               a temperature sensor issuing random values between 55F and
 *               85F moving by one degree at a time at random times.
 *               Initial temp starts at 72F each time.
 */
int temp = 72;

int fakenews() {

	int maxt = 85;
	int mint = 55;
	int rmove = 0;
	
	rmove = (rand() % 10);
	if (rmove==0) temp = temp-1;
	if (rmove==9) temp = temp+1;
	if (temp<mint) temp = mint;
	if (temp>maxt) temp = maxt;

	return temp;
}

