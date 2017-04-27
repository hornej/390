/*
 * transmitter.h
 *
 *  Created on: Feb 23, 2017
 *      Author: hornej2
 */

#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

#define TRANSMITTER_OUTPUT_PIN 13           // JF1 (pg. 25 of ZYBO reference manual).
#define TRANSMITTER_PULSE_WIDTH 20000   // Based on a system tick-rate of 100 kHz.
#include <stdint.h>

// The transmitter state machine generates a square wave output at the chosen frequency
// as set by transmitter_setFrequencyNumber(). The step counts for the frequencies
// are provided in filter.h

// Standard init function.
void transmitter_init();

// Starts the transmitter.
void transmitter_run();

// Returns true if the transmitter is still running.
bool transmitter_running();

// Sets the frequency number. If this function is called while the
// transmitter is running, the frequency will not be updated until the
// transmitter stops and transmitter_run() is called again.
void transmitter_setFrequencyNumber(uint16_t frequencyNumber);

// Standard tick function.
void transmitter_tick();

// Tests the transmitter.
void transmitter_runTest();

void runTest_Continuous();

// Runs the transmitter continuously.
// if continuousModeFlag == true, transmitter runs continuously, otherwise, transmits one pulse-width and stops.
// To set continuous mode, you must invoke this function prior to calling transmitter_run().
// If the transmitter is in currently in continuous mode, it will stop running if this function is
// invoked with continuousModeFlag == false. It can stop immediately or wait until the last 200 ms pulse is complete.
// NOTE: while running continuously, the transmitter will change frequencies at the end of each 200 ms pulse.
void transmitter_setContinuousMode(bool continuousModeFlag);

// This is provided for testing as explained in the transmitter section of the web-page. When enabled,
// debug prints are enabled to help to demonstrate the behavior of the transmitter.
void transmitter_enableTestMode();

// This is provided for testing as explained in the transmitter section of the web-page. When disabled,
// debug prints that were previously enabled no longer appear.
void transmitter_disableTestMode();

#endif /* TRANSMITTER_H_ */
