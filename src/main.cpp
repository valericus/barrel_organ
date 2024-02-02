#include <Arduino.h>

#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// TODO закомментировать эту строку перед заливкой на
// «боевую» сборку
#define DEBUG

// Светодиод успешного запуска
#define HEALTH_CHECK_LED 12

// Пины для общения с DFPlayer
#define RX_PIN 8 // к пину TX на DFP
#define TX_PIN 7 // к пину RX на DFP

// Кнопки переключения записей
#define NEXT_RECORD_PIN 2  // следующая
#define PREV_RECORD_PIN 13 // предыдущая

// Пины энкодера (достаточно одного)
#define ENCODER_FW 3
#define ENCODER_BW 15

// Задержка между началом/концом вращения ручки
// и началом/концом воспроизведения (миллисекунды)
#define PAUSE 300

// Отладочное логирование
#ifdef DEBUG
#define debug(msg) Serial.println(msg)
#else
#define debug(msg)
#endif

SoftwareSerial dfplayerSerial(RX_PIN, TX_PIN);
DFRobotDFPlayerMini myDFPlayer;

volatile int currentRecord = 1; // Кажется, DFPlayer начинает нумерацию записей с 1
int recordsCount;
bool isPlaying = false;

volatile unsigned long lastEncoderSignal = 0;

/**
 * Обработчик на кнопку следующей записи
 **/
void nextRecord() {
  if (currentRecord < recordsCount - 1) {
    currentRecord++;
  } else {
    currentRecord = 0;
  }
  debug("Switched to next track " + String(currentRecord));
}

/**
 * Обработчик на кнопку предыдущей записи
 **/
void prevRecord() {
  if (currentRecord > 0) {
    currentRecord--;
  } else {
    currentRecord = recordsCount + 1;
  }
  debug("Switched to prev track " + String(currentRecord));
}

/**
 * Обработчик вращения энкодера (не учитывает направление)
 **/
void encoderChanged() {
  lastEncoderSignal = millis();
}

void setup() {
  dfplayerSerial.begin(9600);
#ifdef DEBUG
  Serial.begin(9600);
#endif

  pinMode(NEXT_RECORD_PIN, INPUT);
  pinMode(PREV_RECORD_PIN, INPUT);

  pinMode(HEALTH_CHECK_LED, OUTPUT);

  debug("Initializing DFPLayer");
  if (!myDFPlayer.begin(dfplayerSerial)) {
    debug("Unable to begin:");
    debug("1.Please recheck the connection!");
    debug("2.Please insert the SD card!");
    while (true) {
      digitalWrite(HEALTH_CHECK_LED, HIGH);
      delay(200);
      digitalWrite(HEALTH_CHECK_LED, LOW);
      delay(1000);
    };
  } else {
    debug("DFPlayer Mini online.");
    digitalWrite(HEALTH_CHECK_LED, HIGH);
  }

  myDFPlayer.volume(30); //TODO сделать настройку громкости через переменный резистор
  recordsCount = myDFPlayer.readFileCounts();
  debug("Found " + String(recordsCount) + " records on SD card");

  attachInterrupt(digitalPinToInterrupt(NEXT_RECORD_PIN), nextRecord, RISING);
  //attachInterrupt(PREV_RECORD_PIN, prevRecord, HIGH);

  attachInterrupt(digitalPinToInterrupt(ENCODER_FW), encoderChanged, CHANGE);
  //attachInterrupt(ENCODER_BW, encoderChanged, CHANGE);
}

unsigned long pause;

void loop() {
  // TODO учесть момент переполнения счётчика миллисекунд
  pause = millis() - lastEncoderSignal;
  if (pause > PAUSE) {
    if (isPlaying) {
      debug("Disabling music after pause " + String(pause));
      isPlaying = false;
      myDFPlayer.pause();
    }
  } else {
    if (!isPlaying) {
      debug("Enabling music after pause " + String(pause));
      isPlaying = true;
      myDFPlayer.play(currentRecord);
    }
  }

  delay(200);
}