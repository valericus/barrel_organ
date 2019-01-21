#include <Arduino.h>

#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#define DEBUG //TODO comment this line for producion mode

// Pins to talk to DFPlayer
#define RX_PIN 8
#define TX_PIN 7

// Buttons to switch tracks
#define NEXT_RECORD_PIN 3
#define PREV_RECORD_PIN 13

// Encoder pins
#define ENCODER_FW 2
#define ENCODER_BW 15

// Delay between start/stop of handle rotating and start/stop
// of music playing (milliseconds)
#define PAUSE 300

// Sort of debug logging
#ifdef DEBUG
#define debug(msg) Serial.println(msg)
#else
#define debug(msg)
#endif

SoftwareSerial dfplayerSerial(RX_PIN, TX_PIN);
DFRobotDFPlayerMini myDFPlayer;

volatile int currentRecord = 1; // DFPlayer starts track numbers from 1
int recordsCount;
bool isPlaying = false;

volatile unsigned long lastEncoderSignal = 0;

/**
 * Handle next record button
 **/
void nextRecord()
{
  if (currentRecord < recordsCount - 1)
  {
    currentRecord++;
  }
  else
  {
    currentRecord = 1;
  }
  debug("Switched to next track " + String(currentRecord));
}

/**
 * Handle previous record button
 **/
void prevRecord()
{
  if (currentRecord > 1)
  {
    currentRecord--;
  }
  else
  {
    currentRecord = recordsCount + 1;
  }
  debug("Switched to prev track " + String(currentRecord));
}

/**
 * Handle rotation of encoder (don't care about direction)
 **/
void encoderChanged()
{
  lastEncoderSignal = micros();
  debug("Last encoder signal registered on " + String(lastEncoderSignal));
}

void setup()
{
  dfplayerSerial.begin(9600);
#ifdef DEBUG
  Serial.begin(9600);
#endif

  pinMode(NEXT_RECORD_PIN, INPUT);
  pinMode(PREV_RECORD_PIN, INPUT);

  debug("Initializing DFPLayer");
  if (!myDFPlayer.begin(dfplayerSerial))
  {
    debug("Unable to begin:");
    debug("1.Please recheck the connection!");
    debug("2.Please insert the SD card!");
    while (true)
    {
      delay(1000);
    };
  }
  else
  {
    debug("DFPlayer Mini online.");
  }

  myDFPlayer.volume(10); //TODO set volume via variable resistor
  recordsCount = myDFPlayer.readFileCounts();
  debug("Found " + String(recordsCount) + " records on SD card");

  attachInterrupt(digitalPinToInterrupt(NEXT_RECORD_PIN), nextRecord, RISING);
  //attachInterrupt(PREV_RECORD_PIN, prevRecord, HIGH);

  attachInterrupt(digitalPinToInterrupt(ENCODER_FW), encoderChanged, CHANGE);
  //attachInterrupt(ENCODER_BW, encoderChanged, CHANGE);
}

unsigned long currentMillis;
unsigned long pause;

void loop()
{
 currentMillis = millis();
  pause = currentMillis - lastEncoderSignal;
  debug("Current millis " + String(currentMillis) + ", last encoder signal at " + String(lastEncoderSignal) + ", pause is " + String(pause));
  if (pause > PAUSE)
  {
    if (isPlaying)
    {
      debug("Disbling music after pause " + String(pause));
      isPlaying = false;
      myDFPlayer.pause();
    }
  }
  else
  {
    if (!isPlaying)
    {
      debug("Enabling music after pause " + String(pause));
      isPlaying = true;
      myDFPlayer.play(currentRecord);
    }
  }

  delay(200);
}