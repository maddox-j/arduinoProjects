#include <arduinoFFT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET    -1


#define MICROPHONE_BLUE 4
#define MICROPHONE_RED 2

int blueMin = 100;
int blueMax = 100;
int redMin = 100;
int redMax = 100;

int micValue [128];

double im[128], data[128];
int i = 0, val;
int x = 0, ylim =32;


// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

arduinoFFT FFT = arduinoFFT();

void setup() {
 Serial.begin(115200);
  pinMode(MICROPHONE_RED,INPUT);
  pinMode(MICROPHONE_BLUE,INPUT);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  printGreeting();
}

void loop() {
  listen();
  
}

void printGreeting()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 16);
  display.println(F("Hello, Andrej"));
  display.display();      // Show initial text
  delay(2000);
  display.clearDisplay();
}

void listen()
{
  int blueValue=analogRead(MICROPHONE_BLUE);
  int redValue=analogRead(MICROPHONE_RED);
  for (int i = 127; i > 0; i--)
  {
    micValue[i] = micValue[i-1];
  }
  int offset = 700 ;
  micValue[0] = (int)(((blueValue-offset)*32)/(1800-offset)); //2224 is for blueMic 4905 is for redMic
  //micValue[0] = (int)(((redValue)*32)/(4096)); //4096 is since the inputs of the ESP32 are 12-bit inputs.
  displayMusic();
  calculateMaxAndMin(blueValue,redValue);
  displayValues(blueValue, redValue);
 // analyseFrequency();
}

void displayMusic()
{
  display.clearDisplay();

  for(int i = 0; i<127;i++)
  {
    display.drawLine(i,32,i,(32-micValue[i]),SSD1306_WHITE);
  }
 
  display.display();
  
}

void calculateMaxAndMin(int blueValue, int redValue)
{
  if (blueValue>blueMax)
  {
    blueMax = blueValue;
  } else if (blueValue<blueMin)
  {
    blueMin = blueValue;
  }
  
  if (redValue>redMax)
  {
    redMax = redValue;
  }else if (redValue<redMin)
  {
    redMin = redValue;
  }
}

void displayValues(int blueValue, int redValue)
{
  Serial.print("BLUE: ");
  Serial.print(blueValue);
  Serial.print(" ");
  Serial.print(blueMin);
  Serial.print(" ");
  Serial.print(blueMax);
  Serial.println("");
  Serial.print("RED: ");
  Serial.print(redValue);
  Serial.print(" ");
  Serial.print(redMin);
  Serial.print(" ");
  Serial.println(redMax);
  Serial.println("");
}

void analyseFrequency()
{
  int min = 0, max = 4096;
  for (int i = 0; i<128; i++)
  {
    val = analogRead(MICROPHONE_BLUE);
    data[i]=val/4-128;
    im[i]=0;
    if (val>max)
    {
      max=val;
    }
    if (val<min)
    {
      min=val;
    }
  }
  FFT.Compute(data, im,128,FFT_FORWARD); //PREFORMS FFT ON THE DATA ARRAY - 128 is the sample size (needs to be a power of 2). shuld we take 8 samples for the lights ???? 
}

void displayFrequency ()
{
   display.clearDisplay();

  for(int i = 0; i<128;i++)
  {
    int dat = sqrt(data[i]*data[i]+im[i]*im[i]);
    display.drawLine(i,ylim,i,ylim-dat,SSD1306_WHITE);
  }
 
  display.display();
}
