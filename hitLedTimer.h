#ifndef HITLEDTIMER_H_
#define HITLEDTIMER_H_

// The hitLedTimer illuminates the LEDs (LD0 and the LED attached to pin JF-3) for 1/2 second when activated.

#define HIT_LED_TIMER_EXPIRE_VALUE 100000	// Defined in terms of 100 kHz ticks.
#define HIT_LED_TIMER_OUTPUT_PIN 11 // JF-3

// Standard init function. Implement it even if it is not necessary. You may need it later.
void hitLedTimer_init();

// Calling this starts the timer.
void hitLedTimer_start();

// Returns true if the timer is currently running.
bool hitLedTimer_running();

// Standard tick function.
void hitLedTimer_tick();

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn();

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff();

void hitLedTimer_runTest();//run the LED Test

#endif /* HITLEDTIMER_H_ */
