# Euclidean Sequencer Arduino Sketch

## Status
**Planned Features:
**
Change Tempo to an external input
Reset all with a long button press
Add length control for each sequence
Add offset control
Make encoder increment 1 step at a time (currently 2 steps)
Add LED indicator to show which sequencer you're using
Map BPM to a non-linear increment

**Currently Working Features:
**
BPM, Pulse, Length pots

## Overview

This Arduino sketch implements a Euclidean sequencer that generates rhythmic patterns based on user-defined parameters. The sequencer can be controlled using a rotary encoder, button, and analog potentiometers. It outputs sequences of 1s and 0s, representing beats and rests, which can be used to trigger external musical instruments or visual displays.

The main features of this sketch include:
- Euclidean algorithm for generating rhythmic patterns.
- Three independent sequencers (A, B, and C) that share parameters.
- User control of sequence length, pulses, and BPM using potentiometers.
- A button to switch between sequencers (A, B, and C).
- Debouncing for button input to prevent false presses.
- Serial communication for debugging and monitoring patterns.

## Hardware Requirements

To use this sketch, you will need the following hardware components:
- Arduino board (e.g., Arduino Uno)
- Rotary encoder with a button
- Three potentiometers (for sequence length, pulses, and BPM)
- A button for sequencer switching
- An LED matrix (optional, currently disabled in the code)
- Appropriate wiring and connections

## Usage

1. **Setup:** Connect the hardware components to your Arduino board as per your schematic.

2. **Upload the Sketch:** Use the Arduino IDE to upload this sketch to your Arduino board.

3. **Monitor in Serial:** Open the Arduino Serial Monitor at a baud rate of 9600 to observe the output, including Euclidean patterns and BPM adjustments.

4. **Control Sequencers:**
   - Use the rotary encoder to change the sequence length and pulses.
   - Press the encoder button to switch between sequencers A, B, and C.
   - Use the potentiometer for BPM control.

5. **Euclidean Patterns:** The sketch uses the Euclidean algorithm to generate patterns based on the number of beats (pulses) and the sequence length. Patterns are output as strings of 1s and 0s in the Serial Monitor.

6. **Additional Features:** There are several features marked as "TO ADD" in the code's comments. You can enhance the sketch by implementing these features, such as offset control and LED indicators for the active sequencer.

## Troubleshooting

If you encounter any issues or unexpected behavior:
- Double-check your hardware connections and ensure they match your schematic.
- Ensure that you have the required libraries installed in your Arduino IDE, such as SoftTimers, ezButton, ClickEncoder, and TimerOne.

## Credits

This sketch was created with inspiration from the Euclidean Rhythm Generator found at [https://dbkaplun.github.io/euclidean-rhythm/](https://dbkaplun.github.io/euclidean-rhythm/) and the [Groove Mechanics Euclid](http://www.groovemechanics.com/euclid/) tool.

## License

This code is provided as-is under an open-source license. You are free to modify and distribute it as needed. Please refer to the specific licenses of the libraries used in this sketch for additional information.

Enjoy creating unique rhythmic patterns with your Euclidean sequencer!
