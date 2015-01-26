/*
 
    This program uses USR LED 0 and can be executed in three ways:
         makeLED on
         makeLED off
         makeLED flash  (flash at 100ms intervals - on 50ms/off 50ms)
         makeLED status (get the trigger status)
*/
 
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
#include "./iolib.h"
#include "poll.h"
#include "fcntl.h"
#include "string.h"
#include "errno.h"
//using namespace std;

#define LED0_PATH "/sys/class/gpio/gpio47"

static volatile sig_atomic_t gotAlarm = 0;
//static fstream fs; 
static int timer_toggle = 0;
static int output_toggle = 0;
 
void removeTrigger(){
   // remove the trigger from the LED
   //std::fstream fs;
   //fs.open( LED0_PATH "/trigger", std::fstream::out);
   //fs << "none";
   //fs.close();
}

static void sigalrmHandler( int sig )
{
    // obligator caution not to use printf and other async-unsafe calls
    // in a handler in real programs
	gotAlarm = 1;
    //printf("I am timer\n");
    
   if(timer_toggle ^= 1)
   {
	   pin_high(8, 15);
   }
   else
   {
	   pin_low(8, 15);
   }

}
 
int main(int argc, char* argv[]){
   
   struct itimerval itv;
   clock_t prevClock;
   int sigCnt = 0;
   struct sigaction sa;
   struct sched_param sp;
   
   struct pollfd fdset[1];
   int fd, action;
   
   char buf[10];
   int len;
   
//   cout << "Starting the LED2 program" << endl;
  // cout << "The LED Path is: " << LED0_PATH << endl;   
   
   iolib_init(); 
   iolib_setdir(8, 15, DIR_OUT);	// timer output
   //iolib_setdir(8, 16, DIR_IN);		// input 
   iolib_setdir(8, 14, DIR_OUT);	// toggle output
 
   // setup gpio
   //if(system("echo 53 > /sys/class/gpio/export") == -1)
	//perror("unable to export gpio 53");
	
  //if(system("echo out > /sys/class/gpio/gpio53/direction") == -1)
//	perror("unable to set pin to output");

	// set scheduling parameters
	sp.sched_priority = 49;
	if(sched_setscheduler(0, SCHED_FIFO, &sp) == -1)
	{
		perror("setscheduler");
		exit(-1);
	}
		
	// lock memory
	if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1)
		perror("mlockall");
		
   
   // Set up signal handler.
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = 0;
   sa.sa_handler = sigalrmHandler;
   if(sigaction(SIGALRM, &sa, NULL) == -1)
		perror("sigaction");

   // Set up timer
   itv.it_value.tv_sec = 0;
   itv.it_value.tv_usec = 10000;
   itv.it_interval.tv_sec = 0;
   itv.it_interval.tv_usec = 10000;

   // start timer
   if(setitimer(ITIMER_REAL, &itv, 0) == -1)
		perror("setitmer");
	
   // setup file descriptor for poll()		
   fd = open("/sys/class/gpio/gpio46/value", O_RDONLY | O_NONBLOCK);
   
   if(fd < 0)
   {
	   perror("file open problem");
	   exit(0);
   }

	printf("starting while loop..\n");
   while(sigCnt < 100000)
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
				// in case signal interrupts poll
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
			len = read(fdset[0].fd, buf, 10);
			//printf("revents %d, buffer: %s\n", fdset[0].revents, buf);
			
			if(buf[0] == '1')
			{
				pin_high(8, 14);
				//printf("pin_high\n");
			}
			else
			{
				pin_low(8, 14);
				//printf("pin_low");
			}
		}
	}   
   
   close(fd);
   iolib_free();   
   printf("finished\n");
   return 0;
}
