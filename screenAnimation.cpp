#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_CS   4   
#define TFT_DC   3   
#define TFT_RST  2    
#define TFT_MOSI 5    
#define TFT_SCK  12  

Adafruit_ILI9341 lcd = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST);

int xPos = 0;              
int direction = 1;        
const int screenWidth = 320;  
const int screenHeight = 240; 
const int textSpeed = 17;    

void setup() {
  Serial.begin(9600);         
  lcd.begin();               
  lcd.setRotation(3);         
  lcd.fillScreen(ILI9341_BLACK);
}

void loop() {
  lcd.fillScreen(ILI9341_BLACK);

  lcd.setCursor(xPos, 50);     
  lcd.setTextSize(5);            
  
  lcd.setTextColor(ILI9341_WHITE);
  lcd.print("A");
  lcd.setTextColor(ILI9341_CYAN);
  lcd.print("G");
  lcd.setTextColor(ILI9341_WHITE);
  lcd.print("A");
  lcd.setTextColor(ILI9341_CYAN);
  lcd.print("M");
  lcd.setTextColor(ILI9341_WHITE);
  lcd.print("'S");

  lcd.setCursor(xPos, 130);    
  lcd.setTextSize(4);          
  lcd.setTextColor(ILI9341_WHITE);
  lcd.print("HOUSE");

  xPos += direction * textSpeed;

  if (xPos >= screenWidth - 180) {  
    direction = -1;
    Serial.println("Direction changed to left");
  } else if (xPos <= 0) {
    direction = 1;
    Serial.println("Direction changed to right");
  }

  Serial.print("xPos: ");
  Serial.print(xPos);
  Serial.print(" | Direction: ");
  Serial.println(direction);

  delay(30);  
}
