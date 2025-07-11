#include <stddef.h>
#include <Arduino.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SerialMP3Player.h>

Servo servo1; 
Servo servo2; 
SoftwareSerial bluetooth(10, 11);
const int servoPin1 = 8;
const int servoPin2 = 9;
int currentPos = 0; 
int targetPos = 0;

#define MP3_RX A4
#define MP3_TX A5
SerialMP3Player mp3(MP3_RX, MP3_TX);
int currentVolume = 20; 

#define TFT_CS   4
#define TFT_DC   3
#define TFT_RST  2
#define TFT_MOSI 5
#define TFT_SCK  12
Adafruit_ILI9341 lcd = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST);

String animationMode = "animated";
String upperText = "AGAM'S";
String bottomText = "HOUSE";
int xPos = 0;
int yPos = 0;
int xDirection = 1;
int yDirection = 1;
const int screenWidth = 320;
const int screenHeight = 240;
const int textSpeed = 17;

const int LDR_PIN = A0;
const int RED_PIN = A1;
const int GREEN_PIN = A2;
const int BLUE_PIN = A3;
const int LED_STRIP_PIN = 13;
const int LIGHT_THRESHOLD = 200;
const int COLORS[][3] = {
  {LOW, HIGH, HIGH},
  {HIGH, LOW, HIGH}, 
  {HIGH, HIGH, LOW},
};
const int NUM_COLORS = sizeof(COLORS) / sizeof(COLORS[0]);
int currentColor = 0;
unsigned long previousMillis = 0;
const unsigned long colorInterval = 1000;
String ledStripMode = "auto";
String rgbMode = "cycle";

const int trigPin = 7; 
const int echoPin = 6; 
const float DETECTION_DISTANCE = 10.0; 
unsigned long lastUltrasonicCheck = 0;
const unsigned long ULTRASONIC_INTERVAL = 500; 

void printToSerialAndTFT(String message) {
  Serial.println(message);
  if (animationMode == "print") {
    lcd.fillScreen(ILI9341_BLACK);
    lcd.setCursor(0, 0);
    lcd.setTextSize(2);
    lcd.setTextColor(ILI9341_WHITE);
    lcd.println(message);
  }
}

void drawStillText() {
  int upperTextWidth = upperText.length() * 30;
  int bottomTextWidth = bottomText.length() * 24;
  int textWidth = max(upperTextWidth, bottomTextWidth);
  int xCenter = (screenWidth - textWidth) / 2;
  int yCenter = (screenHeight - 120) / 2;

  lcd.setCursor(xCenter, yCenter);
  lcd.setTextSize(5);
  for (int i = 0; i < (int)upperText.length(); i++) {
    lcd.setTextColor(i % 2 == 0 ? ILI9341_WHITE : ILI9341_CYAN);
    lcd.print(upperText[i]);
  }
  lcd.setCursor(xCenter, yCenter + 80);
  lcd.setTextSize(4);
  lcd.setTextColor(ILI9341_WHITE);
  lcd.print(bottomText);
  printToSerialAndTFT("Still mode: Text drawn centered");
}

void moveServosSmoothly(int from, int to) {
  int step = (from < to) ? 1 : -1;
  for (int pos = from; pos != to; pos += step) {
    servo1.write(pos);
    servo2.write(180 - pos);
    delay(10);
  }
  servo1.write(to);
  servo2.write(180 - to);
  currentPos = to;
}

void showColor(int redState, int greenState, int blueState) {
  digitalWrite(RED_PIN, redState);
  digitalWrite(GREEN_PIN, greenState);
  digitalWrite(BLUE_PIN, blueState);
}

float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2;
  return distance;
}

bool checkMP3Status() {
  mp3.qStatus(); 
  delay(100);
  return true; 
}

void playTrack(int track) {
  if (checkMP3Status()) {
    mp3.stop(); 
    delay(100); 
    mp3.play(track);
    printToSerialAndTFT("Playing track " + String(track));
  } else {
    printToSerialAndTFT("Error: MP3 module not responding");
  }
}

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);
  
  mp3.begin(9600);
  delay(500);
  mp3.setVol(currentVolume);
  if (checkMP3Status()) {
    printToSerialAndTFT("MP3 module initialized successfully");
  } else {
    printToSerialAndTFT("Error: MP3 module initialization failed");
  }
  
  servo1.attach(servoPin1);
  servo2.attach(servoPin2);
  servo1.write(0);
  servo2.write(180);
  currentPos = 0;

  lcd.begin();
  lcd.setRotation(3);
  lcd.fillScreen(ILI9341_BLACK);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(LED_STRIP_PIN, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  showColor(HIGH, HIGH, HIGH);
  digitalWrite(LED_STRIP_PIN, LOW);

  drawStillText();
  printToSerialAndTFT("Bluetooth Ready - Send commands");
}

