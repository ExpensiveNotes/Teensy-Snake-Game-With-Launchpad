/* Using USB Host to send and recieve messages from Launchpad Mini Mk3
    Make sure it's in Programmer mode after each reboot
    Long Press: Session and press StopMuteSolo. Press Session and User

    Notes: Programmer Mode assumes a 10x10 grid except that row 10 and column 10 are off the screen
            Can only recieve MIDI notes from 8x8 grid
            So stick to 8x8 grid for games.
            Bottom left corner is "origin" (1,1)

    need to translate (x,y) coords to MIDI notes

*/

#include "USBHost_t36.h"
#define GREEN 87
#define PINK 53

int note = 11;
int vel = 0;
int noteIndex = 11;
int xPos = 1;
int yPos = 1;
int foodX = 8;
int foodY = 8;
int snakeLength = 1;

int snakeColour [10] = {5, 53, 53, 53, 53, 53, 82, 53, 82, 53};
int snakeX [10] = {5, -1, -1, -1, -1, -1, -1, -1, -1, -1};
int snakeY [10] = {5, -1, -1, -1, -1, -1, -1, -1, -1, -1};
int moveX = -1;
int moveY = 0;
int score = 0;

//Notes
int octave = 1;
int root = 26; //D
int minorScale [15] = {0, 2, 3, 5, 7, 9, 10, 12, 14, 15, 17, 19, 21, 22, 24}; // D, E, F, G, A, B♭, and C.
int melody [8] = {0, 1, 2, 3, 4, 5, 6, 7};

//MIDI
int noteON1 = 144; // note on command Channel 1
int noteOFF1 = 128; // note off command Channel 1
int noteON2 = 145; // note on command Channel 2
int noteOFF2 = 129; // note off command Channel 2
int noteON3 = 146; // note on command Channel 3
int noteOFF3 = 130; // note off command Channel 3
int SendCC1 = 176; // CC message to channel 1
int SendCC2 = 177; // CC message to channel 2
int SendCC3 = 178; // CC message to channel 3
byte midi_clock = 0xf8;
//byte startMIDI = 0xfa;
byte stopMIDI = 0xfc; //MIDI stop


USBHost myusb;
//USBHub hub1(myusb);
//USBHub hub2(myusb);
//USBHub hub3(myusb);
MIDIDevice midi1(myusb);

void setup()
{
  Serial.println("USB Host Testing");
  myusb.begin();
  //recieving MIDI functions
  midi1.setHandleNoteOff(OnNoteOff);
  midi1.setHandleNoteOn(OnNoteOn);
  midi1.setHandleControlChange(OnControlChange);
  clearBoard();
  Serial1.begin(31250); //MIDI baud rate
  Serial1.write(stopMIDI); // just in case
  makeRandomMelody();
}

void loop()
{
  myusb.Task();
  midi1.read();
  //sendToDisplay();
  //lightHitNote();
  playMIDInote1(noteON1, octave * root + melody[snakeX[0]] , 0);
  moveSnake();
  octave = snakeY[0] % 3 + 1;
  playMIDInote1(noteON1, octave * root + melody[snakeX[0]] , 110);
  drawFood();
  drawSnake();
  checkFood();
  delay(300);
  //eraseSnake();
  clearBoard();
}

//=============================================================

void makeRandomMelody() {
  for (int i = 0; i < 8; i++) {
    melody[i] = random(15);
  }
}

void clearBoard() {
  for (int i = 11; i < 100; i++) {
    midi1.sendNoteOn(i , 42, 1, 1);
  }
  int iscore = score % 10;
  midi1.sendNoteOn(iscore * 10 + 9 , GREEN, 1, 1);
  int tenScore = int(score / 10);
  if (tenScore > 0) midi1.sendNoteOn(99 - tenScore , GREEN, 1, 1);
}

void checkFood() {
  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    //got food find new spot
    while (foodX == snakeX[0] || foodY == snakeY[0]) {
      //      foodX = int(random(8)) + 1;
      //      foodY = int(random(8)) + 1;
      foodX = millis() % 8 + 1;
      foodY = int(millis() / 20) % 8 + 1;
      Serial.print(foodX);
      Serial.print(" , ");
      Serial.println(foodY);
    }
    score++;
    makeRandomMelody();
    //make snake longer
    if (snakeLength < 7) {
      snakeX[snakeLength] = snakeX[snakeLength - 1];
      snakeY[snakeLength] = snakeY[snakeLength - 1];
      snakeLength++;
    }
  }
}

void drawFood() {
  midi1.sendNoteOn(gridToMIDINote(foodX, foodY), GREEN, 1, 1);
}

