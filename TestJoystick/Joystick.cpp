#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

#define JOY_DEV "/dev/js0"

int main()
{
	int joy_fd, *axis= NULL, num_of_axis = 0, num_of_buttons=0,x;
	
	char *button = NULL , name_of_joystick[80];
	
	struct js_event js;
	// input of joystick values to variable joy_fd
		
	if((joy_fd = open(JOY_DEV,O_RDONLY))== -1 )
	{
		printf(" couldn't open the joystick \n " );
		
		return -1;
	}

	ioctl(joy_fd, JSIOCGAXES , &num_of_axis);
	ioctl(joy_fd, JSIOCGBUTTONS , &num_of_buttons);
	ioctl(joy_fd, JSIOCGNAME(80), &name_of_joystick);
	{
		axis = (int *) calloc(num_of_axis , sizeof(int));
		button = (char *) calloc( num_of_buttons , sizeof (char));
	}
	printf( " Joy stick detected : %s \n \t %d axis \n\t %d buttons \n\n" ,name_of_joystick , num_of_axis , num_of_buttons);

	fcntl( joy_fd, F_SETFL , O_NONBLOCK ); // use non - blocking methods

	while(1) // infinite loop

	{
		// read the joystick 
		
		read (joy_fd, &js , sizeof(struct js_event));

		// see what to do with the event

		switch(js.type & ~ JS_EVENT_INIT)
		{
			case JS_EVENT_AXIS :
			
				axis [ js.number ] = js.value;
			
			case JS_EVENT_BUTTON :
			
				button [js.number ] = js.value;
		}

		// print the results


		printf( " X: %6d y: %6d ", axis[0] , axis[1]);
		
		if( num_of_axis > 2)

			printf( " Z: %6d " , axis[2] );
		if( num_of_axis > 3)

			printf( " R : %6d " , axis [3]);
 
		for( x=0 ; x<num_of_buttons ; ++x)
		
			printf( "B %d : %d " , x, button [x]);
		fflush(stdout);

	}
	
	close(joy_fd);
	return 0;
}
		  
					




