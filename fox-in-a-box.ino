/*
  Originally written by VE6BTC using IDE 1.60
  Updated Nov 2020 by VA6MCP using IDE 1.8.13
  Updated Oct 2021 by KD0TRD using IDE 1.8.13

  The purpose of this sketch is to use your Baofend UV-5R or another radio to transmit morse code for a fox hunt.
  Remember to turn your radio power down.

  KD0TRD updated to:
  1. Add puncuation characters to the morse code list
  2. Remove a limitation on the length of the message to be transmitted - not done yet
  3. Calculated the amount of time the message takes and subtracts that from the rest time. This allows it to transmit exactly on the rest time increments, like
     exactly every 5 minutes.

  Notes:
    Still have a limitation on the length of the morse code. Might have to create a list of morse code strings to iterate through.
 */


// Values to change
long rest = 5; // The time to wait in between transmissions (in minutes).
String Text = "PPRAA Fox Hunt Catch me if you can! See ya! XX0XXX"; // The message you want to transmit in morse code.
String AltText = "PPRAA Fox Hunt Fox found Thanks 4 playing! XX0XXX"; // An alternate message to transmit if pin 10 is held high on startup.
int numRepeats = 2; // Number of times to repeat the message per transmission.
bool transmitTone = 0; // Put a 1 to play the tone at the beginning of the transmission, or a 0 to not play the tone.
bool debug = 0; // Put a 1 to have the arduino print to the serial monitor for debugging, or a 0 for no printing.


// Values you can change, but shouldn't have to
#define tonehz 600        // The approximate frequency of the tones in hz, in reality it will be a tad lower, and include lots of harmonics.
#define dit 64            // The length of the dit in milliseconds. The dah and pauses are derived from this. The higher this number, the slower the morse code will be.
#define toneLength 2000   // The length of the optional beginning tone in milliseconds (every 1,000 is 1 second).
#define audio 5           // pin 5 on the board. Controls audio output. Connected to mic on radio.
#define tx 7              // pin 7 on the board. Controls the transmitting. Signal to 5v relay where you should have the speaker/mic pins from the radio connected.
#define altmsg 10         // pin 10 on the board. Switches to the alternate message on board startup. Helps to have a button to hold while you reset.
#define led 13            // pin 13 on the board. Connected to the onboard LED. Used to display a visual of the morse code being transmitted.


// Values you shouldn't change
bool alt = 0;
//int TextChars = 15;
//int CodeChars;
int duration;
int note = 1000000 / tonehz; // Converts the frequency into period
String morseCode = "";
void playTone(int noteDuration);
void playCode(String input);
String letters = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789?!.,-";
// Values:              SPC  A    B      C      D     E   F      G     H      I    J      K     L      M    N    O     P      Q      R     S     T   U     V      W     X      Y      Z      0       1       2       3       4       5       6       7       8       9       ?        !        .        ,        -
String morseValues[] = {"0","12","2111","2121","211","1","1121","221","1111","11","1222","212","1211","22","21","222","1221","2212","121","111","2","112","1112","122","2112","2122","2211","22222","12222","11222","11122","11112","11111","21111","22111","22211","22221","112211","212122","121212","221122","211112"};
// Values:                A  B    C    D   E F    G   H    I  J    K   L    M  N  O   P    Q    R   S   T U   V    W   X    Y    Z    0     1     2     3     4     5     6     7     8     9     ?      !      .      ,      -
//long morseValues[] = {0,12,2111,2121,211,1,1121,221,1111,11,1222,212,1211,22,21,222,1221,2212,121,111,2,112,1112,122,2112,2122,2211,22222,12222,11222,11122,11112,11111,21111,22111,22211,22221,112211,212122,121212,221122,211112};


// Function definition: setup
void setup() {
  // Inputs
  pinMode(altmsg, INPUT);

  // Outputs
  pinMode(audio, OUTPUT);
  pinMode(tx, OUTPUT);
  pinMode(led, OUTPUT);

  alt = digitalRead(altmsg); // Check pin 10 if it's high to switch messages
  String msg = "";
  if (alt) {
    msg = AltText;
  }
  else {
    msg = Text;
  }

  if (debug) {
    Serial.begin(9600);
  }
  // Translate the message into morse code
  Serial.print("Getting morse code for: ");
  msg.toUpperCase();
  Serial.println(msg);
  for (int i=0; i<msg.length(); i++) {
    char letter = msg[i];
    Serial.print(letter);
    Serial.print("[");
    int ind = letters.indexOf(letter);
    Serial.print(ind);
    Serial.print("] = ");
    Serial.println(morseValues[ind]);
    morseCode = morseCode + morseValues[ind];
    Serial.println(morseCode);
//    delay(250);
  }
  // Get the amount of time the message takes
  long mt = 0;
  long msgTime = 0;
  for (int i=0; i<morseCode.length(); i++) {
    if (morseCode[i] == '0') {
      mt = dit * 2;
    }
    else if (morseCode[i] == '1') {
      mt = dit;
    }
    else if (morseCode[i] == '2') {
      mt = dit * 3;
    }
    mt += dit;
    msgTime += mt;
  }
  msgTime *= numRepeats;
  if (transmitTone) {
    msgTime += toneLength;
  }
  Serial.print("Message: ");
  Serial.println(msg);
  Serial.print("Morse Code: ");
  Serial.println(morseCode);
  Serial.print("Time: ");
  Serial.println(msgTime);
  rest *= 60000;
  Serial.print("Rest: ");
  Serial.println(rest);
  rest -= msgTime;
  Serial.print("Rest - time: ");
  Serial.println(rest);
} // End of Function: setup


// Function definition: loop
void loop() {
  digitalWrite(tx, LOW); // Start transmitting
  digitalWrite(led, HIGH);
  delay(500); // Delay to let radio start transmitting
  if (transmitTone) {
    playTone(toneLength); // Optional tone at the beginning of each transmission
    delay(500); // Delay between tone and morse code message
  }
  playCode(morseCode);
  delay(500); // Delay to let message finish before end the transmission
  digitalWrite(tx, HIGH); // Stop transmitting
  delay(rest); // Delay between transmissions
} // End of Function: loop


// Function definition: playTone
void playTone(int noteDuration) {
  long elapsedTime = 0;
  long startTime = millis();
  if (noteDuration > 0) {
    digitalWrite(led, HIGH);
    while (elapsedTime < noteDuration) {
      digitalWrite(audio, HIGH);
      delayMicroseconds(note / 2);
      digitalWrite(audio, LOW);
      delayMicroseconds(note / 2);
      elapsedTime = millis() - startTime;
    }
    digitalWrite(led, LOW);
  }
  else { // If it's a pause this will run
    delay(dit * 2);
  }  
} // End of Function: playTone


// Function definition: playCode
void playCode(String input) {
  for (int n=0; n<numRepeats; n++) {
    for (int i=0; i<input.length(); i++) {
      Serial.print(input[i]);
      if (input[i] == '0') { // See if it's a pause
        duration = 0;
      }
      else if (input[i] == '1') { // See if it's a dit
        duration = dit;
      }
      else if (input[i] == '2') { // See if it's a dah
        duration = dit * 3;
      }
      playTone(duration);
      delay(dit); // Pause between sounds, otherwise each letter would be continuous.
    }
    Serial.println();
  }
}// End of Function: playCode
