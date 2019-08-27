#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

// LDR's
const int NPots = 4; // Number of LDRs
int potPin[NPots] = {A0, A1, A2, A3}; // LDR pins
int potCState[NPots] = {0}; // Current LDR state

int midiCState[NPots] = {0}; // Current state of the MIDI value

bool block1 = false; //Used the check if a beam is blocked
bool block2 = false;
bool block3 = false;
bool block4 = false;

bool unBlock1 = false; //Used the check if a beam is blocked
bool unBlock2 = false;
bool unBlock3 = false;
bool unBlock4 = false;
byte midiCh = 1; // MIDI channel to be used
int threshold = 300;

void setup() {

  Serial.begin(115200); 
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

}

void loop() {
  float latency = millis();
  potentiometers();
  Serial.println(latency-millis());
}

// POTENTIOMETERS
void potentiometers() {

  delay(10);
  
  
  potCState[0] = analogRead(potPin[0]);
  //Serial.println(potCState[0]);
  if(potCState[0] > threshold) {
    if(!block1) {
      MIDI.sendNoteOn(60, 127, midiCh);
      block1 = true;
      unBlock1 = false;
    } else {
      midiCState[0] = map(potCState[0], threshold, 800, 127, 0); // Maps the LDR value to be used in MIDI
      MIDI.sendControlChange(1, midiCState[0], midiCh); // controller number, controller value, MIDI channel
    }
  } else if (!unBlock1){
    MIDI.sendNoteOn(60, 0, midiCh);
    unBlock1 = true;
    block1 = false;
  }
  
  potCState[1] = analogRead(potPin[1]);
  //Serial.println(potCState[1]);
  if(potCState[1] > threshold) {
    if(!block2) {
      MIDI.sendNoteOn(70, 127, midiCh);
      unBlock2 = false;
      block2 = true;
    } else {
      midiCState[1] = map(potCState[1], threshold, 1000, 127, 0); // Maps the LDR value to be used in MIDI
      MIDI.sendControlChange(1, midiCState[1], midiCh); // controller number, controller value, MIDI channel
    }
  } else if (!unBlock2) {
    MIDI.sendNoteOn(70, 0, midiCh);
    unBlock2 = true;
    block2 = false;
  }

  potCState[2] = analogRead(potPin[2]);
  //Serial.println(potCState[2]);
  if(potCState[2] > threshold) {
    if(!block3) {
      MIDI.sendNoteOn(80, 127, midiCh);
      unBlock3 = false;
      block3 = true;
    } else {
      midiCState[2] = map(potCState[2], threshold, 1000, 127, 0); // Maps the LDR value to be used in MIDI
      MIDI.sendControlChange(1, midiCState[2], midiCh); // controller number, controller value, MIDI channel
    }
  } else if (!unBlock3) {
    MIDI.sendNoteOn(80, 0, midiCh);
    unBlock3 = true;
    block3 = false;
  }

  potCState[3] = analogRead(potPin[3]);
  //Serial.println(potCState[3]);
  if(potCState[3] > threshold) { 
    if(!block4) {
      MIDI.sendNoteOn(90, 127, midiCh);
      unBlock4 = false;
      block4 = true;
    } else {
      midiCState[3] = map(potCState[3], threshold, 1000, 127, 0); // Maps the LDR value to be used in MIDI
      MIDI.sendControlChange(1, midiCState[3], midiCh); // controller number, controller value, MIDI channel
    }
  } else if (!unBlock4) { 
    MIDI.sendNoteOn(90, 0, midiCh);
    unBlock4 = true;
    block4 = false;
  }
}
