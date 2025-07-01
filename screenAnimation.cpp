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

String animationMode = "animated";
String upperText = "AGAM'S";
String bottomText = "HOUSE";
int xPos = 0;
int yPos = 0;
int xDirection = 1;
int yDirection = 1;
const int screenWidth = 320;
const int screenHeight = 240;
const int textSpeed = 5;

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

void setup() {
  Serial.begin(9600);
  lcd.begin();
  lcd.setRotation(3);
  lcd.fillScreen(ILI9341_BLACK);

  if (animationMode == "still") {
    drawStillText();
  }
}

void loop() {
  static String lastAnimationMode = "";
  static String lastUpperText = "";
  static String lastBottomText = "";

  if (animationMode == "still" && (animationMode != lastAnimationMode || upperText != lastUpperText || bottomText != lastBottomText)) {
    lcd.fillScreen(ILI9341_BLACK);
    drawStillText();
    lastAnimationMode = animationMode;
    lastUpperText = upperText;
    lastBottomText = bottomText;
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

    lastAnimationMode = animationMode;
    lastUpperText = upperText;
    lastBottomText = bottomText;
  }

  delay(30);
}
