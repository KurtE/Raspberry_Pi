#include <iostream>
#include "stdio.h"
#include "unistd.h"
#include <time.h>
#include <pthread.h>


//=========================================================================
//#include "fast_gpio.h"
#include <time.h>
#include "mraa.h"
#include <string>
#include "memory.h"


mraa_gpio_context gpioDBG;


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
#define GPIO_INDEX 2


unsigned long time_s;
unsigned long time_e;


__syscall_slong_t echotime;


unsigned long ulDRStart, ulDeltaDr;
unsigned long ulDeltaSum = 0;
unsigned long ulCnt = 0;
unsigned long DoPing(  mraa_gpio_context gpio) {
    mraa_gpio_dir(gpio, MRAA_GPIO_OUT);
    mraa_gpio_write(gpioDBG, HIGH);
    mraa_gpio_write(gpio, HIGH);
    usleep(STARTDELAY);
    mraa_gpio_write(gpio, LOW);
    mraa_gpio_write(gpioDBG, LOW);
    
    ulDRStart = micros2();
    mraa_gpio_dir(gpio, MRAA_GPIO_IN);
    ulDeltaDr = micros2() - ulDRStart;
    
    while (mraa_gpio_read(gpio) == LOW)
        ; //pthread_yield();
    time_s = micros2();
    mraa_gpio_write(gpioDBG, HIGH);

    while (mraa_gpio_read(gpio) == HIGH)
        ; //pthread_yield();
    time_e = micros2();
    mraa_gpio_write(gpioDBG, LOW);

    ulDeltaSum += ulDeltaDr;
    ulCnt++;
    cout << "dt dir: " << ulDeltaDr << "(" << (ulDeltaSum/ulCnt) <<" ): ";
    
    return time_e - time_s;
}

int main(int argc, char **argv)
{
  mraa_result_t rtv = mraa_init();
  if (rtv != MRAA_SUCCESS && rtv != MRAA_ERROR_PLATFORM_ALREADY_INITIALISED)
  {
    cout << "MRAA Init Failed,Return Value is ";
    cout << rtv << endl;
    return 0;
  }
  fprintf(stdout, "MRAA Version: %s\nStarting Read\n",mraa_get_version());

  mraa_gpio_context gpio;
  gpio = mraa_gpio_init(GPIO_INDEX);
  gpioDBG = mraa_gpio_init(3);
  if (gpio == NULL)
  {
    cout << "Init GPIO Out Failed" << endl;
    return 0;
  }

  mraa_gpio_dir(gpio, MRAA_GPIO_OUT);
  mraa_gpio_dir(gpioDBG, MRAA_GPIO_OUT);
  mraa_gpio_use_mmaped(gpio, true);
  mraa_gpio_use_mmaped(gpioDBG, true);
  bool finishcycle = false;

  
  usleep(1000000);  // give time for the ping to settle...
  
  for (;;)
  {
    echotime = DoPing(gpio);
    
    cout << echotime << endl;
    usleep(500000);
  }

  mraa_gpio_close(gpio);


  return 0;
}



