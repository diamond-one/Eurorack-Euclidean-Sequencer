#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <SPI.h>
#include <SoftTimers.h>
#include <ezButton.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

///// TIMING AND MULTI TASKING
SoftTimer tempoTimer; //millisecond timer
int legnth = 16;

int seq1Counter = 0;
int seq1Legnth = 16;
int seq1Memory = 16;

int seq2Counter = 16;
int seq2Legnth = 32;
int seq2Memory = 16;

int seq3Counter = 32;
int seq3Legnth = 48;
int seq3Memory = 16;

const int internalTempoPin = 2;
int internalTempo;

///// USER CONTROLS (POTS, BUTTONS & SWITCHES)

// int PosPin = 6; //Unused as of yet (Should be offset)
// int pulsesPin = 1;
// int tempoPin = 1;

///// ENCODER VARIABLES AND DECLIRATION 
enum encoder { PinA=3, PinB=2, IPINMODE=INPUT };
 static  byte abOld;         // Initialize state
int count;                  // current rotary count
int old_count;              // old rotary count

///// BUTTON TO CHANGE THE SEQUENCER USER IS WORKING WITH  
int stateCounter = 1; // Can be 1,2 or 3.. each state is a sequencer
ezButton seqSwitcher(5); 
// bool switcherPrevious = HIGH;  /CAN DELETE 

//REDUCING CURCIT NOISE ON BUTTON PRESSES[] ALSO USED IN NEW BUTTON ATEMPT 555]
long time = 0;         // the last time the output pin was toggled
long debounce = 50;   // the debounce time, increase if the output flickers


///// DECLARING FUNCTIONS
int Xpos(int position);
int Ypos(int position);
int sequenceProgress(int position);
void sequencerCheck();
void pinChangeISR();


///// NAVIGATING THE LED MATRIX
const int width = 8;
int x;
int y;


void setup() {
  Serial.begin(9600);
  Serial.println("8x8 LED Matrix Test");
  matrix.begin(0x70);  // pass in the address
  tempoTimer.setTimeOutTime(500); // 1 second.
  seqSwitcher.setDebounceTime(50); // set debounce time to 50 milliseconds




///// FULL DEVICE RESET
  tempoTimer.reset(); 
  seq1Counter = 0;  // so all sequencers start at the same time, on their first beat
  seq2Counter = 16; // so all sequencers start at the same time, on their first beat
  seq3Counter = 32; // so all sequencers start at the same time, on their first beat
  stateCounter = 1; // so the state of the button is 1, making the encoder deal with sequencer 1 
  matrix.clear();   // clearing matrix to start

///// ENCODER SETUP
  pinMode(PinA, IPINMODE);
  pinMode(PinB, IPINMODE);
  attachInterrupt(0, pinChangeISR, CHANGE); // Set up pin-change interrupts
  attachInterrupt(1, pinChangeISR, CHANGE);
  abOld =0; //A B OLD
  count =0; 
  old_count =0; // can also be written as 'abOld = count = old_count = 0;'
  Serial.begin(9600);
  Serial.println("Starting Rotary Encoder Test");
}

