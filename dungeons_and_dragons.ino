#include "funshield.h"


/*

Simulation of dice used in a table tob game called Dungeons&Dragons - Final project for Computer Systems class (NSWI170)

!!!Animation used for number generation is accompanied with sound!!!

*/

//!--------!
// class for generating random numbers, utilizes micros for random seed
class RandomNumberGenerator {
private:
  static constexpr int displayThreshold = 10000;  // upper bound for random number used as filler

  unsigned long generateBySeed(unsigned long randSeed) {  // generate random number based of given seed - usually micros of putton hold
    randomSeed(randSeed);
    return random();
  }

public:
  int getSequenceSum(int throwAmount, int diceType, unsigned long seed) {  // returns sum of all throws with chosen dice type
    int sum = 0;
    unsigned long generatingSeed = seed;
    for (int i = 0; i < throwAmount; i++) {
      generatingSeed = generateBySeed(generatingSeed);  // use as next seed
      sum += (generatingSeed % diceType) + 1;
    }
    return sum;
  }

  int getRandomNumber() {  // used for matrix effect
    return generateBySeed(micros()) % displayThreshold;
  }
};

RandomNumberGenerator RNG;


//!--------!
// main timer class for measuring display delays
class DisplayTimer {
private:
  int refreshRate = 50;     // refresh rate of matrix effect - every 50ms new number is generated and shown
  unsigned long ltime = 0;  // low time

public:
  bool shouldRegenerate() {  // check if new number has to be generated
    auto ctime = millis();   // current time

    if (ctime >= ltime + refreshRate) {
      ltime += refreshRate;
      return true;
    }
    return false;
  }
};

DisplayTimer displayTimer;


//!--------!
// class handling all segment display interactions, supports displaying digits and correctly setting up display
class SegmentDisplay {
public:
  SegmentDisplay() {}

  void setup() {  // correctly set all pins to output
    pinMode(latch_pin, OUTPUT);
    pinMode(clock_pin, OUTPUT);
    pinMode(data_pin, OUTPUT);

    writeGlyphBitmask(0xFF, 0xFF);  // clear
  }

  void writeGlyphBitmask(byte glyph, byte pos_bitmask) {
    digitalWrite(latch_pin, LOW);
    shiftOut(data_pin, clock_pin, MSBFIRST, glyph);
    shiftOut(data_pin, clock_pin, MSBFIRST, pos_bitmask);
    digitalWrite(latch_pin, HIGH);
  }

  void glyphFromRight(int i, int indCharacter) {
    int index = 3 - indCharacter;  // adjust to count from right
    writeGlyphBitmask(digits[i], digit_muxpos[index]);
  }

  void displayDigit(int toWrite, int dest = -1) {  // display digit at given destination
    glyphFromRight(toWrite, dest);
  }

  void displayChar(byte charToDisplay, int indCharacter) {  // display char at given position - used for 'd' character
    int index = 3 - indCharacter;                           // adjust to count from right
    writeGlyphBitmask(charToDisplay, digit_muxpos[index]);
  }
};


SegmentDisplay segDisplay;


//!--------!
// base class for button, support press detection and custom get methods
class Button {
private:
  static constexpr int debounceThreshold = 150;  // time in ms required for software debounce - not fully functional!


  unsigned long lastPressTime = 0;  // time of last registered press in ms
  int pin;                          // attached button pin

  bool state = OFF;  // current button state - signalizing pressed state

public:
  Button(int _pin) {
    pin = _pin;
  }

  void setup() {
    pinMode(getPin(), INPUT);
  }

  bool getCurrState() {  // check if button is pressed, returns true if btn is pressed
    return !digitalRead(pin);
  }

  int getPin() {
    return pin;
  }

  bool isNewlyPressed() {  // detect new press, do not detect continuous press
    auto ctime = millis();
    bool currState = getCurrState();
    bool newlyPressed = false;

    if (ctime - lastPressTime > debounceThreshold) {  // avoid double press
      if (currState && !state) {                      // check if button states match
        state = true;
        newlyPressed = true;
        lastPressTime = ctime;
      } else if (!currState) {
        state = false;
      }
    }

    return newlyPressed;
  }

  bool isNewlyReleased() {  // check if button was released, used for showing generated number
    bool currState = getCurrState();
    bool newlyReleased = false;

    if ((currState == false) & (state == true)) {
      newlyReleased = true;
    }

    return newlyReleased;
  }
};

Button buttons[] = { Button(button1_pin), Button(button2_pin), Button(button3_pin) };  // currently used buttons


//!--------!
// class handling all sounds
class SoundManager {
private:
  static constexpr int beepInterval = 40;  // in ms, beep and pause length

  bool play = false;           // beep should currently play
  unsigned long lastBeep = 0;  // in ms, time of last beep

public:
  void manageSound() {
    auto ctime = millis();
    if (ctime >= lastBeep + beepInterval) {  // change sound state
      lastBeep = ctime;
      play = !play;
    }

    play ? turnBeeperOn() : turnBeeperOff();  // if play is true, produce sound
  }

  void turnBeeperOn() {
    digitalWrite(beep_pin, LOW);
  }

  void turnBeeperOff() {
    digitalWrite(beep_pin, HIGH);
  }
};

SoundManager soundManager;


//!--------!
// main class used for dice simulation, handles dice type and throw pick along with generation and interactions
class Dice {
private:
  static constexpr byte letterD = 0b10100001;  // byte code for letter 'd'
  static constexpr int characterCount = 4;     // segment display character count
  static constexpr int loopThreshold = 10000;  // upper bound digit order
  static constexpr int throwThreshold = 10;    // upper bound throw count
  static constexpr int typeAmount = 7;         // amount of dice types - d4, d6, d8, d10, d12, d20, d100
  static constexpr int maxType = 100;          // highest currently supported dice types - if chosen display only 00


