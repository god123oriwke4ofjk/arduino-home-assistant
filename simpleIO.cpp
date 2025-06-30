#include <Arduino.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <SPI.h>             
#include <Adafruit_GFX.h>      
#include <Adafruit_ILI9341.h> 

#define TFT_CS   4   
#define TFT_DC   3   
#define TFT_RST  2    
#define TFT_MOSI 5    
#define TFT_SCK  12 

Adafruit_ILI9341 lcd = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST);

Servo servo1; 
Servo servo2; 
SoftwareSerial bluetooth(10, 11);
const int servo1Pin = 9;  
const int servo2Pin = 8;  
const int trigPin = 7;  
const int echoPin = 6;  
const float distanceThreshold = 30.0; 

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
const int NUM_COLORS = sizeof(COLORS) / sizeof(COLORS[0]);
int currentColor = 0;
unsigned long previousMillis = 0;
const unsigned long colorInterval = 1000;

int cursorY = 10; 

void showColor(int redState, int greenState, int blueState);

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
  servo1.write(servo1CloseAngle); 
  servo2.write(servo2CloseAngle); 
  showColor(HIGH, HIGH, HIGH); 
  lcd.begin();                    
  lcd.fillScreen(ILI9341_BLACK);  
  lcd.setTextSize(2);             
  lcd.setTextColor(ILI9341_GREEN); 
  lcd.setCursor(10, cursorY);     
  lcd.println("Bluetooth Ready");
  cursorY += 20;
  lcd.println("Send 'open'/'close'");
  cursorY += 20;
  Serial.println("Bluetooth Ready - Send 'open' or 'close'");
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
  lcd.fillScreen(ILI9341_BLACK); 
  cursorY = 10; 
  lcd.setCursor(10, cursorY);
  lcd.print("Distance: ");
  lcd.print(distance);
  lcd.println(" cm");
  cursorY += 20;
  return distance <= distanceThreshold; 
}

void showColor(int redState, int greenState, int blueState) {
  digitalWrite(RED_PIN, redState);
  digitalWrite(GREEN_PIN, greenState);
  digitalWrite(BLUE_PIN, blueState);
}

void displayAndPrint(String message, uint16_t color) {
  Serial.println(message);
  lcd.setTextColor(color);
  lcd.setCursor(10, cursorY);
  lcd.println(message);
  cursorY += 20;
  if (cursorY > 280) {
    lcd.fillScreen(ILI9341_BLACK);
    cursorY = 10;
  }
}

void loop() {
  if (bluetooth.available()) {  
    String command = bluetooth.readStringUntil('\n'); 
    command.trim(); 

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
    } 
    else if (command.equalsIgnoreCase("close")) {
      if (!doorClosed) {
        servo1.write(servo1CloseAngle); 
        servo2.write(servo2CloseAngle); 
        String message = "Servos to closed (S1: " + String(servo1CloseAngle) + 
                        ", S2: " + String(servo2CloseAngle) + ")";
        displayAndPrint(message, ILI9341_GREEN);
      } else {
        displayAndPrint("Error: Door already closed", ILI9341_RED);
      }
    } 
    else {
      displayAndPrint("Error: Use 'open'/'close'", ILI9341_RED);
    }
  }

  int lightValue = analogRead(LDR_PIN);
  String lightMessage = "Light: " + String(lightValue);
  displayAndPrint(lightMessage, ILI9341_CYAN);

  if (lightValue > LIGHT_THRESHOLD) {
    unsigned long currentMillis = millis();
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
  } else {
    showColor(HIGH, HIGH, HIGH);
    displayAndPrint("RGB: Off", ILI9341_WHITE);
    currentColor = 0; 
  }
}
