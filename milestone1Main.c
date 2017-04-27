/*
 * milestone1Main.c
 *
 *  Created on: Jan 10, 2017
 *      Author: msj02
 */
#include "queue.h"//call our queue
#include "filter.h"//call our filter functions
#include <stdio.h> //in case we are writing to output
#include "filterTest.h" //in case we need to check the filter
#include "hitLedTimer.h"//hit led timer test
#include "lockoutTimer.h" //for lockout timer
#include "trigger.h" //for trigger run test
#include "supportFiles/utils.h" //in case we want delay
#include "supportFiles/interrupts.h" //for the isr function
#include "detector.h"
#include "runningModes.h"//running modes
#include "../interval_timers/buttons.h" //get the buttons
#include <assert.h>

#define BUTTONS_BTN2_MASK 0x4   // Bit mask for BTN2

// The program comes up in continuous mode.
// Hold BTN2 while the program starts to come up in shooter mode.
int main() {
    buttons_init();  // Init the buttons.
    if (buttons_read() & BUTTONS_BTN2_MASK) // Read the buttons to see if BTN2 is drepressed.
        runningModes_shooter();               // Run shooter mode if BTN2 is depressed.
    else
        runningModes_continuous();            // Otherwise, go to continuous mode.
}

