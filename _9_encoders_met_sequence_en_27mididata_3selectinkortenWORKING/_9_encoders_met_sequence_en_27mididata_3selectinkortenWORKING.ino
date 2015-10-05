#include <MIDI.h>
#include <Encoder.h>
#include <Wire.h>
#include "Adafruit_MCP23017.h"
Adafruit_MCP23017 mcp;

// Encoder pins
Encoder ampEnc0(21, 20);
Encoder ampEnc1(30, 31);
Encoder ampEnc2(14, 15);
Encoder ampEnc3(2, 1);
Encoder ampEnc4(12, 11);
Encoder ampEnc5(7, 8);
Encoder ampEnc6(26, 27);
Encoder ampEnc7(17, 16);
Encoder ampEnc8(24, 28);

// mcp Pins
int mcpIn [7] = {0, 12, 8, 4, 5, 6, 7};
int mcpOut [9] = {1, 2, 3, 9, 10, 11, 13, 14, 15};
// Ledring Pins
int ledPins [9] = {10, 32, 6, 3, 5, 4, 25, 9, 23};

// configurable data//
int midiChannel = 1;
int UP = 68;
int DOWN = 60;
// //////////////// //
float ledValues [27];

long encPos [9] = { -999, -999, -999, -999, -999, -999, -999, -999, -999,};


//buttondebounce stuff
#define DRE(signal, state) (state=(state<<1)|(signal&1)&15)==7
int buttonState [3]; //osc/amp/filtr
int buttonArray [3] = {0, 12, 8};
int counter [3] = {0, 0, 0}; //statustellers
int ampStatusLed [3] = {15, 14, 13};
int oscStatusLed [3] = {3, 2, 1};
int filtrStatusLed [3] = {11, 10, 9};


void setup() {
  Serial.begin(9600);
  mcp.begin();

  for (int m = 0; m < 9; m++) {
    mcp.pinMode(mcpOut[m], OUTPUT);
  }

  for (int n = 0; n < 7; n++) {
    mcp.pinMode(mcpIn[n], INPUT);
  }
  for (int i = 0; i < 9; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  usbMIDI.setHandleControlChange(myCC);
  MIDI.begin();              // Launch MIDI with default options
  updateStatusLeds(0, 0, 0); // start everything at 0
}



void loop() {

  long newampEnc [9];
  // put your main code here, to run repeatedly:
  usbMIDI.read();


  newampEnc [0] = ampEnc0.read();
  newampEnc [1] = ampEnc1.read();
  newampEnc [2] = ampEnc2.read();
  newampEnc [3] = ampEnc3.read();
  newampEnc [4] = ampEnc4.read();
  newampEnc [5] = ampEnc5.read();
  newampEnc [6] = ampEnc6.read();
  newampEnc [7] = ampEnc7.read();
  newampEnc [8] = ampEnc8.read();



  for (int i = 0; i < 9; i++) {
    // checks if encoder has moved
    if (newampEnc[i] != encPos[i]) {
      if (newampEnc[i] > encPos[i]) {
        midiCC(0, i);
      }
      else {
        midiCC (1, i);
      }
    }
    encPos[i] = newampEnc[i];
  }
  usbMIDI.read();
  printLeds();

  //checks if button has changed, if so, updates counter
  for (int m = 0; m < 3; m++) if (DRE(mcp.digitalRead((buttonArray [m])), buttonState[m])) {

      counter [m] ++;
      if (counter [m] > 2) counter [m] = 0;
      int x = counter [0];
      int y = counter [1];
      int z = counter [2];
      updateStatusLeds(x, y, z);
    }
}


void midiCC(int n, int i) {
  //send a midi message
  if (n == 0) { //turn right
    if (i >= 0 && i <= 2) {
      usbMIDI.sendControlChange((i * 3) + counter[0] , UP, midiChannel);  // encoder 0 up to 2 send '68 (up)' to defined midiChannel, control data number dependent on state of counter[0]
      ledValues [(i * 3) + counter[0]] += 0.33;                           // quick adjust ledvalues
    }
    if (i >= 3 && i <= 6) {
      usbMIDI.sendControlChange((i * 3) + counter[1] , UP, midiChannel);  
      ledValues [(i * 3) + counter[1]] += 0.33;
    }
    if (i >= 7 && i <= 8) {
      usbMIDI.sendControlChange((i * 3) + counter[2] , UP, midiChannel);
      ledValues [(i * 3) + counter[2]] += 0.33;
    }
  }
  else { //turn left
    if (i >= 0 && i <= 2) {
      usbMIDI.sendControlChange((i * 3) + counter[0] , DOWN, midiChannel);  // encoder 0 up to 2 send '60(down)' to defined midiChannel, control data number dependent on state of counter[0]
      ledValues [(i * 3) + counter[0]] -= 0.33;
    }
    if (i >= 3 && i <= 6) {
      usbMIDI.sendControlChange((i * 3) + counter[1] , DOWN, midiChannel);
      ledValues [(i * 3) + counter[1]] -= 0.33;
    }
    if (i >= 7 && i <= 8) {
      usbMIDI.sendControlChange((i * 3) + counter[2] , DOWN, midiChannel);
      ledValues [(i * 3) + counter[2]] -= 0.33;
    }
  }


  ledValues[i] = constrain(ledValues[i], 0, 22);                          // don't let ledvalues be more than 22, the max input of the ledrings
}

void myCC (byte channel, byte number, byte value) {
  //   handle incoming cc messages
  if (channel == midiChannel) {
    if ((number >= 0) && (number <= 26)) {
      ledValues [number] = map(value, 0 , 127, 0, 22);                     // map incoming cc to range of  0-22
      if (number >= 0 && number <= 8) {
        analogWrite (ledPins [(number - counter[0]) / 3], ledValues [number]); // write incoming incoming cc to ledring 0-8, depending on state of counters
      }
      if (number >= 9 && number <= 20) {
        analogWrite (ledPins [(number - counter[1]) / 3], ledValues [number]);
      }
      if (number >= 21 && number <= 26) {
        analogWrite (ledPins [(number - counter[2]) / 3], ledValues [number]);
      }
    }
  }
}
void printLeds() {
  //writes Leds, depending on counter states
  for (int i = 0; i < 9; i++) {
    if (i >= 0 && i <= 2) {
      int pos0 = ledValues[(i * 3) + counter[0]];
      analogWrite (ledPins[i], pos0);
    }
    if (i >= 3 && i <= 6) {
      int pos1 = ledValues[(i * 3) + counter[1]];
      analogWrite (ledPins[i], pos1);
    }
    if (i >= 7 && i <= 8) {
      int pos2 = ledValues[(i * 3) + counter[2]];
      analogWrite (ledPins[i], pos2);
    }
  }
}

void updateStatusLeds (int x, int y, int z) {
  // changes statusleds, displaying counter [0],[1] and [2] states
  for (int i = 0; i < 3; i++)
  {
    if (i == x)
    {
      mcp.digitalWrite(oscStatusLed [i], HIGH);
    }
    else
    {
      mcp.digitalWrite(oscStatusLed [i], LOW);
    }
    if (i == y)
    {
      mcp.digitalWrite(ampStatusLed [i], HIGH);
    }
    else
    {
      mcp.digitalWrite(ampStatusLed [i], LOW);
    }
    if (i == z)
    {
      mcp.digitalWrite(filtrStatusLed [i], HIGH);
    }
    else
    {
      mcp.digitalWrite(filtrStatusLed [i], LOW);
    }
  }
}
