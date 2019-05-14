#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

// buttons
const int NButtons = 2; //*
const int buttonPin[NButtons] = {2, 3}; //* the number of the pushbutton pins in the desired not order
int buttonCState[NButtons] = {0};         // stores the button current value
int buttonPState[NButtons] = {0};        // stores the button previous value
//byte pin13index = 3; // put the index of the pin 13 in the buttonPin[] if you are using it, if not, comment lines 68-70

// debounce
unsigned long lastDebounceTime[NButtons] = {0};  // the last time the output pin was toggled
unsigned long debounceDelay = 5;    //* the debounce time; increase if the output flickers

// potentiometers
const int NPots = 4; //*
int potPin[NPots] = {A0, A1, A2, A3}; //* Pin where the potentiometer is
int potCState[NPots] = {0}; // Current state of the pot
int potPState[NPots] = {0}; // Previous state of the pot
int potVar = 0; // Difference between the current and previous state of the pot

int midiCState[NPots] = {0}; // Current state of the midi value
int midiPState[NPots] = {0}; // Previous state of the midi value

int TIMEOUT = 300; //* Amount of time the potentiometer will be read after it exceeds the varThreshold
int varThreshold = 10; //* Threshold for the potentiometer signal variation
boolean potMoving = true; // If the potentiometer is moving
unsigned long PTime[NPots] = {0}; // Previously stored time
unsigned long timer[NPots] = {0}; // Stores the time that has elapsed since the timer was reset

bool block1 = false;
bool block2 = false;
bool block3 = false;
bool block4 = false;
byte midiCh = 1; //* MIDI channel to be used
byte note = 36; //* Lowest note to be used
byte cc = 1; //* Lowest MIDI CC to be used

void setup() {

  Serial.begin(115200); // use if using with ATmega328 (uno, mega, nano...)

  for (int i = 0; i < NButtons; i++) {
    pinMode(buttonPin[i], INPUT_PULLUP);
  }
  //pinMode(buttonPin[3], INPUT); //pin 13

}

void loop() {
  potentiometers();
}

// POTENTIOMETERS
void potentiometers() {

  delay(1000);
  
  
  potCState[0] = analogRead(potPin[0]);
  Serial.println(potCState[0]);
  if(potCState[0] > 900) {
    if(!block1) {
      MIDI.sendNoteOn(40, 127, midiCh);
      block1 = true;
    } else {
      midiCState[0] = map(potCState[0], 0, 1027, 127, 0); // Maps the reading of the potCState to a value usable in midi
      MIDI.sendControlChange(1, midiCState[0], midiCh); // cc number, cc value, midi channel
    }
  } else {
    MIDI.sendNoteOn(40, 0, midiCh);
    block1 = false;
  }
  
  potCState[1] = analogRead(potPin[1]);
  //Serial.println(potCState[1]);
  if(potCState[1] < 350) {
    if(!block2) {
      MIDI.sendNoteOn(50, 127, midiCh);
      block2 = true;
    } else {
      midiCState[1] = map(potCState[1], 0, 1027, 127, 0); // Maps the reading of the potCState to a value usable in midi
      MIDI.sendControlChange(1, midiCState[1], midiCh); // cc number, cc value, midi channel
    }
  } else {
    MIDI.sendNoteOn(50, 0, midiCh);
    block2 = false;
  }

  potCState[2] = analogRead(potPin[2]);
  //Serial.println(potCState[2]);
  if(potCState[2] < 900) {
    if(!block3) {
      MIDI.sendNoteOn(60, 127, midiCh);
      block3 = true;
    } else {
      midiCState[2] = map(potCState[2], 0, 1027, 127, 0); // Maps the reading of the potCState to a value usable in midi
      MIDI.sendControlChange(1, midiCState[2], midiCh); // cc number, cc value, midi channel
    }
  } else {
    MIDI.sendNoteOn(60, 0, midiCh);
    block3 = false;
  }

  potCState[3] = analogRead(potPin[3]);
  //Serial.println(potCState[3]);
  if(potCState[3] < 500) {
    if(!block4) {
      MIDI.sendNoteOn(70, 127, midiCh);
      block4 = true;
    } else {
      midiCState[3] = map(potCState[3], 0, 1027, 127, 0); // Maps the reading of the potCState to a value usable in midi
      MIDI.sendControlChange(1, midiCState[3], midiCh); // cc number, cc value, midi channel
    }
  } else {
    MIDI.sendNoteOn(70, 0, midiCh);
    block4 = false;
  }
}
