#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "fakenews.h"

#define DEFAULT_TEMP 72

// Global Variables
int current_temp;
int set_temp=DEFAULT_TEMP;

// Function to set non-blocking mode for stdin [syntax from ChatGPT]
void set_nonblocking_input() {
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

// Function to get temperature reading
int get_temp() {
	return fakenews();
}

// Function to display temperature reading
void display_temp() {
	printf("Current Temperature: %dF\n", current_temp);
}

// Function to set & display new temperature
void adjust_temp(int new_temp) {
	set_temp=new_temp;
	printf("Set Temperature: %dF\n", set_temp);
}

// Function to read user input
void get_input(char *input){
	if (input[0]=='t' && input[1]==' ') {
		// Set new temperature
		int new_temp=atoi(&input[2]); // [syntax from ChatGPT]
		adjust_temp(new_temp);
		sleep(2);
		display_temp();
	} else if (input[0]=='x') {
		// Exit program
		exit(0);
	} else {
		// invalid input
		printf("Invalid input\n");
	}
}

// Main program
int main() {
	// Define string to receive input:
	char input[10];
	// Print initial messages for program:
	printf("\nWelcome to Den\n");
	printf("========================================================\n");
	printf("Enter 't nn' to set temperature to nn, or 'x' to exit :)\n");
	printf("========================================================\n");
	adjust_temp(set_temp);

	// Get & display current temp
	current_temp=get_temp();
	display_temp();

	// Set non-blocking input
	set_nonblocking_input();
	sleep(3);

	while(1) {
		// retrieve sensor data & check for changes
		int new_temp=get_temp();
		if(new_temp!=current_temp) {
			current_temp=new_temp;
			display_temp();
			sleep(3);
		}

		// Check if there's keyboard input [syntax from ChatGPT]
        	int result = read(STDIN_FILENO, &input, sizeof(input)-1);
        	if (result > 0) {
			// ensure null-termination of string:
			input[result]='\0';
			// process input:
			get_input(input);
			sleep(3);
        	}
	}
	return 0;
}
