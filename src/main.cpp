#include <Arduino.h>

#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#define DEBUG //TODO comment this line for producion mode

// Pins to talk to DFPlayer
#define RX_PIN 10
#define TX_PIN 11

// Buttons to switch tracks
#define NEXT_RECORD_PIN 12
#define PREV_RECORD_PIN 13

// Encoder pins
#define ENCODER_FW 14
#define ENCODER_BW 15

// Delay between start/stop of handle rotating and start/stop
// of music playing
#define PAUSE 150

// Sort of debug logging
#ifdef DEBUG
#define debug(msg) Serial.println(msg)
#else
#define debug(msg)
#endif

SoftwareSerial mySoftwareSerial(RX_PIN, TX_PIN);
DFRobotDFPlayerMini myDFPlayer;

int currentRecord = 1;
int recordsCount;
bool isPlaying = false;

int lastEncoderSignal = -1;

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
}

/**
 * Handle rotation of encoder (don't care about direction)
 **/
void encoderChanged()
{
  lastEncoderSignal = micros();
}

void setup()
{
  mySoftwareSerial.begin(9600);
#ifdef DEBUG
  Serial.begin(115200);
#endif

  pinMode(NEXT_RECORD_PIN, INPUT);
  pinMode(PREV_RECORD_PIN, INPUT);

  debug("Initializing DFPLayer");
  if (!myDFPlayer.begin(mySoftwareSerial))
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

  attachInterrupt(NEXT_RECORD_PIN, nextRecord, HIGH);
  attachInterrupt(PREV_RECORD_PIN, prevRecord, HIGH);

  attachInterrupt(ENCODER_FW, encoderChanged, CHANGE);
  attachInterrupt(ENCODER_BW, encoderChanged, CHANGE);
}

void loop()
{

  if (micros() - lastEncoderSignal > PAUSE)
  {
    if (isPlaying)
    {
      isPlaying = false;
      myDFPlayer.pause();
    }
  }
  else
  {
    if (!isPlaying)
    {
      isPlaying = true;
      myDFPlayer.play(currentRecord);
    }
  }

  delay(20);
}