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
using namespace std;
 
#define LED0_PATH "/sys/class/gpio/gpio53"
 
void removeTrigger(){
   // remove the trigger from the LED
   std::fstream fs;
   fs.open( LED0_PATH "/trigger", std::fstream::out);
   fs << "none";
   fs.close();
}
 
int main(int argc, char* argv[]){
   
   std::fstream fs;
   cout << "Starting the LED flash program" << endl;
   cout << "The LED Path is: " << LED0_PATH << endl;    
 
   for(int i = 0; i < 10; i++)
   {
	   cout << "LED on loop " << i << endl;
	   
	   fs.open (LED0_PATH "/value", std::fstream::out);
	   fs << "1";
	   fs.close();	   
	   usleep(500000);
	   
	   cout << "LED off loop " << i << endl;
	   
	   fs.open (LED0_PATH "/value", std::fstream::out);
	   fs << "0";
	   fs.close();
	   usleep(500000);
   }
   
   cout << "Finished the LED flash program" << endl;
   return 0;
}
