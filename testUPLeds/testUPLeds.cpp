#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#define USE_MRAA
#ifdef USE_MRAA
#include "mraa.h"
#endif

volatile uint8_t g_continue = true;

typedef enum {GREEN_LED = 0, YELLOW_LED = 1, RED_LED = 2, BLUE_LED = 3} Leds;

extern bool set_led(Leds led, uint8_t value);

//====================================================================================================
// SignalHandler - Try to free up things like servos if we abort.
//====================================================================================================
void SignalHandler(int sig){
    printf("Caught signal %d\n", sig);

    if (!g_continue) {
        printf("Second signal Abort\n");
        exit(1);
    }
    // Set global telling main to abort and return
    g_continue = false;
}


int main(int argc, char** argv)
{
	bool has_blue_led = false;
	uint8_t exit_loop_count = 8;

    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = SignalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

	// Pretty simple setup.
	printf("\n\n*** Start testing leds ***\n");
#ifdef USE_MRAA
	mraa_result_t rtv = mraa_init();
  	//if (rtv != MRAA_SUCCESS && rtv != MRAA_ERROR_PLATFORM_ALREADY_INITIALISED)
  	if (rtv != MRAA_SUCCESS)
  	{
  		printf("MRAA Init Failed,Return Value is %d\n", rtv);
    	return 0;
  	}
  	printf("MRAA Version: %s\nStarting Read\n",mraa_get_version());
  	printf("MRAA platform: %s\n",  mraa_get_platform_name());
  	mraa_platform_t platform = mraa_get_platform_type();
  	if (platform == MRAA_UP) {
  		printf("Running on UP board - 3 leds\n");
  	} else if (platform == MRAA_UP2) {
  		printf("Running on UP2 board - 4 leds\n");
  		has_blue_led = true;
  		exit_loop_count = 16;
  	}
#endif

	//usleep(250000L);
	while(g_continue) {
		for (int i = 0; i < exit_loop_count; i++) {
			set_led(GREEN_LED, (i&1)? 1 : 0);
			set_led(YELLOW_LED, (i&2)? 1 : 0);
			set_led(RED_LED, (i&4)? 1 : 0);
			if (has_blue_led) set_led(BLUE_LED, (i&8)? 1 : 0);
			usleep(250000L);	// sleep 250ms
		}
	}
	// Make sure they are all off. 
	set_led(GREEN_LED, 0);
	set_led(YELLOW_LED, 0);
	set_led(RED_LED, 0);
	if (has_blue_led) set_led(BLUE_LED, 0);
}

bool set_led(Leds led, uint8_t value) {
	int led_handle = -1;
	char value_string[4];

	// Open file
	switch (led) {
		case GREEN_LED:
			led_handle = open("/sys/class/leds/upboard:green:/brightness", O_WRONLY);
			break;
		case YELLOW_LED:
			led_handle = open("/sys/class/leds/upboard:yellow:/brightness", O_WRONLY);
			break;
		case RED_LED:
			led_handle = open("/sys/class/leds/upboard:red:/brightness", O_WRONLY);
			break;
		case BLUE_LED:
			led_handle = open("/sys/class/leds/upboard:blue:/brightness", O_WRONLY);
			break;
	}
	if (led_handle == -1) {
		printf("set_led(%d, %d) failed to open file: %d\n\r", (uint8_t)led, value, errno);
		return false;
	}
	// Write file
	int length = snprintf(value_string, sizeof(value_string), "%d", value);
	if (write(led_handle, value_string, length * sizeof(char)) == -1) {
		printf("set_led(%d, %d) failed to write to file: %d\n\r", (uint8_t)led, value, errno);
		close(led_handle);
		return false;
	}
	close(led_handle);
	//printf("set_led(%d, %d) succeed\n\r", (uint8_t)led, value);
	return true;
}
