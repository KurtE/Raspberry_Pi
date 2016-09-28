#include <iostream>
#include "stdio.h"
#include "unistd.h"
#include <time.h>
#include <pthread.h>
#include <signal.h>


//=========================================================================
//#include "fast_gpio.h"
#include <time.h>
#include "mraa.h"
#include <string>
#include "memory.h"


mraa_gpio_context gpioDBG;
mraa_gpio_context gpioDBG2;


using namespace std;


static float cpufreq = 0;
static uint64_t tsc_init = 0;
static float clocks_per_ns = 0;


unsigned long micros2(void)
{
  struct timespec t;
  t.tv_sec = t.tv_nsec = 0;
  clock_gettime(CLOCK_REALTIME, &t);
  return (unsigned long)(t.tv_sec) * 1000000L + t.tv_nsec / 1000L;


}


//=========================================================================
using namespace std;


#define HIGH 1
#define LOW  0
#define STARTDELAY 2
#define GPIO_INDEX 27
#define GPIO_DEBUG 29
#define GPIO_DEBUG2 16

unsigned long time_s;
unsigned long time_e;


__syscall_slong_t echotime;


unsigned long ulDRStart, ulDeltaDr;
unsigned long ulDeltaSum = 0;
unsigned long ulCnt = 0;
unsigned long DoPing(  mraa_gpio_context gpio) {
    mraa_gpio_write(gpioDBG2, HIGH);
  	unsigned long ulDWStart = micros2();
    mraa_gpio_dir(gpio, MRAA_GPIO_OUT);
    unsigned long ulDeltaDw = micros2() - ulDWStart;
    mraa_gpio_write(gpioDBG2, LOW);
    mraa_gpio_write(gpioDBG, HIGH);
    mraa_gpio_write(gpio, HIGH);
    usleep(STARTDELAY);
    mraa_gpio_write(gpio, LOW);
    mraa_gpio_write(gpioDBG, LOW);
    
    mraa_gpio_write(gpioDBG2, HIGH);
    ulDRStart = micros2();
    mraa_gpio_dir(gpio, MRAA_GPIO_IN);
    ulDeltaDr = micros2() - ulDRStart;
    mraa_gpio_write(gpioDBG2, LOW);
    uint32_t loop_count = 0xffff;
    while ((mraa_gpio_read(gpio) == LOW) && (loop_count--))
        ; //pthread_yield();
    time_s = micros2();
    mraa_gpio_write(gpioDBG, HIGH);

    loop_count = 0xfffff;
    while ((mraa_gpio_read(gpio) == HIGH) && (loop_count--))
        ; //pthread_yield();
    time_e = micros2();
    mraa_gpio_write(gpioDBG, LOW);

    ulDeltaSum += ulDeltaDr;
    ulCnt++;
    cout << "dt dir: " << ulDeltaDw << " " << ulDeltaDr << "(" << (ulDeltaSum/ulCnt) <<" ): ";
    
    return time_e - time_s;
}

// Building under linux
//====================================================================================================
// SignalHandler - Try to free up things like servos if we abort.
//====================================================================================================
uint8_t g_fSignaled = false;
void SignalHandler(int sig){
    printf("Caught signal %d\n", sig);

    if (g_fSignaled) {
        printf("Second signal Abort\n");
        exit(1);
    }
    // Set global telling main to abort and return
    g_fSignaled = true;

}

int main(int argc, char **argv)
{
  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = SignalHandler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, NULL);

  mraa_result_t rtv = mraa_init();
  //if (rtv != MRAA_SUCCESS && rtv != MRAA_ERROR_PLATFORM_ALREADY_INITIALISED)
  if (rtv != MRAA_SUCCESS)
  {
    cout << "MRAA Init Failed,Return Value is ";
    cout << rtv << endl;
    return 0;
  }
  fprintf(stdout, "MRAA Version: %s\nStarting Read\n",mraa_get_version());

  mraa_gpio_context gpio;
  gpio = mraa_gpio_init(GPIO_INDEX);
  gpioDBG = mraa_gpio_init(GPIO_DEBUG);
  gpioDBG2 = mraa_gpio_init(GPIO_DEBUG2);
  if (gpio == NULL)
  {
    cout << "Init GPIO Out Failed" << endl;
    return 0;
  }

  mraa_gpio_dir(gpio, MRAA_GPIO_OUT);
  mraa_gpio_dir(gpioDBG, MRAA_GPIO_OUT);
  mraa_gpio_dir(gpioDBG2, MRAA_GPIO_OUT);
  mraa_gpio_use_mmaped(gpio, true);
  mraa_gpio_use_mmaped(gpioDBG, true);
  mraa_gpio_use_mmaped(gpioDBG2, true);
  bool finishcycle = false;

  
  usleep(1000000);  // give time for the ping to settle...
  
  while(!g_fSignaled) 
  {
    echotime = DoPing(gpio);
    
    cout << echotime << endl;
    usleep(500000);
  }

  cout << "Signal caught closing down" << endl;
  mraa_gpio_close(gpio);
  mraa_gpio_close(gpioDBG);
  mraa_gpio_close(gpioDBG2);

  return 0;
}