  int diceTypes[typeAmount] = { 4, 6, 8, 10, 12, 20, 100 };  // all possible dice types - numbers indicate amount of dice sides
  int modes[2] = { 0, 1 };                                   // current modes, 0 = normal mode, 1 = configuration mode

  bool calculating = false;  // in normal mode, new number is being generated

  int currentValue = 0;  // generated val
  int throws = 1;        // amount of throws

  int currentMode = 1;
  int typeCounter = 0;                       // keeps track of type index in typeAmount
  int currentType = diceTypes[typeCounter];  // amount of sides

  int digitOrder = 1;  // value digit order
  int targetChar = 0;  // display tagret character

  int matrixNumber = RNG.getRandomNumber();  // number shown while calculating is true - number being generated


  void hopCharacter() {  // cycle through display characters and generated number digit orders
    targetChar = (1 + targetChar) % characterCount;
    digitOrder *= 10;
    if (digitOrder == loopThreshold) {  // check if digit order is out of bounds
      digitOrder = 1;
    }
  }

  int getDigit(int number, int digitOrder) {  // return current digit value at given digit order
    return (number / digitOrder) % 10;
  }

public:
  void computeNewValue(unsigned long seed) {  // gets new random value based on passed seed
    currentValue = RNG.getSequenceSum(throws, currentType, seed);
    calculating = false;
  }

  bool isCalculating() {
    return calculating;
  }

  void setNormalMode() {  // sets dice into normal mode - generation or generated number shown
    digitalWrite(beep_pin, HIGH);
    currentMode = modes[0];
    calculating = true;
  }

  void setConfigMode() {  // sets dice to config mode - allows to change dice type and throws
    currentMode = modes[1];
  }

  void increaseThrows() {
    throws += 1;
    if (throws == throwThreshold) {
      throws = 1;
    }
  }

  void increaseType() {  // increase number of dice sides
    typeCounter = (1 + typeCounter) % typeAmount;
    currentType = diceTypes[typeCounter];
  }

  void displayNumber(int val) {  // show randomly generated number
    int toDisplay = getDigit(val, digitOrder);
    segDisplay.displayDigit(toDisplay, targetChar);
  }

  void displayConfigurationMode() {  // displays currently configurated dice in format of: throws-'d'-sides (e.g. 5d12)
    soundManager.turnBeeperOff();

    switch (targetChar) {
      case 0:
        segDisplay.displayDigit(currentType % 10, targetChar);
        break;
      case 1:
        if (currentType == maxType) {
          segDisplay.displayDigit(0, targetChar);
        } else {
          segDisplay.displayDigit(currentType / 10, targetChar);
        }
        break;
      case 2:
        segDisplay.displayChar(letterD, targetChar);
        break;
      case 3:
        segDisplay.displayDigit(throws, targetChar);  // display amount of trows with chosen dice
        break;
    }
  }

  void matrixAnimation() {  // animation shown whilst generating new random number
    int digitToDisplay = getDigit(matrixNumber, digitOrder);
    segDisplay.displayDigit(digitToDisplay, targetChar);

    if (displayTimer.shouldRegenerate()) {  // show new number
      matrixNumber = RNG.getRandomNumber();
    }
  }


  void handleModes() {  // handles current dice mode - change displayed content and sound
    switch (currentMode) {
      case 1:
        displayConfigurationMode();
        break;
      case 0:
        if (calculating) {
          soundManager.manageSound();
          matrixAnimation();  // show random sequence of numbers
        } else {
          soundManager.turnBeeperOff();
          displayNumber(currentValue);
        }
        break;
    }
    hopCharacter();
  }

  void loop() {
    handleModes();
  }
};

Dice dice;


//!--------!
// class handling all buttons and managing their interaction with dice
class ButtonStateMachine {
private:
  static constexpr int btnCount = 3;  // amount of used buttons

  unsigned long lastPress = 0;  // last time of generating button being pressed

  void generateResult() {  // generate new random number based on duration of button hold
    auto ctime = micros();
    unsigned long timeDiff = ctime - lastPress;
    dice.computeNewValue(timeDiff);
  }

public:
  void setupButtons() {
    for (int i = 0; i < btnCount; i++) {
      buttons[i].setup();
    }
  }

  void loopButtons() {  // loops through all buttons and checks for their respective actions
    for (int i = 0; i < btnCount; i++) {
      if ((i == 0) & (buttons[i].isNewlyReleased())) {
        generateResult();
      }

      if (buttons[i].isNewlyPressed()) {
        stateMachine(buttons[i].getPin());
      }
    }
  }

  void stateMachine(int buttonPin) {  // match button interaction with dice and sound when pressed
    auto ctime = micros();
    switch (buttonPin) {
      case button1_pin:
        dice.setNormalMode();
        lastPress = ctime;
        break;
      case button2_pin:
        if (!dice.isCalculating()) {
          dice.setConfigMode();
          dice.increaseThrows();
        };
        break;
      case button3_pin:
        if (!dice.isCalculating()) {
          dice.setConfigMode();
          dice.increaseType();
        };
        break;
    }
  }
};

ButtonStateMachine stateMachine;


void setup() {
  pinMode(beep_pin, OUTPUT);
  soundManager.turnBeeperOff();

  segDisplay.setup();
  stateMachine.setupButtons();

  //Serial.begin(9600);
}

void loop() {
  stateMachine.loopButtons();
  dice.loop();
}