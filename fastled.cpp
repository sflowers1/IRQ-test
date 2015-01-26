/*
 
    This program uses USR LED 0 and can be executed in three ways:
         makeLED on
         makeLED off
         makeLED flash  (flash at 100ms intervals - on 50ms/off 50ms)
         makeLED status (get the trigger status)
*/
 
#include<iostream>
#include<fstream>
#include<string>
#include<unistd.h>
#include<sys/time.h>
#include<time.h>
#include<sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <sys/mman.h>
#include "./iolib.h"
//using namespace std;

#define LED0_PATH "/sys/class/gpio/gpio47"

static volatile sig_atomic_t gotAlarm = 0;
//static fstream fs; 
static int toggle = 0;
 
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
    
   if(toggle ^= 1)
   {
	   pin_high(1, 15);
   }
   else
   {
	   pin_low(1, 15);
   }

}
 
int main(int argc, char* argv[]){
   
   struct itimerval itv;
   clock_t prevClock;
   int sigCnt = 0;
   struct sigaction sa;
   struct sched_param sp;
   
//   cout << "Starting the LED2 program" << endl;
  // cout << "The LED Path is: " << LED0_PATH << endl;   
   
   iolib_init(); 
   iolib_setdir(1, 15, DIR_OUT);
 
   // setup gpio
   //if(system("echo 53 > /sys/class/gpio/export") == -1)
	//perror("unable to export gio 53");
	
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

   while(sigCnt < 100000)
   {
	   sleep(1);
	   /*if(gotAlarm)
	   {
		   //cout << "received a signal" << endl;
			gotAlarm = 0;
			sigCnt++;
		   
		   if(toggle ^= 1)
		   {
			   //cout << "LED on" << endl;
			   fs.open (LED0_PATH "/value", std::fstream::out);
			   fs << "1";
			   fs.close();	   
		   }
		   else
		   {
			   //cout << "LED off" << endl;
			   fs.open (LED0_PATH "/value", std::fstream::out);
			   fs << "0";
			   fs.close();
		   }
	   }*/
   }   
   
   iolib_free();   
   //cout << "Finished the LED flash program" << endl;
   return 0;
}
