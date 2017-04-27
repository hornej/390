#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "queue.h"
#include "filter.h"//for the filter
#include "lockoutTimer.h"//for the lockout timer
#include "isr.h"//to access adc queue
#include "hitLedTimer.h"//for the hit led timer
#include "../../supportFiles/interrupts.h"
#include "../switches_buttons_lab/switches.h"
#define NUMPLAYERS 10 //number of players in the array
#define INITIAL 0 //the initial value of the element count
#define HALF_WAY 2048.0 //half way the full possible value
#define HALF_WAY_INT 2048 //half way the full possible value
#define REFRESH 10 //the refresh threshold
#define INCREMENT 1//increments the player who hit me count
#define FUDGE_FACTOR 250 //a random fudge factor
#define PADDING 1 //padding to increment index location
#define MEDIAN 4 //median player index
#define NOHITVAL 20 //a number larger than the number of players
#define TRIVIAL 20 //non hit
#define NO_HIT 10 //no hit value
#define HIT 500 //a sumulated hit value
#define TEST_PLAYER 4 //the plauyer that we're hitting with
#define BIT_MASK 0xF //here is the bitmask for our switches read


static uint16_t myFrequencyIndex = NUMPLAYERS; //check for own frequency
typedef uint16_t detector_hitCount_t;
static uint32_t elementCount = INITIAL; //number of elements given by adc buffer
static uint32_t rawAdcValue = INITIAL; //the raw value from the adc buffer
static uint16_t addNewInputCount = INITIAL; //the raw value from the adc buffer
static double scaledAdcValue = INITIAL;//the scaledAdcValue coming from the buffer
static detector_hitCount_t myHitArray[NUMPLAYERS]; //which players have hit you
static bool firstRun = true;//keep track to see if we have to force compute the power
static bool testMode = false;//keep track to see if we have to force compute the power
static bool hitDetectedFlag = false; //indicates a hit has been detecteed
static uint16_t playerWhoHitMe = NUMPLAYERS; //the index of the player who hit us
static double powerValueHolder[NUMPLAYERS] = {INITIAL,INITIAL,INITIAL,INITIAL,INITIAL,INITIAL,INITIAL,INITIAL,INITIAL,INITIAL}; //holds the current power values

// Returns the current switch-setting
uint16_t detector_getFrequencySetting() {
    uint16_t switchSetting = switches_read() & BIT_MASK;  // Bit-mask the results.
    // Provide a nice default if the slide switches are in error.
    if (!(switchSetting < FILTER_FREQUENCY_COUNT))
        return FILTER_FREQUENCY_COUNT - PADDING;//return a nil value
    else
        return switchSetting;//return the switch setting
}

// Always have to init things.
void detector_init(){
    for(uint16_t i = INITIAL; i < NUMPLAYERS; i++){//set all of the initial player values
        myHitArray[i] = INITIAL;//reset the hit value to zero
    }
    filter_init();//initialize the filter
    lockoutTimer_init();// initalize the lockout timer
    hitLedTimer_init(); //initialize the hit led
    myFrequencyIndex = detector_getFrequencySetting(); //get my freuqency index
    //myFrequencyIndex = NOHITVAL;//ignore for the sake of his tests

}
//checks for a hit, returns the index of a player that hit you; returns an invalid index above nine if nobody
//has hit you
uint16_t detector_computeHitDetected(bool ignoreHitFromSelf){
    filter_getCurrentPowerValues(powerValueHolder);
    static double maxPower = INITIAL; //this will hold our max power value
    maxPower = INITIAL; //reset the power value to zero
    static uint16_t maxPowerIndex = INITIAL; //hold our max power index
    maxPowerIndex = NOHITVAL; //reset this to zero
    for (uint16_t i = INITIAL; i < NUMPLAYERS; i++){//start of insertion sort
        double x = powerValueHolder[i]; //hold a current value
        if(maxPower < powerValueHolder[i]){//see if max power is less than current val
            if(!ignoreHitFromSelf || i != myFrequencyIndex){//see if ignore hit from self is on
                maxPower = powerValueHolder[i];//reset the max power value
                maxPowerIndex = i; //reset the max power index
            }
        }
        int16_t j = i - PADDING; //take the index before that
        while (j >= INITIAL && powerValueHolder[j] > x){ //move down if it is less than our value
            powerValueHolder[j+PADDING] = powerValueHolder[j]; //move the power value
            j--;//move them down in the array
        }
        powerValueHolder[j+PADDING] = x;//place our placeholder value
    }
    double median = powerValueHolder[MEDIAN]; //get the median power value
    if(median*FUDGE_FACTOR < maxPower){ //see if we have a hit
        return maxPowerIndex;//return index of hit
    }
    else{
        return NOHITVAL; //return nil hit index because we didn't hit it
    }
}