void loop() {

///// CHECKING USER CONTROLS 
seqSwitcher.loop(); // MUST call the loop() function first

  if(seqSwitcher.isPressed()){
    Serial.println("The button is pressed");
    sequencerCheck();
    Serial.println(stateCounter);
    if (stateCounter == 1){
      seq3Memory = legnth;
      legnth = seq1Memory;
    }
    else if (stateCounter == 2){
      seq1Memory = legnth;
      legnth = seq2Memory;
        }

    else if (stateCounter == 3){
      seq2Memory = legnth;
      legnth = seq3Memory;
    }
}
///// ENCODER PRINT OUT 
  if (old_count != count) {
    // Serial.println(count);
    old_count = count;
    legnth = constrain(count, 0, 16); // constraining encoder to 16 steps
    Serial.println(legnth);
  }

// BPM ADJUSTMENT 
internalTempo = map(analogRead(internalTempoPin), 0, 1023, 10, 2000);
tempoTimer.setTimeOutTime(internalTempo); // 1 secon
// Serial.println(internalTempo); //DEBUG

if (tempoTimer.hasTimedOut()){ // keeping tempo
  matrix.clear();      // clear display
  seq1Counter = seq1Counter +1; // progressing the sequencers 1 step
  seq2Counter = seq2Counter +1; // progressing the sequencers 1 step
  seq3Counter = seq3Counter +1; // progressing the sequencers 1 step

///// KEEPING SEQUIENCERS PROGRESSING ONE STEP AT A TIME, AS LONG AS THEIR SEQLEGNTH
  if (seq1Counter >= seq1Legnth){ // keeping steps inside their own 16 step sequencer
  seq1Counter = 0;
  }
  if (seq2Counter >= seq2Legnth){
  seq2Counter = 16;
  }
   if (seq3Counter >= seq3Legnth){
  seq3Counter = 32;
  }

///// SEQUENCE LEGNTH MODIFICATION  
switch (stateCounter){
  case 1:
  seq1Legnth = legnth;
  break;

  case 2:
  seq2Legnth = legnth + 16;
  break;

  case 3:
  seq3Legnth = legnth + 32;
  break;
}

///// ROUTING STEP COUNTERS TO XY ON LED MATRIX 
sequenceProgress(seq1Counter);
sequenceProgress(seq2Counter);
sequenceProgress(seq3Counter);
tempoTimer.reset();

}
}

///// FUNCTIONS BELOW ///// FUNCTIONS BELOW ///// FUNCTIONS BELOW ///// 

///// THIS FORMULA LETS THE PROGRAM ACCESS TWO XY COORDINATES WITH A SINGLE DIGIT
int Xpos (int position){
int m = floor(position/width);
return m;
}

int Ypos (int position){
int o = position % width;
return o;
}

// THIS FUNCTION APPLIES THE X AND Y TO ACCESS LEDS ON THE LED MATRIX
int sequenceProgress(int position){
x = Xpos(position);
y = Ypos(position);
 
matrix.drawPixel(y, x, LED_ON);  
matrix.writeDisplay();  
}

///// THIS CHECKS WHICH SEQUENCE THE LEGNTH KNOB SHOULD BE
void sequencerCheck(){ //sequencer A B or C (1,2,3)
  stateCounter = stateCounter +1; //Strange... stateCounter++ doesn't work here. Why?
  if (stateCounter >= 4){
    stateCounter = 1;  // i've put the following inside this top if statment.. needed? 
  }
}

///// ENCODER CHECK FUCNTION : ISR [Interrupt Service Routine]
void pinChangeISR() {
  enum { upMask = 0x66, downMask = 0x99 };
  byte abNew = (digitalRead(PinA) << 1) | digitalRead(PinB); //| (bitwise or) //The right shift operator >> causes the bits of the left operand to be shifted right by the number of positions specified by the right operand.
  byte criterion = abNew^abOld; // A bitwise XOR operation results in a 1 only if the input bits are different, else it results in a 0.
  if (criterion==1 || criterion==2) {  //|| (logical or) 
    if (upMask & (1 << (2*abOld + abNew/2)))
      count--;
    else count++;       // upMask = ~downMask
  }
  abOld = abNew;        // Save new state
}

//// THINGS TO ADD
// RESET ALL WITH LONG BUTTON PRESS 
// ADD LEGNTH CONTROL FOR EACH SEQUENCE 
// ADD BPM ADJUSTMENT
// ADD EUCLID MATH
// ADD OFFSET CONTROL
// MAKE ENCODER INCRIMENT 1 AT A TIME, NOT 2
// ADD LED TO SHOW WHICH SEQUENCER YOU'RE USING 
