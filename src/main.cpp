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

/* AIGHT bruv, in a nutshell, this is what i'm building https://dbkaplun.github.io/euclidean-rhythm/
a euclidian sequencer. Basically you give it a few perameters and it outputs beats. 
you don't have control of where the beats sit in your sequencer, but you do have control of the devisions. 

This site is similar to what i'm building (same fucntionality) http://www.groovemechanics.com/euclid/
The differnce being the site has one more fucntion (offset) which I have yet to impliment. 

My euclidian devision function (found at the bottom of this code) outputs a string of boolians, the string is as 
long as the 'legnth' of the sequence, and the amount of '1s' in the string is the amount of devisions. 
Example.. we have a sequence 5 beats long which is '00000' if we devide that by 1 we will have '00100'. 
We devide it by 2 and we have '10100' the algorythm trys to space the beats out as evenly as it can, but when you 
feed it odd numbers it ends up with some of the gaps between the beats being bigger than others. 

This creates world music rythms... no shit. 
"10100" Clasical, jazz, persian.
"1010100" Bulgarian folk
"10010010010" Frank Zappa
*/


///// TIMING AND MULTI TASKING
SoftTimer tempoTimer; //millisecond timer
int legnth = 16; // Global legnth to start things out

int seq1Counter = 0; //My LED screen has 64 LEDs I've written a function to give each of them a number
// 1- 64, that's how i'm accessing them. This seq1Counter is 0 because it's the first LED
int seq1Legnth = 16; // 16 because it's the last in the first sequence.. the other two sequencers which have the 
// same logic
int seq1Memory = 16; // This is a variable that the program stores the legnth of a sequence in case it was changed
//by the user. 
String seq1EuclidMem = ; // holding euclidan sequence in 'memory' while we deal with other sequencers. 

int seq2Counter = 16;
int seq2Legnth = 32;
int seq2Memory = 16;
String seq1EuclidMem = ;

int seq3Counter = 32;
int seq3Legnth = 48;
int seq3Memory = 16;
String seq1EuclidMem = ;

const int internalTempoPin = 2; //allocating a potentiometer to Pin 2 on the circuit board.  
int internalTempo;

///// USER CONTROLS (POTS, BUTTONS & SWITCHES)

int pulsesPin = 3;
unsigned int pulses = 1;

///// ENCODER VARIABLES AND DECLIRATION
// I have an encoder hooked up which outputs bytes as it turns, this keeps old and new variables
enum encoder { PinA=3, PinB=2, IPINMODE=INPUT };
static  byte abOld;         // Initialize state
int count;                  // current rotary count
int old_count;              // old rotary count

///// BUTTON TO CHANGE THE SEQUENCER USER IS WORKING WITH  
// This helps me read if the encoder button is pushed or not
int stateCounter = 1; // Can be 1,2 or 3.. each state is a sequencer
ezButton seqSwitcher(5); 
// bool switcherPrevious = HIGH;  /CAN DELETE 

//REDUCING CURCIT NOISE ON BUTTON PRESSES
// this eliminates any false button presses due to short circuts in the encoder. 
long time = 0;         // the last time the output pin was toggled
long debounce = 50;    // the debounce time, increase if the output flickers

///// DECLARING FUNCTIONS
// function explinations at bottom of page above each fucntion. 
int Xpos(int position); 
int Ypos(int position);
int sequenceProgress(int position); 
void sequencerCheck();
void pinChangeISR();
String euclideanAlgo(int beats, int steps);

///// NAVIGATING THE LED MATRIX
const int width = 8;
int x;
int y;

void setup() {
  Serial.begin(9600);
  Serial.println("Euclidian sequencer program running");
  matrix.begin(0x70);  // pass in the address
  tempoTimer.setTimeOutTime(500); // 1 second.
  seqSwitcher.setDebounceTime(50); // set debounce time to 50 milliseconds

///// FULL DEVICE RESET
//Maybe all overkill but keeping this here for readability
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
  old_count =0; 
}

