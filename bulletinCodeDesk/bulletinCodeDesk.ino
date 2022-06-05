#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "ssid"; //network of other esp32
const char* password = "password";

const char* serverName = "http://152.100.1.1"; //server name
const char* serverValueRed = "http://152.100.1.1/red"; //get red value
const char* serverValueGreen = "http://152.100.1.1/green"; //get green value
const char* serverValueBlue = "http://152.100.1.1/blue"; //get blue value
const char* serverUpdateRed = "http://152.100.1.1/updateRed"; //change bulletin to red
const char* serverUpdateYellow = "http://152.100.1.1/updateYellow"; //change bulletin to yellow
const char* serverUpdateGreen = "http://152.100.1.1/updateGreen"; //change bulletin to green

unsigned long previousMillis = 0; //when to update led
const long interval = 2000; 

int button1 = 21; 
int button2 = 18;
int button3 = 14;

const int redPin = 15;
const int greenPin = 2;
const int bluePin = 4;

void setup() {
  Serial.begin(115200);

  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);

  WiFi.begin(ssid, password); //connecting to esp32 in outside bulletin
  
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  ledcAttachPin(redPin, 1);
  ledcAttachPin(greenPin, 2);
  ledcAttachPin(bluePin, 3);
  ledcSetup(1,12000,8);
  ledcSetup(2,12000,8);
  ledcSetup(3,12000,8);

  setColor(0, 255, 0); //sets led to blue at start up

}

void loop() {
  unsigned long currentMillis = millis();
  
  if(currentMillis - previousMillis >= interval) { //checks if 2 seconds have passed
    
    if(WiFi.status()== WL_CONNECTED ){ 
      String red = httpGETRequest(serverValueRed); //get values from outside bulletin
      String green = httpGETRequest(serverValueGreen);
      String blue = httpGETRequest(serverValueBlue);
      setColor(red.toInt(), green.toInt(), blue.toInt());
      
      previousMillis = currentMillis;
    }
    else {
      Serial.println("WiFi Disconnected"); //sets blue color if disconnected
      setColor(0,0,255);
    }
  }

  checkButton(); //checks button if it needs to send 
}

void setColor(unsigned int red, unsigned int green, unsigned int blue) { //changes rgb color
  red = 255 - red;
  green = 255 - green;
  blue = 255 - blue;
  ledcWrite(1, red);
  ledcWrite(2, green);
  ledcWrite(3, blue);
}

String httpGETRequest(const char* serverName) { //sends to requests to arduino in the bulletin
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "0"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    
    payload = http.getString();
    Serial.println(payload);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

void checkButton() { //checks for buttons, then sends requests if it needs to change color
  int value1 = digitalRead(button1);
  int value2 = digitalRead(button2);
  int value3 = digitalRead(button3);
  if(value1 == HIGH) {
    Serial.println("ButtonPresed1");
    httpGETRequest(serverUpdateRed);
    delay(500);
  } else if(value2 == HIGH) {
    Serial.println("ButtonPresed2");
    httpGETRequest(serverUpdateYellow);
    delay(500);
  } else if(value3 == HIGH) {
    Serial.println("ButtonPresed3");
    httpGETRequest(serverUpdateGreen);
    delay(500);
  }
}
