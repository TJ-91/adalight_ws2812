Adalight WS2812
===============

This is a fork of Wifsimster/adalight_ws2812 and adds smooth color transitions.

The following libraries are used:
* FastLED https://github.com/FastLED/FastLED
* CircularBuffer https://github.com/rlogiacco/CircularBuffer


Notes
===============
The program is not in its finished state, yet.
Currently, the prismatik frame time has to be set such that it's greater than the overall color transition time. Otherwise there will be artefacts.

The total color transition time obviously contains `SMOOTH_TRANSITION_STEPS * SMOOTH_TRANSITION_WAIT_INTERMEDIATE`.
Furthermore you have to include the time that `FastLED.show()` will take. According to https://github.com/FastLED/FastLED/wiki/Interrupt-problems you have to calculate 3ms per 100 LEDs. I personally have had better luck using an estimate of 6ms per 100 LEDs (3ms for 53 LEDs). So the formula that seems to work for my setup is: 
```
PrismatikFrametime >= (SMOOTH_TRANSITION_STEPS * SMOOTH_TRANSITION_WAIT_INTERMEDIATE) + (SMOOTH_TRANSITION_STEPS * ((NUM_LEDS/100) * 6ms))
```

Due to lack of more data this formula could be very wrong for different configurations.

Any help is welcome!