void loop() {

///// CHECKING USER CONTROLS 
seqSwitcher.loop(); // MUST call the loop() function first


//This is a switcher block which gets a read from the button, the button when pressed adds 1 to stateCounter up to 3,
// by doing this the program knows which sequencer to work with , sequence, 1, 2 or 3.
  if(seqSwitcher.isPressed()){
    Serial.println("The button is pressed"); //Debug to check if encoder was pressed
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
//Here we are taking a read from an analog potentiometer, and mapping the read to  1-16
//this will then be our devision for our euclidian devision algorythm. 
pulses = map(analogRead(pulsesPin), 0, 1023, 16, 0);
// pulses = 3; // comment out the line above, and uncomment this if you want to change the euclidian devision manually

///// EUCLIDEAN ALGO TEST AREA 
  String euclidPattern = euclideanAlgo(pulses, legnth); 
  // This will give you a read out in your terminal of 0's and 1's.
  // Serial.println(euclidPattern); 
  // delay(500); // Delay if you want to slow that shit down in the terminal


///// ENCODER PRINT OUT 
  if (old_count != count) {
  
    old_count = count;
    // because encoders are endless,t his is constraining it's output to 16 steps. 
    //The print out actually goes outside of the 0-16 range, but 'legnth' doesn't
  
    legnth = constrain(count, 0, 16);
    // Serial.println(count); //Uncomment to see encoder output
  }

// BPM ADJUSTMENT 
// this is reading an analog potentioemter, mapping it to readable tempos... 10BPM-2000BPM
internalTempo = map(analogRead(internalTempoPin), 0, 1023, 10, 2000);
tempoTimer.setTimeOutTime(internalTempo); 
// Serial.println(internalTempo); //DEBUG

if (tempoTimer.hasTimedOut()){  // keeping tempo without using 'delay' functions
  matrix.clear();               // clear LED display, this happens just this once, though each loop
  seq1Counter = seq1Counter +1; // progressing sequence 1, 1 step
  seq2Counter = seq2Counter +1; // progressing sequence 2, 1 step
  seq3Counter = seq3Counter +1; // progressing sequence 3, 1 step

///// KEEPING SEQUIENCERS PROGRESSING ONE STEP AT A TIME, AS LONG AS THEIR 'seqLegnth'
  if (seq1Counter >= seq1Legnth){ // keeping steps inside their own 16 steps
  seq1Counter = 0;
  }
  if (seq2Counter >= seq2Legnth){
  seq2Counter = 16;
  }
   if (seq3Counter >= seq3Legnth){
  seq3Counter = 32;
  }

///// SEQUENCE LEGNTH MODIFICATION 
// this uses the switcher from the buttom press to adjust the lengh of each sequence.  
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

String euclideanAlgo(int beats, int steps) //Steps == pulses
{
    //We can only have as many beats as we have steps (0 <= beats <= steps)
    if (beats > steps)
        beats = steps;

    //Each iteration is a process of pairing strings X and Y and the remainder from the pairings
    //X will hold the "dominant" pair (the pair that there are more of)
    String x = "1";
    int x_amount = beats;

    String y = "0";
    int y_amount = steps - beats;

    do
    {
        //Placeholder variables
        int x_temp = x_amount;
        int y_temp = y_amount;
        String y_copy = y;

        //Check which is the dominant pair 
        if (x_temp >= y_temp)
        {
            //Set the new number of pairs for X and Y
            x_amount = y_temp;
            y_amount = x_temp - y_temp;

            //The previous dominant pair becomes the new non dominant pair
            y = x;
        }
        else
        {
            x_amount = x_temp;
            y_amount = y_temp - x_temp;
        }

        //Create the new dominant pair by combining the previous pairs
        x = x + y_copy;
    } while (x_amount > 1 && y_amount > 1);//iterate as long as the non dominant pair can be paired (if there is 1 Y left, all we can do is pair it with however many Xs are left, so we're done)

    //By this point, we have strings X and Y formed through a series of pairings of the initial strings "1" and "0"
    //X is the final dominant pair and Y is the second to last dominant pair
    String rhythm;
    for (int i = 1; i <= x_amount; i++)
        rhythm += x;
    for (int i = 1; i <= y_amount; i++)
        rhythm += y;
    return rhythm;
}

//// THINGS TO ADD
// RESET ALL WITH LONG BUTTON PRESS 
// ADD LEGNTH CONTROL FOR EACH SEQUENCE 
// ADD BPM ADJUSTMENT
// ADD EUCLID MATH
// ADD OFFSET CONTROL
// MAKE ENCODER INCRIMENT 1 AT A TIME, NOT 2
// ADD LED TO SHOW WHICH SEQUENCER YOU'RE USING 
// MAP BPM TO A NON LINIAR INCREMENT