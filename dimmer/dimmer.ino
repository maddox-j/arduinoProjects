#include <WiFi.h>

const char* ssid     = "ESP32-DIMMER";
const char* password = "123456789";
WiFiServer server(80);

String header;

const int ledPin = 17;
const int BUTTON = 18;
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;
int dutyCycle = 0;
boolean upper = false;

String startingSlider="50";

void setup() {
  Serial.begin(115200);
  
  //Light change setup.
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(ledPin, ledChannel);
  pinMode(ledPin, OUTPUT);
  pinMode(BUTTON,INPUT);

  //Server setup
  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  server.begin();
  
}

void loop() 
{
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        //Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            // Web Page Heading
            client.println(homeWebPage());
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
         
        }else{
           Serial.println(currentLine);
           if (currentLine.indexOf("brightness")>-1){ 
              String tempString = currentLine;           
              tempString.replace("GET /leptiric?brightness=","");
              tempString.replace(" HTTP/1.1","");
              tempString.trim();
              Serial.println("*******" + tempString);
              startingSlider=tempString;
              double tempDutyCycle = (tempString.toDouble()/100)*255;
              Serial.println(tempDutyCycle);
              dutyCycle = (int)tempDutyCycle;
              Serial.println(dutyCycle);
              ledcWrite(ledChannel, dutyCycle);           
           }
           currentLine="";
           
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  
}

void determineNextValue()
{
  if (dutyCycle == 255)
  {
    upper = true;
    dutyCycle--;
    determineNextValue();
  }
  if(dutyCycle == 0)
  {
    upper = false; 
    dutyCycle++;
    determineNextValue();
  }

  if (upper == false)
  {
    dutyCycle++;
  }
  else
  {
    dutyCycle--;
  }
}

void dimmerButtonChange()
{
   if (digitalRead(BUTTON) == HIGH)
  {
    determineNextValue();
    ledcWrite(ledChannel, dutyCycle);
    delay(15);
  }
  else
  {
    ledcWrite(ledChannel, dutyCycle);
    delay(15);
  }
}

String homeWebPage()
{
 String HTML;
 HTML = "\<!DOCTYPE html>";
 HTML +="\<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
 HTML += "\<title>ESP32 Dimmer Control System </title>";
 HTML+= "\<style> body{background:  rgb(50,50,50); color: white; }h1{color: red; font-family: arial; }</style>";
 HTML += "\<html>\<body><h1>ESP32 Light Dimmer Controller</h1>";
 HTML += "\<p>Welcome to the page that controls the brightness of an LED on the breadboard.</p>";
 HTML+="\<p>Please choose your desired percentage brightness:</p>";
 HTML+="\<form action=\"leptiric\" method=\"get\">";
 HTML+="\<label for=\"brightness\"><em>Brightness (between 0% and 100%):</em></label>";
 HTML+="\<INPUT type=\"range\" id =\"brightness\" name=\"brightness\" value=" + startingSlider + " min=\"0\" step\"1\" max =\"100\" onchange=\"this.form.submit()\">\<BR>";
 HTML+="\</form>";
 HTML += "\</body>\</html>";
 Serial.println(HTML);
 return HTML;
}
