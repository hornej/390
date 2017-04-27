#include <stdio.h>//standard input output
#include <stdlib.h>//get the standard library
#include "lockoutTimer.h"//import header file
#include "../interval_timers/intervalTimer.h"//import header file
#include <stdint.h> //standard int


#define START 0 //start of the lockout timer
#define TIMER_NUM 1 //we're using the 1 timer number

static bool timerRunning = false;//the timer is not initally running
static uint32_t lockoutCounter = START; //set the lockout counter to zero

enum hitLedTimer_st_t {
    running_st,//normal, running and detect hit
    lockout_st,// do nothing if hit
} lockoutCurrentState = running_st; //sets the current state to initialize

// Standard init function. Implement even if you don't find it necessary at present.
// Might be handy later.
void lockoutTimer_init(){
    timerRunning = false; //the timer is not initially running
    lockoutCounter = START; //set the lockoutcounter to zero
    lockoutCurrentState = running_st; //sets the current state to initialize
}

// Calling this starts the timer.
void lockoutTimer_start(){
    timerRunning = true; //the timer is now running
}

// Returns true if the timer is running.
bool lockoutTimer_running(){
    return timerRunning; //returns whether or not the timer is running
}
// Standard tick function.
void lockoutTimer_tick(){
    switch(lockoutCurrentState) {//go to where the current state is
    case running_st://start in running state
        break;//leave case statement
    case lockout_st://enter on state
        lockoutCounter++; //increment lockouttimer
        break;//leave case statement
    default:
        printf("lockoutTimer_tick state action: hit default\n\r"); //default state
        break;//leave case statement
    }
    switch(lockoutCurrentState) {//state transitions
    case running_st: //init state
        if (timerRunning){
            lockoutCurrentState = lockout_st;//go to the lockout State
        }
        break;//leave case statement
    case lockout_st://enter off state
        if(lockoutCounter >= LOCKOUT_TIMER_EXPIRE_VALUE){//if the timer is already on
            lockoutCounter = START;//restart the lockout timer
            timerRunning = false; //the timer is no longer running
            lockoutCurrentState = running_st; //go to on state
        }
        break;//leave case statement
    default:
        printf("lockoutTimer_tick state transition: hit default\n\r"); //default state
        break;//leave case statement
    }

}

//lockout timer test function
void lockoutTimer_runTest(){
    intervalTimer_init(TIMER_NUM);//initialize the interval timer
    intervalTimer_reset(TIMER_NUM); //make sure the timer is reset to zero
    intervalTimer_start(TIMER_NUM); //start the timer
    lockoutTimer_start();//start the lockout timer
    while(lockoutTimer_running());//wait until the timer is done running
    intervalTimer_stop(TIMER_NUM); //stop the timer
    printf("lockout duration: %f \n\r", intervalTimer_getTotalDurationInSeconds(TIMER_NUM));//print out the total lockout duration

}
