#include <Arduino.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SerialMP3Player.h>

#define TFT_CS   4
#define TFT_DC   3
#define TFT_RST  2
#define TFT_MOSI 5
#define TFT_SCK  12

#define MP3_RX A4
#define MP3_TX A5

#define LED_STRIP_PIN 13

Adafruit_ILI9341 lcd = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST);
Servo servo1; 
Servo servo2; 
SoftwareSerial bluetooth(10, 11);
SerialMP3Player mp3(MP3_RX, MP3_TX);
const int servo1Pin = 9;
const int servo2Pin = 8;
const int trigPin = 7;
const int echoPin = 6;
const float distanceThreshold = 10.0;

const int servo1OpenAngle = 180;
const int servo1CloseAngle = 0;
const int servo2OpenAngle = 180;
const int servo2CloseAngle = 0;

const int LDR_PIN = A0;
const int RED_PIN = A1;
const int GREEN_PIN = A2;
const int BLUE_PIN = A3;
const int LIGHT_THRESHOLD = 200;

const int COLORS[][3] = {
  {LOW, HIGH, HIGH},
  {HIGH, LOW, HIGH},
  {HIGH, HIGH, LOW},
};
const int FIXED_COLORS[][3] = {
  {LOW, HIGH, HIGH},
  {HIGH, LOW, HIGH},
  {HIGH, HIGH, LOW},
};
const int NUM_COLORS = sizeof(COLORS) / sizeof(COLORS[0]);
int currentColor = 0;
unsigned long previousMillis = 0;
const unsigned long colorInterval = 1000;
String rgbMode = "cycle";

String ledStripMode = "auto";

bool printMode = false;
String animationMode = "still";
String upperText = "AGAM'S";
String bottomText = "HOUSE";
int xPos = 0;
int yPos = 0;
int xDirection = 1;
int yDirection = 1;
const int screenWidth = 320;
const int screenHeight = 240;
const int textSpeed = 5;
int cursorY = 10;
static String lastAnimationMode = "";
static String lastUpperText = "";
static String lastBottomText = "";

void showColor(int redState, int greenState, int blueState);
void drawStillText();
void controlLedStrip(int lightValue);

void setup() {
  servo1.attach(servo1Pin);  
  servo2.attach(servo2Pin);  
  bluetooth.begin(9600);    
  Serial.begin(9600);      
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);  
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(LED_STRIP_PIN, OUTPUT);
  digitalWrite(LED_STRIP_PIN, LOW);
  mp3.begin(9600);
  delay(500);
  mp3.setVol(20);
  delay(100);
  servo1.write(servo1CloseAngle); 
  servo2.write(servo2CloseAngle); 
  showColor(HIGH, HIGH, HIGH); 
  lcd.begin();                    
  lcd.setRotation(3);
  lcd.fillScreen(ILI9341_BLACK);  
  drawStillText();
  Serial.println("Bluetooth Ready - Send 'open', 'close', 'isopen', number for MP3, 'animated', 'still', 'print', 'upper:<text>', 'bottom:<text>', 'on', 'off', 'auto', 'red', 'green', 'blue', 'cycle'");
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

bool isDoorClosed() {
  float distance = getDistance();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  if (printMode) {
    lcd.fillScreen(ILI9341_BLACK);
    cursorY = 10;
    lcd.setTextSize(2);
    lcd.setTextColor(ILI9341_GREEN);
    lcd.setCursor(10, cursorY);
    lcd.print("Distance: ");
    lcd.print(distance);
    lcd.println(" cm");
    cursorY += 20;
  }
  return distance <= distanceThreshold;
}

void showColor(int redState, int greenState, int blueState) {
  digitalWrite(RED_PIN, redState);
  digitalWrite(GREEN_PIN, greenState);
  digitalWrite(BLUE_PIN, blueState);
}