// Runs the entire detector: decimating fir-filter, iir-filters, power-computation, hit-detection.
// if interruptsEnabled = false, interrupts are not running. If interruptsEnabled = true
// you can pop values from the ADC queue without disabling interrupts.
// If interruptsNotEnabled = false, do the following:
// 1. disable interrupts.
// 2. pop the value from the ADC queue.
// 3. re-enable interrupts.
// Use this to determine whether you should disable and re-enable interrrupts when accessing the adc queue.
// if ignoreSelf == true, ignore hits that are detected on your frequency.
// Your frequency is simply the frequency indicated by the slide switches.
void detector(bool interruptsEnabled, bool ignoreSelf){
    elementCount = isr_adcBufferElementCount();//get the element count
    if(testMode){ //check if in test mode
        elementCount = INCREMENT;//wanna run our loop only once
    }
    for(uint16_t i = INITIAL; i < elementCount; i++){//go through element count times
        if(interruptsEnabled){//check to see if the interrupts are enabled
            interrupts_disableArmInts();//disable before we pop an element off adcQueue
            rawAdcValue = isr_removeDataFromAdcBuffer(); //remove a value from the buffer
            interrupts_enableArmInts();//enable interrupts again
        }
        else{
            rawAdcValue = isr_removeDataFromAdcBuffer(); //remove a value from the buffer
        }
        if(testMode){ //see if we are in test mode
            rawAdcValue = INITIAL;//reset the raw adc value
        }

        if (rawAdcValue <= HALF_WAY_INT){ //see if the scaled value should be negative
            scaledAdcValue = -((HALF_WAY-rawAdcValue)/HALF_WAY); //set the scaled value
        }else{
            scaledAdcValue = (rawAdcValue-HALF_WAY)/HALF_WAY; //set the scaled value
        }
        filter_addNewInput(scaledAdcValue);//add to the fir filter
        addNewInputCount++; //increment the add new input count
        if(addNewInputCount >= REFRESH || testMode){//if 10 have been added, it is time to run the system
            addNewInputCount = INITIAL; //reset the input count
            filter_firFilter();//run the fir Filter
            if(!testMode){ //check to see if it is in test mode
                for(uint16_t i = INITIAL; i < NUMPLAYERS; i++){//go through element count times
                    filter_iirFilter(i); //run each iir filter
                    filter_computePower(i, firstRun, false); //compute the power for the given filter
                }
            }
            firstRun = false;//make sure that it is no longer the first run
            if(!lockoutTimer_running() || testMode){//check to see if we have recently been hit
                myFrequencyIndex = detector_getFrequencySetting(); //get my freuqency index
                playerWhoHitMe = detector_computeHitDetected(ignoreSelf);//see if someone hit us
                if(playerWhoHitMe < NUMPLAYERS){//check to see if we have detected a hit
                    hitLedTimer_start(); //start the LED timer
                    lockoutTimer_start(); //start the lockout timer
                    myHitArray[playerWhoHitMe] = myHitArray[playerWhoHitMe] + INCREMENT;//increments the number
                    //the player who hit me
                    hitDetectedFlag = true;//the hit has been detected
                }
            }

        }

    }

}

// Returns true if a hit was detected.
bool detector_hitDetected(){
    return hitDetectedFlag;//return the hit detected flag
}

// Clear the detected hit once you have accounted for it.
void detector_clearHit(){
    hitDetectedFlag = false;//reset the hit detected flag
}

// Get the current hit counts.
// Copy the current hit counts into the user-provided hitArray
// using a for-loop.
void detector_getHitCounts(detector_hitCount_t hitArray[]){
    for(uint16_t i = INITIAL; i < NUMPLAYERS; i++){//get all of the player values
        hitArray[i] = myHitArray[i];//copy them to the hit array
    }
}


void detector_runTest(){//test to see if our detector system works
    printf("Filling the current power array with a simulated hit\n\r"); //message to the user
    static double powerVals[NUMPLAYERS] = {NO_HIT,NO_HIT,NO_HIT,NO_HIT,HIT,NO_HIT,NO_HIT,NO_HIT,NO_HIT,NO_HIT};//initialize current power values
    testMode = true;//we are running in test mode
    filter_setCurrentPowerValues(powerVals); //set the current power values
    printf("Running the detector with simulated hit\n\r"); //message the user
    detector(true,true); //ignore a hit from self, assuming interrupts are enabled
    printf("Hit Detected Should be 1 (or true): %d\n\r", detector_hitDetected());//see if a hit has been detected
    detector_clearHit();//clear the hit from our queue
    printf("Filling the current power array with no hit\n\r");//message to user
    powerVals[TEST_PLAYER] = NO_HIT;//reset the power values to not be a hit
    filter_setCurrentPowerValues(powerVals);//reset the power values
    printf("Running the detector without hit\n\r"); //message to the user
    detector(true,false); //dont ignore a hit from self, assuming interrupts are enabled
    printf("Hit Detected Should be 0 (or false): %d\n\r", detector_hitDetected());//see if a hit has been detected
}

