/**
 * @file [blink_led.c]
 * @brief [This file turns gpio4 high and low forever]
 * @author [Jacob Allenwood]
 * @date [September 28, 2015]
 * @setup [Raspberry Pi 2 - gpio4 is connected to the pullup resistor which is connected to the led going to ground]
 */
#include <stdio.h>
#include <fcntl.h>	/* open() */
#include <unistd.h>	/* close() */
#include <string.h>

// file path defines ---------------------------------------
#define ENABLE_GPIO_FILE "/sys/class/gpio/export"
#define SET_DIRECTION_FILE "/sys/class/gpio/gpio4/direction"
#define SET_VALUE_FILE "/sys/class/gpio/gpio4/value"

// function declarations -----------------------------------
int write_to_file(char * file, char * value);	// open write and close file
// ---------------------------------------------------------


int main(int argc, char **argv) {

// intialize gpio -------------------------------------------------
	// enable gpio 4 and close the file
	if(write_to_file(ENABLE_GPIO_FILE, "4")) {
		perror("could not enable gpio4");
		return 1;
	}
	
	// make gpio4 an output and close the file
	if(write_to_file(SET_DIRECTION_FILE, "out")) {
		perror("could not make gpio4 an output");
		return 1;
	}

// blink LED forever ----------------------------------------------
	while(1) {
		// blink on and off
		write_to_file(SET_VALUE_FILE, "0");
		usleep(500000);
		write_to_file(SET_VALUE_FILE, "1");	
		usleep(500000);
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
