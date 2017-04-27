#ifndef RUNNINGMODES_H_
#define RUNNINGMODES_H_

#include <stdint.h>

// This mode runs continously until btn3 is pressed.
// When btn3 is pressed, it exits and prints performance information to the TFT.
// During operation, it continously displays that received power on each channel, on the TFT.
// Frequency is selected via the slide-switches.
void runningModes_continuous();

// This mode runs continously until btn3 is pressed.
// When btn3 is pressed, it exits and prints performance information to the TFT.
// Game-playing mode. Each shot is registered on the histogram on the TFT.
// Press BTN0 or the gun-trigger to shoot.
// Frequency is selected via the slide-switches.
void runningModes_shooter();

// Continuously cycles through all channels, shooting one pulse per channel.
void runningModes_testShootAllChannels();

// Returns the current switch-setting
uint16_t runningModes_getFrequencySetting();

#endif /* RUNNINGMODES_H_ */
