/*
 * trigger.c
 *
 *  Created on: Feb 23, 2017
 *      Author: msj02
 */
#include <stdio.h>//standard input output
#include <stdlib.h>//get the standard library
#include "trigger.h"//import header file
#include "supportFiles/mio.h"//import header file
#include "supportFiles/utils.h" //for the wait in the run test
#include "../interval_timers/buttons.h" //reading the buttons
#include "transmitter.h" //for the transmitter run

#define TRIGGER_GUN_TRIGGER_MIO_PIN 10 //jf pin 2
#define BUTTONS_BTN0_MASK 0x1 //to see if button zero is pressed
#define GUN_TRIGGER_PRESSED 1 //value if the trigger is being pressed
#define SETTLE_DELAY 5000 //the time for debouncing
#define START 0 //initialize to zero

static bool ignoreGunInput = false; //determining whether or not we can ignore the gun input
static bool triggerEnabled = false;//the trigger is not initially enabled
static uint32_t delayCounter = START; //reset the delay counter
static bool firstRun = true; //see if this is the first run through the transmission send
static bool debugPrint = false; //printing for testing

enum trigger_st_t {
    waiting_for_touch_st,//waiting for trigger to be pressed
    pressed_debounce_st,// deboouncing period
    trigger_pressed_st,// the trigger was actually pressed
    released_debounce_st,// deboouncing period
} triggerState = waiting_for_touch_st; //sets the current state to initialize


//prints out when a state changes
void trigger_stateDebugPrint() {
    //print the current state (for debugging purposes)
    static trigger_st_t previousState; //saves the previous state to know whether or not to switch
    static bool firstPass = true; //set a variable to see if it is the first pass
    // Only print the message if:
    // 1. This the first pass and the value for previousState is unknown.
    // 2. previousState != currentState - this prevents reprinting the same state name over and over.
    if (previousState != triggerState || firstPass) {
        firstPass = false;                // previousState will be defined, firstPass is false.
        previousState = triggerState;     // keep track of the last state that you were in.
        switch(triggerState) { //print out according to which state you are in
        case waiting_for_touch_st:
            printf("waiting_for_touch_st\n\r"); //print current state
            break; //leave case statement
        case pressed_debounce_st:
            printf("pressed_debounce_st\n\r");//print current state
            break;//leave case statement
        case trigger_pressed_st:
            printf("trigger_pressed_st\n\r");//print current state
            break;//leave case statement
        case released_debounce_st:
            printf("released_debounce_st\n\r");//print current state
            break;//leave case statement
        default:
            printf("trigger_tick state debug print: hit default\n\r");//print current state
            break;//leave case statement
        }
    }
}


// Trigger can be activated by either btn0 or the external gun that is attached to TRIGGER_GUN_TRIGGER_MIO_PIN
// Gun input is ignored if the gun-input is high when the init() function is invoked.
bool triggerPressed() {
    return ((!ignoreGunInput & (mio_readPin(TRIGGER_GUN_TRIGGER_MIO_PIN) == GUN_TRIGGER_PRESSED)) ||
            (buttons_read() & BUTTONS_BTN0_MASK));//returns whether or not the trigger has been pressed
}
// Init trigger data-structures.
// Determines whether the trigger switch of the gun is connected (see discussion in lab web pages).
// Initializes the mio subsystem.
// Configure the trigger-pin as an input.
// Ignore the gun if the trigger is high at init (means that it is not connected).
void trigger_init() {
    mio_setPinAsInput(TRIGGER_GUN_TRIGGER_MIO_PIN);
    // If the trigger is pressed when trigger_init() is called, assume that the gun is not connected and ignore it.
    if (triggerPressed()) {
        ignoreGunInput = true;//see whether or not the gun is plugged in
    }
}


// Enable the trigger state machine. The trigger state-machine is inactive until this function is called.
// This allows you to ignore the trigger when helpful (mostly useful for testing).
// I don't have an associated trigger_disable() function because I don't need to disable the trigger.
void trigger_enable(){
    triggerEnabled = true;//we can now run our state machine
}

// Standard tick function.
void trigger_tick(){
    if(debugPrint){//print if necessary
        trigger_stateDebugPrint(); //print for debugging
    }
    switch(triggerState) {//go to where the current state is
    case waiting_for_touch_st: //enter state
        break;//leave case statement
    case pressed_debounce_st: //enter state
        delayCounter++;//waiting for the debounce
        break;//leave case statement
    case trigger_pressed_st: //enter state
        if(firstRun){
            transmitter_run();//run the transmitter //UNCOMMENT HERE!
            firstRun = false;//don't send the transmission twice
            if(debugPrint){//for debugging
                printf("D\n");//successfully debonced
            }
        }
        break;//leave case statement
    case released_debounce_st: //enter state
        delayCounter++;//waiting for the debounce
        break;//leave case statement
    default:
        printf("trigger_tick state action: hit default\n\r"); //default state
        break;//leave case statement
    }

    switch(triggerState) {//go to where the current state is
    case waiting_for_touch_st: //enter state
        if (!triggerEnabled){ //see if the trigger has been enabled

        }else{
            if(triggerPressed()){//see if the trigger has been touched
                triggerState = pressed_debounce_st;//set the new trigger state
            }
        }
        break;//leave case statement
    case pressed_debounce_st: //enter state
        if(delayCounter >= SETTLE_DELAY && triggerPressed()){//see if settling has taken place and user is still touching button
            delayCounter = START; //reset delay counter
            triggerState = trigger_pressed_st; //go to the next state
        }else if(delayCounter >= SETTLE_DELAY){//see if settling has taken place
            delayCounter = START; //reset delay counter
            triggerState = waiting_for_touch_st; //go to the next state
        }
        break;//leave case statement
    case trigger_pressed_st: //enter state
        if(!triggerPressed()){ //see if the trigger has stopped being pressed
            triggerState = released_debounce_st; //go to the next state
        }
        break;//leave case statement
    case released_debounce_st: //enter state
        if(delayCounter >= SETTLE_DELAY && !triggerPressed()){//see if settling has taken place and user is still touching button
            delayCounter = START; //reset delay counter
            firstRun = true;//can now send a new transmission
            if(debugPrint){//for debugging
                printf("U\n");//successfully debonced
            }
            triggerState = waiting_for_touch_st; //go to the next state
        }else if(delayCounter >= SETTLE_DELAY){//see if settling has taken place
            delayCounter = START; //reset delay counter
            triggerState = trigger_pressed_st; //go to the next state
        }

        break;//leave case statement
    default:
        printf("trigger_tick state transition: hit default\n\r"); //default state
        break;//leave case statement
    }
}

void trigger_runTest(){
    debugPrint = true; //printing for testing
}