void loop() {
  static String btCommand = ""; 
  bluetooth.listen(); 

  while (bluetooth.available()) {
    char c = bluetooth.read();
    Serial.print("BT> "); Serial.println(c);
    
    if (c == '\r' || c == '\n') {
      if (btCommand.length() > 0) {
        btCommand.trim();
        printToSerialAndTFT("Received: " + btCommand);

        if (btCommand == "open") {
          targetPos = 0;
          printToSerialAndTFT("Opening...");
          moveServosSmoothly(currentPos, targetPos);
          delay(500);
          playTrack(1);
        } else if (btCommand == "close") {
          targetPos = 180;
          printToSerialAndTFT("Closing...");
          moveServosSmoothly(currentPos, targetPos);
          delay(500); 
          playTrack(3); 
        } else if (btCommand == "still") {
          animationMode = "still";
        } else if (btCommand == "animate") {
          animationMode = "animated";
        } else if (btCommand == "print") {
          animationMode = "print";
        } else if (btCommand == "on") {
          ledStripMode = "on";
          digitalWrite(LED_STRIP_PIN, HIGH);
        } else if (btCommand == "off") {
          ledStripMode = "off";
          digitalWrite(LED_STRIP_PIN, LOW);
          showColor(HIGH, HIGH, HIGH);
        } else if (btCommand == "auto") {
          ledStripMode = "auto";
        } else if (btCommand == "red") {
          rgbMode = "red";
        } else if (btCommand == "green") {
          rgbMode = "green";
        } else if (btCommand == "blue") {
          rgbMode = "blue";
        } else if (btCommand == "cycle") {
          rgbMode = "cycle";
        } else if (btCommand == "play") {
          playTrack(1);
        } else if (btCommand == "stop") {
          mp3.stop();
          printToSerialAndTFT("Stopped playback");
        } else if (btCommand.startsWith("vol")) {
          String volStr = btCommand.substring(3);
          if (volStr.length() > 0 && (volStr.toInt() > 0 || volStr == "0")) {
            int vol = volStr.toInt();
            if (vol >= 0 && vol <= 30) {
              currentVolume = vol;
              mp3.setVol(currentVolume);
              printToSerialAndTFT("Volume set to " + String(currentVolume));
            } else {
              printToSerialAndTFT("Invalid volume: " + volStr + " (must be 0-30)");
            }
          } else {
            printToSerialAndTFT("Invalid volume command: " + btCommand);
          }
        } else if (btCommand.startsWith("+")) {
          String volStr = btCommand.substring(1);
          if (volStr.length() > 0 && volStr.toInt() > 0) {
            int volChange = volStr.toInt();
            currentVolume += volChange;
            if (currentVolume > 30) currentVolume = 30;
            if (currentVolume < 0) currentVolume = 0;
            mp3.setVol(currentVolume);
            printToSerialAndTFT("Volume increased to " + String(currentVolume));
          } else {
            printToSerialAndTFT("Invalid volume increase command: " + btCommand);
          }
        } else if (btCommand.startsWith("-")) {
          String volStr = btCommand.substring(1);
          if (volStr.length() > 0 && volStr.toInt() > 0) {
            int volChange = volStr.toInt();
            currentVolume -= volChange;
            if (currentVolume > 30) currentVolume = 30;
            if (currentVolume < 0) currentVolume = 0;
            mp3.setVol(currentVolume);
            printToSerialAndTFT("Volume decreased to " + String(currentVolume));
          } else {
            printToSerialAndTFT("Invalid volume decrease command: " + btCommand);
          }
        } else if (btCommand.toInt() > 0) {
          int track = btCommand.toInt();
          playTrack(track);
        } else {
          printToSerialAndTFT("Unknown command: " + btCommand);
        }

        btCommand = ""; 
      }
    } else {
      btCommand += c; 
    }
  }

  unsigned long currentMillis = millis();
  if (currentMillis - lastUltrasonicCheck >= ULTRASONIC_INTERVAL) {
    float distance = getDistance();
    if (distance > 0 && distance <= DETECTION_DISTANCE) {
      printToSerialAndTFT("Movement detected at " + String(distance) + " cm, playing track 2");
      playTrack(2);
    }
    lastUltrasonicCheck = currentMillis;
  }

  int lightValue = analogRead(LDR_PIN);
  bool ledStripState = digitalRead(LED_STRIP_PIN);

  if (ledStripMode == "auto") {
    if (lightValue > LIGHT_THRESHOLD && !ledStripState) {
      digitalWrite(LED_STRIP_PIN, HIGH);
    } else if (lightValue <= LIGHT_THRESHOLD && ledStripState) {
      digitalWrite(LED_STRIP_PIN, LOW);
      showColor(HIGH, HIGH, HIGH);
    }
  }

  if (ledStripState) {
    if (rgbMode == "red") showColor(LOW, HIGH, HIGH);
    else if (rgbMode == "green") showColor(HIGH, LOW, HIGH);
    else if (rgbMode == "blue") showColor(HIGH, HIGH, LOW);
    else if (rgbMode == "cycle") {
      if (currentMillis - previousMillis >= colorInterval) {
        previousMillis = currentMillis;
        showColor(COLORS[currentColor][0], COLORS[currentColor][1], COLORS[currentColor][2]);
        currentColor = (currentColor + 1) % NUM_COLORS;
      }
    }
  } else {
    showColor(HIGH, HIGH, HIGH);
    currentColor = 0;
  }

  if (animationMode == "still") {
    drawStillText();
  } else if (animationMode == "animated") {
    lcd.fillScreen(ILI9341_BLACK);
    lcd.setCursor(xPos, yPos);
    lcd.setTextSize(5);
    for (int i = 0; i < (int)upperText.length(); i++) {
      lcd.setTextColor(i % 2 == 0 ? ILI9341_WHITE : ILI9341_CYAN);
      lcd.print(upperText[i]);
    }
    lcd.setCursor(xPos, yPos + 80);
    lcd.setTextSize(4);
    lcd.setTextColor(ILI9341_WHITE);
    lcd.print(bottomText);

    xPos += xDirection * textSpeed;
    yPos += yDirection * textSpeed;

    int upperTextWidth = upperText.length() * 30;
    if (xPos >= screenWidth - upperTextWidth || xPos <= 0) xDirection *= -1;
    if (yPos >= screenHeight - 120 || yPos <= 0) yDirection *= -1;
  }

  delay(30); 
}
