/*
 * transmitter.c
 *
 *  Created on: Feb 23, 2017
 *      Author: hornej2
 */

#include <stdint.h>
#include <stdio.h> //import standard input output
#include <stdlib.h> //import standard library
#include <string.h> //include for string library
#include <stdbool.h> //include to do boolean
#include "transmitter.h" //include .h for transmitter
#include "supportFiles/mio.h" //mio used to access probe
#include "filter.h" //uses filter freq tick table
#include "../switches_buttons_lab/switches.h" //include switch functionality
#include "../interval_timers/buttons.h" //include button functionality
#include "supportFiles/utils.h" //include for delay functionality

#define PLAYER_1 0 //player 1 array address
#define PLAYER_2 1 //player 2 array address
#define PLAYER_3 2 //player 3 array address
#define PLAYER_4 3 //player 4 array address
#define PLAYER_5 4 //player 5 array address
#define PLAYER_6 5 //player 6 array address
#define PLAYER_7 6 //player 7 array address
#define PLAYER_8 7 //player 8 array address
#define PLAYER_9 8 //player 9 array address
#define PLAYER_10 9 //player 10 array address
#define TRANSMITTER_OUTPUT_PIN 13 //output to transmitter probe
#define TRANSMITTER_HIGH_VALUE 1 //output 1's to probe
#define TRANSMITTER_LOW_VALUE 0 //output 0's to probe
#define PULSE_LENGTH 20000 //200ms
#define FIFTY_PERCENT_DUTY_CYCLE 1/2 //square wave uses 50% duty cycle
#define RESET 0 //reset to 0
#define TRANSMITTER_WAIT_IN_MS 300 //time to wait between pulses

static uint32_t frequency_number = 0; //decided the freq of player number
static bool startFlag; //flag that will allow for use of the transmitter tick
static bool runContinuous = false; //flag to choose continuous/non-continuous mode
uint32_t pulse_cnt = 0; //keeps the pulse length 200ms
uint32_t freq_cnt = 0; //initialie freq cnt to 0

// Standard init function.
void transmitter_init(){
    mio_init(false);  // false disables any debug printing if there is a system failure during init.
    mio_setPinAsOutput(TRANSMITTER_OUTPUT_PIN);  // Configure the signal direction of the pin to be an output.

}

void transmitter_set_jf1_to_one() {
    mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_HIGH_VALUE); // Write a '1' to JF-1.
}

void transmitter_set_jf1_to_zero() {
    mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW_VALUE); // Write a '1' to JF-1.
}

// Starts the transmitter.
void transmitter_run(){
    startFlag = true;
}

// Sets the frequency number. If this function is called while the
// transmitter is running, the frequency will not be updated until the
// transmitter stops and transmitter_run() is called again.
void transmitter_setFrequencyNumber(uint16_t frequencyNumber){
    frequency_number = filter_frequencyTickTable[frequencyNumber];
}

// States for the controller state machine.
enum clockControl_st_t {
    init_st, // Start here, stay in this state for just one tick.
    wait_for_startFlag_st, //wait here till the start flag is raised
    low_st, //outputs the low signal 0
    high_st //outputs the high signal 1
} current_State = init_st; //start in init_st

static clockControl_st_t previousState; //variable to keep track of prev state
static bool firstPass = true; //initialize firstpass to true

void debugStatePrint() {
    // Only print the message if:
    // 1. This the first pass and the value for previousState is unknown.
    // 2. previousState != current_State - this prevents reprinting the same state name over and over.
    if (previousState != current_State || firstPass) {
        firstPass = false;                // previousState will be defined, firstPass is false.
        previousState = current_State;     // keep track of the last state that you were in.
        switch(current_State) {            // This prints messages based upon the state that you were in.
        case init_st:
            printf("init_st\n\r"); //print init_st to console
            break;
        case wait_for_startFlag_st:
            printf("wait_for_startFlag_st\n\r"); //print wait_for_startFlag_st to console
            break;
        case low_st:
            printf("low_st\n\r"); //print low_st to console
            break;
        case high_st:
            printf("high_st\n\r"); //print high_st to console
            break;
        }
    }
}

// Returns true if the transmitter is still running.
bool transmitter_running(){
    return startFlag; //return whether or not the transmitter is running
}

