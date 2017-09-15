This repository contains the development history of the digital component of a synthesizer module. It utilizes the STM32 series of microcontrollers as the brains of the operation. This allows us to take advantage of the STM32CubeMX toolchain, which generates the tedious and overwheling setup code for the chip via a rather friendly GUI. We will highlight the relevant sections of user code required to make the chip do what we want it to do. 

If this all looks like gibberish but you want to know more, liquidcitymotors@gmail.com

Some context: The module takes two inputs and crossfades between them, with the position of the crossfader determined by a digital oscillator, implemented on the STM32. By constructiong the oscillator in two segments, "attack" and "release" and utilizing two distinct control modes, one for audio rate (typical 1v/oct oscillator) and one for CV rate (envelopes and LFOs), we get a surprisingly versatile oscillator that adapts quite nicely to its analog habitat. Keep in mind that applying our oscillator to two DC signals is a nifty way of outputtng that value with scaling and bias according to the DC values at the crossfader.

The oscillator also implements a "wavestacking" technique wherin a "family" of wavetables is defined, and the user can "morph" through the family. This allows us to change the shape of CV modulation or the timbre of an audio rate oscillator.

The oscillator also controls a pair of sample and holds on the inputs, which allow for chaining multiple units at low frequency and strange analog bit reduction/sample rate reduction effects in the audio range. A nice perk of the crossfader implementation is that the A and B inputs act as ring modulators for the oscillator.

There are three knobs and three CV jacks to control the parameters of the oscillator. The jacks scale -5V to 5V eurorack full scale (ish) voltage to 0 - 3.3V, aka full scale voltage for the ADCs on the chip. 

4 buttons are used to toggle the parameters for 4 modes, "speed", "retrigger", "loop", and "sample and hold". All combinations of these modes are valid and unique (with the exception that a few of the retrigger modes are lumped into one behavior when "speed" is audio and "loop" is in one shot mode).

The most current working version can be found in Documents/stm32 workspace/f4interpolatortest

Within that folder, most of the code is generated from the STM32CubeMX app. The project file is f4interpolatortest.ioc

User code can be found in in /inc/tables.h, /src/stm32f4xx_it.c, and /src/main.c

tables.h contains, surprise surprise, the definitions for the wavetables used by the synth 

main.c contains some initialization code drudgery to get the timers, DAC, and ADC up and running as well as the UI handling functions

stm32f4xx_it.c contains the interrupt service routines (ISR) which do most of the work in our program. For those who are unfamiliar, an "interrupt" is a block of code that can take precendence over the main loop at run time. This is ideal for digital audio, which requires us to update the DAC (digital to analog converter) at a precise rate. Essentially, whenver its time to come up with the next sample value in our signal, the processor pauses whatever it had been doing and completes the routine needed to come up with the next sample. Once that is complete, it goes back to what it had been doing (usually watching our UI for button presses and lighting up the RGB indicator). This also allows it to react in almost real time to its control inputs.

The architecture of the program is as follows: Timer 6 triggers an interrupt routine at 90khz. The oscillator is implemented within that interrupt service function, TIM6_DAC_IRQHandler(). The behavior of that ISR is beholden to a few global variables, namely the variables that hold the current mode, the ADC readings (knobs and jacks, which always live in the array ADCReadings[] thanks to DMA), and a trigger and retrigger flag. 

A separate ISR (TIM2_IRQHandler()) sets the "trig" flag high whenever it is called (which would indicate a rising edge at the "trigger in" jack). This sets the oscillator in motion, so to speak. It also sets a "retrigger" flag which is parsed by the various trigger modes.

The oscillator checks conditionals at different points in the ISR for the state of 3 variables which indicate the current "mode" for 3 parameters: "speed", "loop", and "trigmode" (determined in the main loop). More on the practical charateristics of these different modes can be found in the manual.

The oscillator raises a flag and sets a pending interrupt request (to be handled by EXTI15_10_IRQHandler()) whenever the phase pointer ("position") enters the release or attack phase. Due to the "priority" of the interrupts (defined in the CubeMX project), the processor waits to complete the oscillator maintenance before attending to this pending interrupt request.

EXTI15_10_IRQHandler() in turn sets the state of the two sample and holds which are part of the analog circuit associated with the microcontroller. The exact behavior of this ISR is determined by a mode variable "samphold" as well as the particular flag that was raised along with the interrupt request ("intoattackfroml", "intoattackfromr", "intoreleasefroml", "intoreleasefromr"). Details of the sample and hold modes are best left for the manual. We also set a GPIO pin high (to a jack as a gate output) when in attack mode, and another gate high when in release mode.

A single cycle of an audio rate waveform can be useful but we thought this an apt time to add a bonus mode for the best sort of sounds, drums. Thus, when when "speed" is audio and "loop" is in one shot mode, the Drum() function is called. This uses the 1v/oct lookup table to generate an exponential decay curve, the length of which is controlled by the time 2 knob and CV. That same exponential decay is also applied to the morph parameter, with the morph knob and CV controlling the starting point of the decay. When we arrange a wavetable family such that there is increasing high frequency content across the family, this can yield lowpass gate-esque pings (keeping in mind the time 1 knob and cv retain their 1v/oct characteristic).

The main loop sequentially checks for a button press on each of the mode change buttons and then updates the PWM channels of the relevant timers with the latest value of the oscillator and the current amount of "morph". If it detects a buttom press, it shows the current mode for that button on the 4 leds and blanks the RGB LED. On releasing the button, it changes the mode and shows the new mode for a second or so. Two of the buttons change the wavetable family, but that is indicated similarly. Eventually, if you hold the button long enough, it wont change the mode upon release, so basically, a tap switches to the next mode and a hold shows you the current mode.

More nuance in the comments, dive in if you want to grab any bits of the above.