void  moveSnake() {
  for (int i = 9; i > 0; i--) {
    if (snakeX[i] != -1) {
      snakeX[i] = snakeX[i - 1];
      snakeY[i] = snakeY[i - 1];
    }
    //  Serial.print(snakeX[i]);
    //Serial.print(" , ");

  }
  //Serial.print(snakeX[0]);
  //  Serial.println();
  // Serial.println(score);

  snakeX[0] = snakeX[0] + moveX;
  if (snakeX[0] < 1) snakeX[0] = 8;
  if (snakeX[0] > 8) snakeX[0] = 1;
  snakeY[0] = snakeY[0] + moveY;
  if (snakeY[0] < 1) snakeY[0] = 8;
  if (snakeY[0] > 8) snakeY[0] = 1;
}

void  drawSnake() {
  for (int i = 9; i >= 0; i--) {
    if (snakeX[i] != -1) {
      midi1.sendNoteOn(gridToMIDINote(snakeX[i], snakeY[i]) , snakeColour[i], 1, 1);
    }
  }
}

//void eraseSnake() {
//  for (int i = 0; i < 10; i++) {
//    if (snakeX[i] != -1) {
//      midi1.sendNoteOn(gridToMIDINote(snakeX[i], snakeY[i]) , 42, 1, 1);
//    }
//  }
//}

//void lightHitNote() {
//  midi1.sendNoteOn(noteIndex, GREEN, 1, 1);
//}
//
//void sendToDisplay() {
//  //works on user setting on Launchpad
//  //void sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel, uint8_t cable=0)
//  midi1.sendNoteOn(note, note, 1, 1);
//  note++;
//  if (note > 99) note = 11;
//}

int noteIndexToX(int noteIndex) {
  //translate MIDI note to x position
  return noteIndex % 10;
}

int noteIndexToY(int noteIndex) {
  //translate MIDI note to y position
  return int(noteIndex / 10);
}

int gridToMIDINote(int x, int y) {
  return (y * 10 + x);
}

void OnNoteOn(byte channel, byte note, byte velocity)
{
  //which pad hit
  noteIndex = note;
  //translate to xy grid
  xPos = noteIndexToX(noteIndex);
  yPos = noteIndexToY(noteIndex);

  //sending to Launchpad
  midi1.sendNoteOn(xPos + 10, 20, 1, 1);
  midi1.sendNoteOn(yPos * 10 + 1, 20, 1, 1);
  midi1.sendNoteOn(noteIndex, 84, 1, 1);
  delay(100);
  midi1.sendNoteOn(xPos + 10, 0, 1, 1);
  midi1.sendNoteOn(yPos * 10 + 1, 0, 1, 1);
}

void OnNoteOff(byte channel, byte note, byte velocity)
{
  //not needed send 0 for off
}

void OnControlChange(byte channel, byte control, byte value)
{
  //Black Buttons
  //  Serial.print("Control Change, ch=");
  //  Serial.print(channel);
  //  Serial.print(", control=");
  //  Serial.print(control);
  //  Serial.print(", value=");
  //  Serial.print(value);
  //  Serial.println();

  //Launchpad upside down so arrow keys are accessable
  if (control == 91) {
    //Serial.println("down");
    if (moveY == 0) {
      moveX = 0;
      moveY = 1;
    } else {
      //sending error message to Launchpad
      midi1.sendNoteOn(91, 5, 1, 1);
      delay(10);
      midi1.sendNoteOn(91, 0, 1, 1);
    }
  }
  if (control == 92) {
    // Serial.println("up");
    if (moveY == 0) {
      moveX = 0;
      moveY = -1;
    } else {
      //sending error message to Launchpad
      midi1.sendNoteOn(92, 5, 1, 1);
      delay(10);
      midi1.sendNoteOn(92, 0, 1, 1);
    }
  }
  if (control == 93) {
    // Serial.println("right");
    if (moveX == 0) {
      moveX = -1;
      moveY = 0;
    } else {
      //sending error message to Launchpad
      midi1.sendNoteOn(93, 5, 1, 1);
      delay(10);
      midi1.sendNoteOn(93, 0, 1, 1);
    }
  }
  if (control == 94) {
    //Serial.println("left");
    if (moveX == 0) {
      moveX = 1;
      moveY = 0;
    } else {
      //sending error message to Launchpad
      midi1.sendNoteOn(94, 5, 1, 1);
      delay(10);
      midi1.sendNoteOn(94, 0, 1, 1);
    }
  }

  //reset snake position
  if (control == 19) {
    for (int i = 0; i < 10; i++) {
      snakeX[i] = -1;
      snakeY[i] = -1;
    }
    for (int i = 0; i < 5; i++) {
      snakeX[i] = i + 5;
      snakeY[i] = 4;
    }
  }

}

//Serial out 1 channel 1
void playMIDInote1(int command, int Note, int Velocity) {
  //sending to the tx pin on Serial 1
  Serial1.write(command);//send note on or note off command
  Serial1.write(Note);//send pitch data
  Serial1.write(Velocity);//send velocity data
}