// Standard tick function.
void transmitter_tick(){
    // Perform state action first
    switch(current_State) {
    case init_st:
        transmitter_set_jf1_to_zero(); //start the signal at 0
        break;
    case wait_for_startFlag_st: //do nothing here
        break;
    case low_st:
        pulse_cnt++; //increase the pulse count
        freq_cnt++; //increase the frequency count
        break;
    case high_st:
        pulse_cnt++; //increase the pulse count
        freq_cnt++; //increase the frequency count
        break;
    default:
        printf("transmitter_tick state update: hit default\n\r");
        break;
    }

    // Perform state update next
    switch(current_State) {
    case init_st:
        current_State = wait_for_startFlag_st; //go immediately to wait_for_startFlag_st
        break;
    case wait_for_startFlag_st:
        if (startFlag){ //if the startFlag is high
            current_State = low_st; //then start in the low_st
        }
        else {
            transmitter_set_jf1_to_zero(); //start the signal at 0
            current_State = wait_for_startFlag_st; //if the flag is down than wait here
        }
        break;
    case low_st:
        if (pulse_cnt < PULSE_LENGTH){ //go into this until the pulse is over
            if(freq_cnt >= frequency_number*FIFTY_PERCENT_DUTY_CYCLE){ //go high for half the freq
                transmitter_set_jf1_to_one(); //output 1's
                freq_cnt = RESET; //reset freq_cnt
                current_State = high_st; //go to high_st
            }
            else { //stay in low until freq_cnt reaches half freq number
                current_State = low_st; //stay in low_st
            }
        }
        else { //if the pulse count has reached 200ms
            pulse_cnt = RESET; //reset pulse_cnt
            freq_cnt = RESET; //reset freq_cnt
            if(!runContinuous){ //unless in runContinuous mode
                startFlag = false; //lower startFlag
                transmitter_set_jf1_to_zero(); //start the signal at 0
                current_State = wait_for_startFlag_st; //go to wait_for_startFlag_st
            }
        }
        break;
    case high_st:
        if (pulse_cnt < PULSE_LENGTH){ //go into this until the pulse is over
            if(freq_cnt >= frequency_number*FIFTY_PERCENT_DUTY_CYCLE){ //go high for half the freq
                transmitter_set_jf1_to_zero(); //output 0's
                freq_cnt = RESET; //reset freq_cnt
                current_State = low_st; //go to low_st
            }
            else { //stay in high until freq_cnt reaches half freq number
                current_State = high_st; //go to high_st
            }
        }
        else { //if the pulse count has reached 200ms
            pulse_cnt = RESET; //reset pulse_cnt
            freq_cnt = RESET; //reset freq_cnt
            if(!runContinuous){ //unless in runContinuous mode
                startFlag = false; //lower startFlag
                transmitter_set_jf1_to_zero(); //start the signal at 0
                current_State = wait_for_startFlag_st; //go to wait_for_startFlag_st
            }
        }
        break;
    default:
        printf("transmitter_tick state action: hit default\n\r");
        break;
    }
}

void transmitter_runTest() {
    //transmitter_setContinuousMode(true); //uncomment this to use continuous mode
    buttons_init(); // Using buttons
    switches_init(); // and switches.
    transmitter_init(); // init the transmitter.
    if (!runContinuous){ //if not in continuous mode
        while (!(buttons_read() & BUTTONS_BTN1_MASK)) { // Run continuously until btn1 is pressed.
            uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;  // Compute a safe number from the switches.
            transmitter_setFrequencyNumber(switchValue); // set the frequency number based upon switch value.
            transmitter_run(); // Start the transmitter.
            while(transmitter_running()){} //loop while the transmitter is running
            utils_msDelay(TRANSMITTER_WAIT_IN_MS); //off for 300ms
        }
    }
    else { //if in continuous mode
        transmitter_run(); //run the transmitter
        while(1){ //infinite loop
            uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT; //read the switches
            transmitter_setFrequencyNumber(switchValue); //transmit the desired freq
        }
    }
}

// Runs the transmitter continuously.
// if continuousModeFlag == true, transmitter runs continuously, otherwise, transmits one pulse-width and stops.
// To set continuous mode, you must invoke this function prior to calling transmitter_run().
// If the transmitter is in currently in continuous mode, it will stop running if this function is
// invoked with continuousModeFlag == false. It can stop immediately or wait until the last 200 ms pulse is complete.
// NOTE: while running continuously, the transmitter will change frequencies at the end of each 200 ms pulse.
void transmitter_setContinuousMode(bool continuousModeFlag){
    runContinuous = continuousModeFlag;//set the continuous value
}


