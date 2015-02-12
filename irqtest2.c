
#include<sys/time.h>
#include<time.h>
#include<sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <sys/mman.h>
#include "poll.h"
#include "fcntl.h"
#include "string.h"
#include "errno.h"
#include <sys/timerfd.h>

#define TIMER_OUT_PATH 	"/sys/class/gpio/gpio47"
#define LED_OUT_PATH 	"/sys/class/gpio/gpio26"
#define IRQ_IN_PATH 	"/sys/class/gpio/gpio46"

static int timer_toggle = 0;
static int output_toggle = 0;
 
void stack_prefault(void)
{
	unsigned char dummy[8192];
	memset(dummy, 0, 8192);
}


int main(int argc, char* argv[])
{   
   struct itimerspec itv;
   unsigned long long timer_increment = 0;
     
   clock_t prevClock;
   int sigCnt = 0;
   struct sigaction sa;
   struct sched_param sp;
   
   struct pollfd fdset[2];
   int fd_in;
   int action;
   int fd_led;
   int fd_timer_out;      
   int fd_timer_in;
   
   char buf[2];
   int len;
 
   // setup gpio
   if(system("echo 46 > /sys/class/gpio/export") == -1)
		perror("unable to export gpio 46");
  
   if(system("echo 47 > /sys/class/gpio/export") == -1)
		perror("unable to export gpio 47");
		
   if(system("echo 26 > /sys/class/gpio/export") == -1)
		perror("unable to export gpio 26");
	
   // timer out
   if(system("echo out > /sys/class/gpio/gpio47/direction") == -1)
		perror("unable to set 47 to output");

	// led out
	if(system("echo out > /sys/class/gpio/gpio26/direction") == -1)
		perror("unable to set 26 to output");
	
	// irq in
	if(system("echo in > /sys/class/gpio/gpio46/direction") == -1)
		perror("unable to set 46 to input");
		
	if(system("echo both > /sys/class/gpio/gpio46/edge") == -1)
		perror("unable to set 46 edge");
	
	// set scheduling parameters
	sp.sched_priority = 49;//sched_get_priority_max(SCHED_FIFO);
	if(sched_setscheduler(0, SCHED_FIFO, &sp) == -1)
	{
		perror("setscheduler");
		exit(-1);
	}
		
	// lock memory
	if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1)
		perror("mlockall");
	
	stack_prefault();
	   
   // Set up timer
   #if USE_SIGNALS
   itv.it_value.tv_sec = 0;
   itv.it_value.tv_usec = 10000;
   itv.it_interval.tv_sec = 0;
   itv.it_interval.tv_usec = 10000;

   // start timer
   if(setitimer(ITIMER_REAL, &itv, 0) == -1)
		perror("setitmer");
	#else
	fd_timer_in = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	printf("fd_timer:%d\n",fd_timer_in);

	if(fd_timer_in < 0)
	{
		perror("timerfd_create()");
	}
	
   itv.it_value.tv_sec = 0;
   itv.it_value.tv_nsec = 10000000;
   itv.it_interval.tv_sec = 0;
   itv.it_interval.tv_nsec = 10000000;
   if(-1 == timerfd_settime(fd_timer_in, 0, &itv, NULL))
   {
	   perror("settime()");
   }
   #endif

   // setup file descriptor for poll()		
   fd_in = open(IRQ_IN_PATH "/value", O_RDONLY | O_NONBLOCK);
   printf("fd irq input:%d\n",fd_in);
   
   if(fd_in < 0)
   {
	   perror("file open problem");
	   exit(0);
   }

   while(1)
   {
		memset((void*)fdset, 0, sizeof(fdset));
		fdset[0].fd = fd_in;
		fdset[0].events = POLLPRI | POLLERR;
		fdset[0].revents = 0;
              
        fdset[1].fd = fd_timer_in;
		fdset[1].events = POLLIN | POLLERR;
		fdset[1].revents = 0;
		action = poll(fdset, 2, -1);
		
		if(action < 0)
		{
			if(errno == EINTR)
			{
				// when signal interrupts poll, we poll again
				continue;
			}
			else
			{
				perror("poll failed");
				exit(0);
			}
		}
		
		if(fdset[1].revents & POLLIN)
		{
			//len = read(fdset[1].fd, 0, SEEK_SET);
			len = read(fdset[1].fd, &timer_increment, sizeof(timer_increment));
			
			fd_timer_out = open( TIMER_OUT_PATH "/value", O_WRONLY);
     
		   if(timer_toggle ^= 1)
		   {
			   write(fd_timer_out, "1", 2);
		   }
		   else
		   {
			   write(fd_timer_out, "0", 2);
		   }
		   
		   close(fd_timer_out);
		}
	
		if(fdset[0].revents & POLLPRI)
		{
			lseek(fdset[0].fd, 0, SEEK_SET);	// read from start of file
			len = read(fdset[0].fd, buf, sizeof(buf));
			
			fd_led = open( LED_OUT_PATH "/value", O_WRONLY | O_NONBLOCK);
			
			if(buf[0] == '1')
			{
				write(fd_led, "1", 2);
			}
			else
			{
				write(fd_led, "0", 2);
			}
			
			close(fd_led);
		}
	}   
   
   close(fd_in);
   close(fd_timer_in);
   close(fd_led);
   printf("finished\n");
   return 0;
}
