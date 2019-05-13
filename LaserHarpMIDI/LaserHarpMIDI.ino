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
//bool block[4] = {false};
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

  //buttons();
  potentiometers();

}

// BUTTONS
void buttons() {

  for (int i = 0; i < NButtons; i++) {

    buttonCState[i] = digitalRead(buttonPin[i]);
    /*
        // Comment this if you are not using pin 13...
        if (i == pin13index) {
          buttonCState[i] = !buttonCState[i]; //inverts pin 13 because it has a pull down resistor instead of a pull up
        }
        // ...until here
    */
    if ((millis() - lastDebounceTime[i]) > debounceDelay) {

      if (buttonPState[i] != buttonCState[i]) {
        lastDebounceTime[i] = millis();

        if (buttonCState[i] == LOW) {
          // use if using with ATmega328 (uno, mega, nano...)
          //do usbMIDI.sendNoteOn if using with Teensy
          MIDI.sendNoteOn(note + i, 127, midiCh); // note, velocity, channel

          // use if using with ATmega32U4 (micro, pro micro, leonardo...)
          //          noteOn(midiCh, note + i, 127);  // channel, note, velocity
          //          MidiUSB.flush();

          //          Serial.print("button on  >> ");
          //          Serial.println(i);
        }
        else {
          // use if using with ATmega328 (uno, mega, nano...)
          //do usbMIDI.sendNoteOn if using with Teensy
          MIDI.sendNoteOn(note + i, 0, midiCh); // note, velocity, channel

          // use if using with ATmega32U4 (micro, pro micro, leonardo...)
          //          noteOn(midiCh, note + i, 0);  // channel, note, velocity
          //          MidiUSB.flush();

          //          Serial.print("button off >> ");
          //          Serial.println(i);
        }
        buttonPState[i] = buttonCState[i];
      }
    }
  }
}

// POTENTIOMETERS
void potentiometers() {
  /*
  potCState[0] = analogRead(potPin[0]);
  if(potCState[0] > 150) {
    if(!block1) {
      MIDI.sendNoteOn(40, 127, 1);
      block1 = true;
    } else {
      midiCState[0] = map(potCState[0], 0, 400, 127, 0); // Maps the reading of the potCState to a value usable in midi
      MIDI.sendControlChange(1, midiCState[0], midiCh); // cc number, cc value, midi channel
    }
  } else {
    MIDI.sendNoteOn(40, 0, midiCh);
    block1 = false;
  }
  */

  potCState[1] = analogRead(potPin[1]);
  if(potCState[1] > 150) {
    if(!block2) {
      MIDI.sendNoteOn(50, 127, 2);
      block2 = true;
    } else {
      midiCState[1] = map(potCState[1], 0, 400, 127, 0); // Maps the reading of the potCState to a value usable in midi
      MIDI.sendControlChange(1, midiCState[1], midiCh); // cc number, cc value, midi channel
    }
  } else {
    MIDI.sendNoteOn(50, 0, midiCh);
    block2 = false;
  }

  
  

  /*
  for (int i = 0; i < NPots; i++) { // Loops through all the potentiometers

    potCState[i] = analogRead(potPin[i]); // Reads the pot and stores it in the potCState variable
    if (potCState[i] > 900) {
      if (!test) {
        MIDI.sendNoteOn(note + i, 127, midiCh + 1); // note, velocity, channel
        test = true;

      }
    } else {
      MIDI.sendNoteOn(note + i, 0, midiCh + 1);
      test = false;
    }
    

    midiCState[i] = map(potCState[i], 0, 400, 127, 0); // Maps the reading of the potCState to a value usable in midi


    potVar = abs(potCState[i] - potPState[i]); // Calculates the absolute value between the difference between the current and previous state of the pot

    if (potVar > varThreshold) { // Opens the gate if the potentiometer variation is greater than the threshold
      PTime[i] = millis(); // Stores the previous time
    }

    timer[i] = millis() - PTime[i]; // Resets the timer 11000 - 11000 = 0ms

    if (timer[i] < TIMEOUT) { // If the timer is less than the maximum allowed time it means that the potentiometer is still moving
      potMoving = true;
    }
    else {
      potMoving = false;
    }

    if (potMoving == true) { // If the potentiometer is still moving, send the change control
      if (midiPState[i] != midiCState[i]) {

        // use if using with ATmega328 (uno, mega, nano...)
        //do usbMIDI.sendControlChange if using with Teensy
        MIDI.sendControlChange(cc + i, midiCState[i], midiCh); // cc number, cc value, midi channel

        // use if using with ATmega32U4 (micro, pro micro, leonardo...)
        //        controlChange(midiCh, cc + i, midiCState[i]); //  (channel, CC number,  CC value)
        //        MidiUSB.flush();


        //Serial.println(midiCState);
        potPState[i] = potCState[i]; // Stores the current reading of the potentiometer to compare with the next
        midiPState[i] = midiCState[i];
      }
    }
  }
  */
}
