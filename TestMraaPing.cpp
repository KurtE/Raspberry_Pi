#include <iostream>

#include "stdio.h"

#include "unistd.h"

#include <time.h>

 

 

#include "fast_gpio.h"

 

 

using namespace std;

 

 

#define HIGH 1

#define LOW  0

#define STARTDELAY 20

#define GPIO_IN_INDEX 3

#define GPIO_OUT_INDEX 2

 

 

unsigned long time_s;

unsigned long time_e;

 

 

__syscall_slong_t echotime;

 

 

 

 

int getrange(__syscall_slong_t timeus)

{

  if (timeus <= 60000 && timeus >= 1)

  {

  return (((timeus*34)/100)) / 2;

  }

}

 

 

 

 

void interrupt(void * args)

{

  if (time_s == 0 && time_e == 0)

  {

  time_s = micros();

  }

  else if (time_s != 0 && time_e == 0)

  {

  time_e = micros();

  echotime = time_e - time_s;

  }

}

 

 

int main(int argc, char **argv)

{

  mraa_result_t rtv = mraa_init();

  timeInit();

  if (rtv != MRAA_SUCCESS && rtv != MRAA_ERROR_PLATFORM_ALREADY_INITIALISED)

  {

  cout << "MRAA Init Failed,Return Value is ";

  cout << rtv << endl;

  return 0;

  }

  fprintf(stdout, "MRAA Version: %s\nStarting Read on IO3\n",mraa_get_version());

 

  mraa_gpio_context gpio_out;

  mraa_gpio_context gpio_in;

 

 

  gpio_out = mraa_gpio_init(GPIO_OUT_INDEX);

  if (gpio_out == NULL)

  {

  cout << "Init GPIO Out Failed" << endl;

  return 0;

  }

  gpio_in = mraa_gpio_init(GPIO_IN_INDEX);

  if (gpio_in == NULL)

  {

  cout << "Init GPIO In Failed" << endl;

  return 0;

  }

 

 

  mraa_gpio_dir(gpio_out, MRAA_GPIO_OUT);

  mraa_gpio_dir(gpio_in, MRAA_GPIO_IN);

  mraa_gpio_use_mmaped(gpio_out, true);

  mraa_gpio_use_mmaped(gpio_in, true);

  //mraa_gpio_isr(gpio_in, MRAA_GPIO_EDGE_BOTH, &interrupt, NULL);

  bool finishcycle = false;

  for (;;)

  {

  time_s = 0;

  time_e = 0;

  mraa_gpio_write(gpio_out, HIGH);

  usleep(STARTDELAY);

  mraa_gpio_write(gpio_out, LOW);

 

  while (mraa_gpio_read(gpio_in) == LOW)

  {

  if (time_s != 0)

  {

  time_e = micros2();

  echotime = time_e - time_s;

  break;

  }

  while (mraa_gpio_read(gpio_in) == HIGH)

  {

  if (time_s == 0)

  {

  time_s = micros2();

  }

  }

  }

 

 

  usleep(1000000);

  cout << echotime << endl;

  cout << "Echo distance is ";

  cout << getrange(echotime);

  cout << " mm" << endl;

  }

 

  mraa_gpio_close(gpio_out);

  mraa_gpio_close(gpio_in);

 

 

  return 0;

}

 

 

 

fast_gpio.h

 

 

#include <time.h>

#include "mraa.h"

#include <string>

#include "memory.h"

 

 

using namespace std;

 

 

static float cpufreq = 0;

static uint64_t tsc_init = 0;

static float clocks_per_ns = 0;

 

 

uint64_t rdtsc(void)

{

  uint32_t lo, hi;

  uint64_t returnVal;

  /* We cannot use "=A", since this would use %rax on x86_64 */

  __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));

  returnVal = hi;

  returnVal <<= 32;

  returnVal |= lo;

 

 

  return returnVal;

}

 

 

unsigned long micros2(void)

{

  struct timespec t;

  t.tv_sec = t.tv_nsec = 0;

  clock_gettime(CLOCK_REALTIME, &t);

  return (unsigned long)(t.tv_sec) * 1000000L + t.tv_nsec / 1000L;

 

 

}

 

 

unsigned long micros(void)

{

  uint64_t tsc_cur = rdtsc(), diff = 0, divisor = 0;

  divisor = (cpufreq);

  diff = tsc_cur - tsc_init;

  return (unsigned long)(diff / divisor);

}

 

 

 

 

int timeInit(void)

{

  int cpufreq_fd, ret;

  char buf[0x400];

  char * str = 0, *str2 = 0;

  char * mhz_str = "cpu MHz\t\t: ";

 

 

  /* Grab initial TSC snapshot */

  tsc_init = rdtsc();

 

 

  cpufreq_fd = open("/proc/cpuinfo", O_RDONLY);

  if (cpufreq_fd < 0){

  fprintf(stderr, "unable to open /proc/cpuinfo\n");

  return -1;

  }

  memset(buf, 0x00, sizeof(buf));

  ret = read(cpufreq_fd, buf, sizeof(buf));

  if (ret < 0){

  fprintf(stderr, "unable to read cpuinfo !\n");

  close(cpufreq_fd);

  return -1;

  }

  close(cpufreq_fd);

  str = strstr(buf, mhz_str);

  if (!str){

  fprintf(stderr, "Buffer %s does not contain CPU frequency info !\n", buf);

  return -1;

  }

 

 

  str += strlen(mhz_str);

  str2 = str;

 

 

  while (str2 < buf + sizeof(buf) - 1 && *str2 != '\n'){

  str2++;

  }

  if (str2 == buf + sizeof(buf - 1) && *str2 != '\n'){

  fprintf(stderr, "malformed cpufreq string %s\n", str);

  return -1;

  }

  *str2 = '\0';

  cpufreq = atof(str);

 

 

 

 

  printf("cpufrequency is %f mhz\n", cpufreq);

 

 

  /* Calculate nanoseconds per clock */

  clocks_per_ns = 1000 / cpufreq;

 

 

  printf("nanoseconds per clock %f\n", clocks_per_ns);

}