void controlLedStrip(int lightValue) {
  if (ledStripMode == "on") {
    digitalWrite(LED_STRIP_PIN, HIGH);
    Serial.println("LED Strip: On");
  } else if (ledStripMode == "off") {
    digitalWrite(LED_STRIP_PIN, LOW);
    Serial.println("LED Strip: Off");
  } else {
    if (lightValue > LIGHT_THRESHOLD) {
      digitalWrite(LED_STRIP_PIN, HIGH);
      Serial.println("LED Strip: On (auto, dark)");
    } else {
      digitalWrite(LED_STRIP_PIN, LOW);
      Serial.println("LED Strip: Off (auto, bright)");
    }
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
  Serial.println("Still mode: Text drawn centered");
}

void displayAnimation() {
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
  if (xPos >= screenWidth - upperTextWidth) {
    xDirection = -1;
    Serial.println("Hit right edge, xDirection changed to left");
  } else if (xPos <= 0) {
    xDirection = 1;
    Serial.println("Hit left edge, xDirection changed to right");
  }
  if (yPos >= screenHeight - 120) {
    yDirection = -1;
    Serial.println("Hit bottom edge, yDirection changed to up");
  } else if (yPos <= 0) {
    yDirection = 1;
    Serial.println("Hit top edge, yDirection changed to down");
  }
  Serial.print("xPos: ");
  Serial.print(xPos);
  Serial.print(" | yPos: ");
  Serial.print(yPos);
  Serial.print(" | xDirection: ");
  Serial.print(xDirection);
  Serial.print(" | yDirection: ");
  Serial.println(yDirection);
}

void displayAndPrint(String message, uint16_t color) {
  Serial.println(message);
  if (printMode) {
    lcd.setTextColor(color);
    lcd.setCursor(10, cursorY);
    lcd.println(message);
    cursorY += 20;
    if (cursorY > 280) {
      lcd.fillScreen(ILI9341_BLACK);
      cursorY = 10;
    }
  }
}

void loop() {
  if (!printMode) {
    if (animationMode == "still" && (animationMode != lastAnimationMode || upperText != lastUpperText || bottomText != lastBottomText)) {
      lcd.fillScreen(ILI9341_BLACK);
      drawStillText();
      lastAnimationMode = animationMode;
      lastUpperText = upperText;
      lastBottomText = bottomText;
    } else if (animationMode == "animated") {
      lcd.fillScreen(ILI9341_BLACK);
      displayAnimation();
      lastAnimationMode = animationMode;
      lastUpperText = upperText;
      lastBottomText = bottomText;
    }
  }
  if (bluetooth.available()) {  
    String command = bluetooth.readStringUntil('\n'); 
    command.trim();
    if (command.equalsIgnoreCase("isopen")) {
      bool doorClosed = isDoorClosed();
      String message = doorClosed ? "Door is closed" : "Door is open";
      displayAndPrint(message, ILI9341_CYAN);
    } else if (command.equalsIgnoreCase("on")) {
      ledStripMode = "on";
      digitalWrite(LED_STRIP_PIN, HIGH);
      Serial.println("LED Strip: On");
      if (printMode) {
        displayAndPrint("LED Strip: On", ILI9341_WHITE);
      }
    } else if (command.equalsIgnoreCase("off")) {
      ledStripMode = "off";
      digitalWrite(LED_STRIP_PIN, LOW);
      Serial.println("LED Strip: Off");
      if (printMode) {
        displayAndPrint("LED Strip: Off", ILI9341_WHITE);
      }
    } else if (command.equalsIgnoreCase("auto")) {
      ledStripMode = "auto";
      Serial.println("LED Strip: Auto mode");
      if (printMode) {
        displayAndPrint("LED Strip: Auto mode", ILI9341_WHITE);
      }
    } else if (command.equalsIgnoreCase("red")) {
      rgbMode = "red";
      Serial.println("RGB LED: Set to red");
      if (printMode) {
        displayAndPrint("RGB LED: Set to red", ILI9341_RED);
      }
    } else if (command.equalsIgnoreCase("green")) {
      rgbMode = "green";
      Serial.println("RGB LED: Set to green");
      if (printMode) {
        displayAndPrint("RGB LED: Set to green", ILI9341_GREEN);
      }
    } else if (command.equalsIgnoreCase("blue")) {
      rgbMode = "blue";
      Serial.println("RGB LED: Set to blue");
      if (printMode) {
        displayAndPrint("RGB LED: Set to blue", ILI9341_BLUE);
      }
    } else if (command.equalsIgnoreCase("cycle")) {
      rgbMode = "cycle";
      Serial.println("RGB LED: Set to cycle");
      if (printMode) {
        displayAndPrint("RGB LED: Set to cycle", ILI9341_YELLOW);
      }
    } else if (command.equalsIgnoreCase("animated")) {
      animationMode = "animated";
      printMode = false;
      Serial.println("Switched to animated mode");
      lcd.fillScreen(ILI9341_BLACK);
    } else if (command.equalsIgnoreCase("still")) {
      animationMode = "still";
      printMode = false;
      Serial.println("Switched to still mode");
      lcd.fillScreen(ILI9341_BLACK);
    } else if (command.equalsIgnoreCase("print")) {
      printMode = true;
      Serial.println("Switched to print mode");
      lcd.fillScreen(ILI9341_BLACK);
      cursorY = 10;
    } else if (command.startsWith("upper:")) {
      upperText = command.substring(6);
      Serial.println("Upper text set to: " + upperText);
      if (!printMode) {
        lcd.fillScreen(ILI9341_BLACK);
        if (animationMode == "still") {
          drawStillText();
        }
      }
      lastUpperText = upperText;
    } else if (command.startsWith("bottom:")) {
      bottomText = command.substring(7);
      Serial.println("Bottom text set to: " + bottomText);
      if (!printMode) {
        lcd.fillScreen(ILI9341_BLACK);
        if (animationMode == "still") {
          drawStillText();
        }
      }
      lastBottomText = bottomText;
    } else if (int trackNumber = command.toInt()) {
      if (trackNumber > 0 && command == String(trackNumber)) {
        mp3.play(trackNumber);
        String message = "Playing track " + String(trackNumber);
        displayAndPrint(message, ILI9341_MAGENTA);
      } else {
        displayAndPrint("Error: Invalid track number", ILI9341_RED);
      }
    } else {
      bool doorClosed = isDoorClosed();
      if (command.equalsIgnoreCase("open")) {
        if (doorClosed) {
          servo1.write(servo1OpenAngle); 
          servo2.write(servo2OpenAngle); 
          String message = "Servos to open (S1: " + String(servo1OpenAngle) + 
                          ", S2: " + String(servo2OpenAngle) + ")";
          displayAndPrint(message, ILI9341_GREEN);
        } else {
          displayAndPrint("Error: Door already open", ILI9341_RED);
        }
      } else if (command.equalsIgnoreCase("close")) {
        if (!doorClosed) {
          servo1.write(servo1CloseAngle); 
          servo2.write(servo2CloseAngle); 
          String message = "Servos to closed (S1: " + String(servo1CloseAngle) + 
                          ", S2: " + String(servo2CloseAngle) + ")";
          displayAndPrint(message, ILI9341_GREEN);
        } else {
          displayAndPrint("Error: Door already closed", ILI9341_RED);
        }
      } else {
        displayAndPrint("Error: Use 'open'/'close'/'isopen', number, 'animated'/'still'/'print', 'upper:<text>'/'bottom:<text>', 'on'/'off'/'auto', 'red'/'green'/'blue'/'cycle'", ILI9341_RED);
      }
    }
  }
  int lightValue = analogRead(LDR_PIN);
  String lightMessage = "Light: " + String(lightValue);
  displayAndPrint(lightMessage, ILI9341_CYAN);
  controlLedStrip(lightValue);
  if (lightValue > LIGHT_THRESHOLD) {
    unsigned long currentMillis = millis();
    if (rgbMode == "cycle") {
      if (currentMillis - previousMillis >= colorInterval) {
        previousMillis = currentMillis;
        showColor(
          COLORS[currentColor][0],
          COLORS[currentColor][1],
          COLORS[currentColor][2]
        );
        String colorMessage;
        switch (currentColor) {
          case 0: colorMessage = "RGB: Red"; break;
          case 1: colorMessage = "RGB: Green"; break;
          case 2: colorMessage = "RGB: Blue/Yellow"; break;
        }
        displayAndPrint(colorMessage, ILI9341_YELLOW);
        currentColor = (currentColor + 1) % NUM_COLORS;
      }
    } else if (rgbMode == "red") {
      showColor(FIXED_COLORS[0][0], FIXED_COLORS[0][1], FIXED_COLORS[0][2]);
      if (currentMillis - previousMillis >= colorInterval) {
        previousMillis = currentMillis;
        displayAndPrint("RGB: Red", ILI9341_YELLOW);
      }
    } else if (rgbMode == "green") {
      showColor(FIXED_COLORS[1][0], FIXED_COLORS[1][1], FIXED_COLORS[1][2]);
      if (currentMillis - previousMillis >= colorInterval) {
        previousMillis = currentMillis;
        displayAndPrint("RGB: Green", ILI9341_YELLOW);
      }
    } else if (rgbMode == "blue") {
      showColor(FIXED_COLORS[2][0], FIXED_COLORS[2][1], FIXED_COLORS[2][2]);
      if (currentMillis - previousMillis >= colorInterval) {
        previousMillis = currentMillis;
        displayAndPrint("RGB: Blue", ILI9341_YELLOW);
      }
    }
  } else {
    showColor(HIGH, HIGH, HIGH);
    displayAndPrint("RGB: Off", ILI9341_WHITE);
    currentColor = 0;
  }
  if (!printMode && animationMode == "animated") {
    delay(30);
  }
}
