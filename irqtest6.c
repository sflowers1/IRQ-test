
//#include<iostream>
//#include<fstream>
//#include<string>
//#include<unistd.h>
#include<sys/time.h>
#include<time.h>
#include<sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <sys/mman.h>
//#include "./iolib.h"
#include "poll.h"
#include "fcntl.h"
#include "string.h"
#include "errno.h"
//using namespace std;

#define TIMER_OUT_PATH 	"/sys/class/gpio/gpio47"
#define LED_OUT_PATH 	"/sys/class/gpio/gpio26"
#define IRQ_IN_PATH 	"/sys/class/gpio/gpio46"

static int timer_toggle = 0;
static int output_toggle = 0;
//static int fd_timer;
 
void stack_prefault(void)
{
	unsigned char dummy[8196];
	memset(dummy, 0, 8196);
}

 
int main(int argc, char* argv[])
{   
   struct itimerval itv;
   clock_t prevClock;
   int sigCnt = 0;
   struct sigaction sa;
   struct sched_param sp;
   
   struct pollfd fdset[1];
   int fd, action;
   int fd_led;
   
   char buf[2];
   int len;
   
//   cout << "Starting the LED2 program" << endl;
  // cout << "The LED Path is: " << LED0_PATH << endl;   
   
   //iolib_init(); 
   //iolib_setdir(8, 15, DIR_OUT);	// timer output
   //iolib_setdir(8, 16, DIR_IN);		// input 
   //iolib_setdir(8, 14, DIR_OUT);	// toggle output
 
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
	   
    // setup file descriptor for poll()		
   fd = open(IRQ_IN_PATH "/value", O_RDONLY | O_NONBLOCK);
   fd_led = open( LED_OUT_PATH "/value", O_WRONLY | O_NONBLOCK);
   
   if(fd < 0)
   {
	   perror("file open problem");
	   exit(0);
   }

	printf("starting while loop..\n");
   while(1)
   {
		memset((void*)fdset, 0, sizeof(fdset));
		fdset[0].fd = fd;
		fdset[0].events = POLLPRI | POLLERR;
		fdset[0].revents = 0;
		action = poll(fdset, 1, -1);
              
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
	
		if(fdset[0].revents & POLLPRI)
		{
			lseek(fdset[0].fd, 0, SEEK_SET);	// read from start of file
			len = read(fdset[0].fd, buf, sizeof(buf));
			
			lseek(fd_led, 0, SEEK_SET);			
			
			if(buf[0] == '1')
			{
				write(fd_led, "1", 2);
				//pin_high(8, 14);
				//printf("led on\n");
			}
			else
			{
				write(fd_led, "0", 2);
				//pin_low(8, 14);
				//printf("led off\n");
			}
			
			//close(fd_led);
		}
	}   
   
   close(fd);
   //close(fd_timer);
   close(fd_led);
   //iolib_free();   
   printf("finished\n");
   return 0;
}
