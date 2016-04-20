/**
 * @file [gpio_extra.c]
 * @brief [This file turns an led on if the switch is pressed, it turns the led off if the switch is released.]
 * @author [Jacob Allenwood]
 * @date [September 30, 2015]
 * @setup [Raspberry Pi 2 - gpio4 runs the led - gpio17 is between the switch and the pulldown resistor to ground]
 */

#include <stdio.h>
#include <fcntl.h>	/* open() */
#include <unistd.h>	/* close() */
#include <string.h>

#define ENABLE_GPIO_FILE "/sys/class/gpio/export"
#define SET_DIRECTION_FILE "/sys/class/gpio/gpio17/direction"
#define GPIO17_VALUE_FILE "/sys/class/gpio/gpio17/value"

#define ENABLE_GPIO4_FILE "/sys/class/gpio/export"
#define SET_DIRECTION4_FILE "/sys/class/gpio/gpio4/direction"
#define GPIO4_VALUE_FILE "/sys/class/gpio/gpio4/value"

// struct containing switch value information --------------
typedef struct debounced_switch_values {
	char values[10];// array holding the most recent 10 read values from the gpio17 value file
	int high_status;// number of high values read
	int low_status;	// number of low values read
} SWITCH_T;

// function declarations -----------------------------------
SWITCH_T * init_switch(void); // initialize switch values structure
int init_gpio(void);	// set up gpio17
int write_to_file(char * file, char * value);	// open write and close file
int get_debounce_vals(SWITCH_T * S);	// read state of the switch (pressed or not pressed)
void switch_led(SWITCH_T * S);	// print the state of the switch to the console
// ---------------------------------------------------------



int main(int argc, char **argv) {

	// initialize gpio17 that is hooked up to the switch
	if(init_gpio()) {
		perror("could not initialize gpio17");
		return 1;
	}

	// S points to the switch values structure
	SWITCH_T * S = init_switch();

	while(1) {
		// read the switch values
		if(get_debounce_vals(S)) {
			perror("could not read values");
		}
		// print the state of the switch and light led accordingly
		switch_led(S);
	}

	return 0;
}


/**
 * @brief [initalize structure which contains information on the state of the switch]
 * @return [pointer to the structure]
 */
SWITCH_T * init_switch(void) {
	SWITCH_T * S;
	S = malloc(sizeof(SWITCH_T));

	S->high_status = 0;
	S->low_status = 0;

	return S;
}

/**
 * @brief [initializes gpio17 for reading its value file (determining state of the switch) and gpio4 for writing to its value file (turning led on/off)]
 * @details [open write and close the enable and direction files for gpio17 and gpio4]
 * @return [1 for error, 0 for success]
 */
int init_gpio(void) {
	// enable gpio17 (read switch state)
	if(write_to_file(ENABLE_GPIO_FILE, "17")) {
		perror("error enabling gpio17");
		return 1;
	}
	// set gpio17 to an input
	if(write_to_file(SET_DIRECTION_FILE, "in")) {
		perror("error setting gpio17 as input");
		return 1;
	}

	// enable gpio4 (led) and close the file
	if(write_to_file(ENABLE_GPIO4_FILE, "4")) {
		perror("could not enable gpio4");
		return 1;
	}
	
	// make gpio4 an output and close the file
	if(write_to_file(SET_DIRECTION4_FILE, "out")) {
		perror("could not make gpio4 an output");
		return 1;
	}

	return 0;
}

/**
 * @brief [Open file, write to file and close file]
 * @details [Given the parameters, this function will open the specified file, write the specified value, and close the file]
 * 
 * @param file [string containing path to file to operate on]
 * @param value [what to write to the file]
 * @return [0 on success, 1 on error]
 */
int write_to_file(char * file, char * value) {

	FILE * fp;
	// open file and error check
	fp = fopen(file, "w");	
	if (!fp) {
		perror("file open failed");
		return 1;
	}
	// write value to file and error check
	if(fprintf(fp, "%s\n", value) == 0) {
		perror("error writing to file");
		return 1;
	}
	// clear write buffer
	fflush(fp);
	// close file
	if(fclose(fp)) {
		perror("close error");
		return 1;
	}

	return 0;
}

/**
 * @brief [read the switch state values from the gpio17 values file]
 * @details [reading 10 values before determining the state of the switch is a form of debouncing, ensuring that switch is actually pressed, or actually released]
 * 
 * @param S [pointer to the structure containing the switch information]
 * @return [1 for error, 0 on success]
 */
int get_debounce_vals(SWITCH_T * S) {
		int i, j;
		char read_buffer[10];
		FILE * fd;

		// grab 10 values from file
		for(i = 0; i < 10; i++) {

			// open file to read value of switch
			fd = fopen(GPIO17_VALUE_FILE, "r");
			if(!fd) {
				perror("could not open value file");
				return 1;
			}

			// read byte of data (switch high or low)
			if(fread(read_buffer, 1, 1, fd) < 1) {
				perror("error reading from file");
				return 1;
			}
			// get either a high or low value from file
			S->values[i] = (read_buffer[0] - 48); // ascii 0 is "48" ascii 1 is "49"

			fflush(fd);

			if(fclose(fd)) {
				perror("close error");
				return 1;
			}
		}

		// reset debounced values
		S->high_status = 0;
		S->low_status = 0;
		// count to see if there was a continuos stream of 1 or 0 (software debounce)
		for (j = 0; j < 10; j++) {
			if((S->values[j]) == 1) {
				S->high_status++;
			} else {
				S->low_status++;
			}
		}

		return 0;
}

/**
 * @brief [print the state of the switch (pressed or released) and turn the led on or off depending on switch state]
 * @details [the switch state is determined by the condition that the most 10 recent values read from the values file 
 * (found in S->values array) are either all high or all low, if the switch was determined as pressed, then turn the led on,
 * otherwise, the switch was released and the led should be off]
 * @return [void]
 */
void switch_led(SWITCH_T * S) {
	// if 10 continuous "1" then print switch is on
	if(S->high_status == 10) {
		printf("Switch pressed!\n");
		// turn led on when switch is pressed
		if(write_to_file(GPIO4_VALUE_FILE, "1")) {
			perror("could not write to gpio4 value file");
		}
	}
	// if 10 continuous "0" then print switch is off
	if(S->low_status == 10) {
		printf("Switch released!\n");
		// turn off led when switch is released
		if(write_to_file(GPIO4_VALUE_FILE, "0")) {
			perror("could not write to gpio4 value file");
		}
	}
}
