#include<unistd.h>
#include<sys/time.h>
#include<time.h>
#include<sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <sys/mman.h>
#include "iolib.h"

static volatile sig_atomic_t gotAlarm = 0;
static int toggle = 0;
 
static void sigalrmHandler( int sig )
{
   if(toggle ^= 1)
   {
	   pin_high(8, 12);
   }
   else
   {
	   pin_low(8, 12);
   }

}
 
int main(int argc, char* argv[]){
   
   struct itimerval itv;
   clock_t prevClock;
   int sigCnt = 0;
   struct sigaction sa;
   struct sched_param sp;
   
   iolib_init(); 
   iolib_setdir(8, 12, DIR_OUT);
 
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

   while(sigCnt < 30000)
   {
	   sleep(1);
	   sigCnt++;
   }   
   
   iolib_free();   
   return 0;
}
