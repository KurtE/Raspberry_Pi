#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#include <unistd.h>
#include <time.h>
#include <inttypes.h>
#include <signal.h>


#define JOY_DEV "/dev/input/js0"

uint8_t g_bContinue = 1;

//====================================================================================================
// SignalHandler - Try to free up things like servos if we abort.
//====================================================================================================
void SignalHandler(int sig){
    printf("Caught signal %d\n", sig);
    g_bContinue = 0;    // tell main loop to exit.
}

int main()
{
    int joy_fd, *axis= NULL, *min_values=NULL, *max_values=NULL, num_of_axis = 0, num_of_buttons=0,x;
    
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
        min_values = (int *) calloc(num_of_axis , sizeof(int));
        max_values = (int *) calloc(num_of_axis , sizeof(int));
        button = (char *) calloc( num_of_buttons , sizeof (char));
    }
    printf( " Joy stick detected : %s \n \t %d axis \n\t %d buttons \n\n" ,name_of_joystick , num_of_axis , num_of_buttons);
    
    for (int i=0; i<num_of_axis; i++) 
    {
        min_values[i] = 0;
        max_values[i] = 0;
    }
        

    // Setup signal handler
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = SignalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

//	fcntl( joy_fd, F_SETFL , O_NONBLOCK ); // use non - blocking methods
    usleep(2000000);
    
    // BUGBUG:: lets not process all of the initial messages that reset our states...
    int num_of_axis_and_buttons = num_of_axis + num_of_buttons;
    while (g_bContinue && num_of_axis_and_buttons) 
    {
        int count_read __attribute__((unused));
        count_read = read (joy_fd, &js , sizeof(struct js_event));
        num_of_axis_and_buttons--;
    }
    
    while(g_bContinue) // infinite loop

    {
        // read the joystick 
        
        int count_read __attribute__((unused));
        count_read = read (joy_fd, &js , sizeof(struct js_event));

        // see what to do with the event

        switch(js.type & ~ JS_EVENT_INIT)
        {
            case JS_EVENT_AXIS :
            
                axis [ js.number ] = js.value;
//                if (js.number < 25)                     // BUGBUG temporary hack...
                    printf("%d : %d\n", js.number, js.value);
                if (js.value < min_values[js.number])
                    min_values[js.number] = js.value;
                if (js.value > max_values[js.number])
                    max_values[js.number] = js.value;
                break;
            
            case JS_EVENT_BUTTON :
                button [js.number ] = js.value;
                printf("BTN %d = %d\n", js.number, js.value);
                break;
        }
        fflush(stdout);

    }

    // print out min and max values
    printf("\nAxis Min and Max Values\n");
    for (int i=0; i<num_of_axis; i++) 
    {
        if (min_values[i] != 0 || max_values[i] != 0)
            printf("%d: %d - %d\n", i, min_values[i], max_values[i]);
    }
        

    close(joy_fd);
    return 0;
}
          
                    




