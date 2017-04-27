#include <stdio.h>//standard input output
#include <stdlib.h>//get the standard library
#include "hitLedTimer.h"//import header file
#include "lockoutTimer.h"//import header file
#include "supportFiles/leds.h"//import header file
#include "supportFiles/mio.h"//import header file
#include "supportFiles/utils.h" //for the wait in the run test


#define TURN_LD0_ON 0x01 //turns on the Led0
#define TURN_LD0_OFF 0x00 //turns off the Led0
#define JF3_PIN 11 //the corresponding MIO value for JF3
#define TRANSMITTER_HIGH_VALUE 1 //high value for an output pin
#define TRANSMITTER_LOW_VALUE 0//low value for an output pin
#define HAS_RUN_TRUE 1 //see if already written to the output pin
#define START 0 //set it to the start value
#define HALF_SECOND .5*HIT_LED_TIMER_EXPIRE_VALUE //what a half second looks like
#define THREE_HUNDRED_MS 300 //300ms delay


static bool timerIsOn = false; //timer is not on to begin;
static uint8_t hasRun = START; //see if we've already written to the output pin
static uint32_t onCounter = START; //initialize the on counter to zero
// Standard init function.
void hitLedTimer_init(){
    timerIsOn = false;//intialize the timer to off
    lockoutTimer_init(); //make sure the lockout timer is initialized
    leds_init(false); //init the leds
    mio_init(false); //initialize the mio
    mio_setPinAsOutput(JF3_PIN);  // Configure the signal direction of the pin to be an output.
}

enum hitLedTimer_st_t {
    init_st,// Start here, stay in this state for just one tick.
    off_st,//stays here until it is asked to turn on
    on_st,// turns on leds
} currentState = init_st; //sets the current state to initialize

// Calling this starts the timer.
void hitLedTimer_start(){
    if (lockoutTimer_running()){//if the lockout timer is running, do nothing
    }else{
        timerIsOn = true; //set this to true in order to allow the timer to turn off
    }
}

// Returns true if the timer is currently running.
bool hitLedTimer_running(){
    return timerIsOn; //return whether or not the timer is running
}

// Standard tick function.
void hitLedTimer_tick(){
    switch(currentState) {//go to where the current state is
    case init_st: //init state
        hasRun = START; //see if we've already written to the output pin
        onCounter = START; //initialize the on counter to zero
        break;//leave case statement
    case off_st://enter off state
        if(hasRun >= HAS_RUN_TRUE){//see if we've already written to the output pin
        }else{
            hasRun++; //increment the hasRun variable
            hitLedTimer_turnLedOff(); //turn off the Led
        }
        break;//leave case statement
    case on_st://enter on state
        if(hasRun >= HAS_RUN_TRUE){//see if we've already written to the output pin
            onCounter++; //increment the oncounter
        }else{
            hasRun++; //increment the hasRun variable
            hitLedTimer_turnLedOn(); //turn on the Led
            onCounter++; //increment the oncounter
        }
        break;//leave case statement
    default:
        printf("hitLedTimer_tick state action: hit default\n\r"); //default state
        break;//leave case statement
    }
    switch(currentState) {//state transitions
    case init_st: //init state
        currentState = off_st; //wait in the off state
        break;//leave case statement
    case off_st://enter off state
        if(timerIsOn){//if the timer is already on
            hasRun = START;//reset the hasRunValue
            currentState = on_st; //go to on state
        }
        break;//leave case statement
    case on_st://enter on state
        if(onCounter >= HALF_SECOND){//if the timer has gone for sufficient time
            hasRun = START;//reset the hasRunValue
            timerIsOn = false;//the timer is no longer on
            onCounter = START; //reset the onCounter
            currentState = off_st; //go to on state
        }
        break;//leave case statement
    default:
        printf("hitLedTimer_tick state transition: hit default\n\r"); //default state
        break;//leave case statement
    }

}

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn(){
    leds_write(TURN_LD0_ON);//turns on the Led ld0
    mio_writePin(JF3_PIN, TRANSMITTER_HIGH_VALUE); // Write a '1' to JF-3.
}

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff(){
    leds_write(TURN_LD0_OFF);//turns off the Led ld0
    mio_writePin(JF3_PIN, TRANSMITTER_LOW_VALUE); // Write a '1' to JF-3.
}

void hitLedTimer_runTest(){//the run test for the hitLedTimer
    while(true){//do this forever
        hitLedTimer_start();//start the led timer
        while(hitLedTimer_running()); //wait until it is done running
        utils_msDelay(THREE_HUNDRED_MS); //delay three hundred milliseconds
    }

}
