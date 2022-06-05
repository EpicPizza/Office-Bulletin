#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LiquidCrystal_I2C.h>

AsyncWebServer server(80);

const char* ssid = "ssid"; //settings to server
const char* password = "password";

IPAddress ap_local_IP(152,100,1,1); //ip for server
IPAddress ap_gateway(152,100,1,1);
IPAddress ap_subnet(255,255,255,0);

const char* PARAM_RED = "inputRed";
const char* PARAM_GREEN = "inputGreen";
const char* PARAM_BLUE = "inputBlue";
const char* PARAM_MESSAGE_ONE = "inputMessageOne";
const char* PARAM_MESSAGE_TWO = "inputMessageTwo";

int lcdColumns = 16; //declare i2c display
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 

String message1; //one for the top line, one for the bottom line
String message2;

int red = 0; //current color of arduino outside
int blue = 0;
int green = 0;
int red_out = 0; //current color of arduino at desk
int blue_out = 0;
int green_out = 0;
/*Idk why i made it this complicated when i was did it, 
but I think was just trying to make the yellow look 
more like yellow and less green on the leds of the arduino, 
although when I set a custom color through the website, they 
both get set the same values.*/

const int redPin = 18;
const int greenPin = 2;
const int bluePin = 4;
const int statusPin = 23;

const char index_html[] PROGMEM = R"rawliteral( //website code for setting custom colors and messages
<!DOCTYPE html>
<style>
  .Interface {
    width: 300px;
    height: fixed;
    background-color: black;
    padding: 15px;
    border-radius: 8px;
    font-family: sans-serif;
    text-align: center;
    margin: 20px;
    color: aqua;
  }

  html {
    background-color: aqua;
    width: 300px;
  }

  button {
    background-color: aqua;
    color: white;
    padding: 10px;
    text-align: center;
    font-size: 13px;
    border-color: black;
    border-width: 2px;
    border-radius: 5px;
    margin: 5px;
    width: 100px
  }

  button {
    background-color: aqua;
    color: black;
    padding: 10px;
    text-align: center;
    font-size: 13px;
    border-color: black;
    border-width: 2px;
    border-radius: 5px;
    margin: 5px;
    height: fixed;
    width: 100px;
  }
  input {
    width:75px;
    margin: 5px;
  }
</style>
<html>

<body onLoad="startUp()">
<meta name="viewport" content="width=device-width, initial-scale=1">
  <div class="Interface" id="home">
    <h1>Color Bullentin</h1>
    <p>Chose one of settings.</p>
    <button onclick="colorUp()">Color</button>
    <button onclick="mes1Up()">Message</button>
  </div>
  <div class="Interface" id="color">
    <h1>Color</h1>
    <p>Custom RGB:</p>
    <form action="/get" target="hidden-form">
      <input type=number name="inputRed"></input>
      <input type=number name="inputGreen"></input>
      <input type=number name="inputBlue"></input>
      <input type="submit" value="Submit" onclick="submitMessage()"></input>
    </form>
  </div>
  <div class="Interface" id="mes1">
    <h1>Message</h1>
    <form action="/send" target="hidden-form">
    <input type=text name="inputMessageOne"></input>
    <input type="submit" value="Submit" onclick="submitMessage()"></input>
  </form>
  <br>
  <form action="/send" target="hidden-form">
    <input type=text name="inputMessageTwo"></input>
    <input type="submit" value="Submit" onclick="submitMessage()"></input>
  </form>
    <br></br>
    <button onclick="startUp()">Home</button>
  </div>
  <iframe style="display:none" name="hidden-form"></iframe>
</body>

</html>
<script>
  function startUp() {
    document.getElementById("home").hidden = false;
    document.getElementById("color").hidden = true;
    document.getElementById("mes1").hidden = true;

  }
  function colorUp() {
    document.getElementById("home").hidden = true;
    document.getElementById("color").hidden = false;
    document.getElementById("mes1").hidden = true;
  }
  function mes1Up() {
    document.getElementById("home").hidden = true;
    document.getElementById("color").hidden = true;
    document.getElementById("mes1").hidden = false;
  }
  function submitMessage() {
    alert("Saved value to ESP");
    setTimeout(function(){ document.location.reload(false); }, 500);  
  }
</script>)rawliteral";

void updateLcd() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(message1);
  lcd.setCursor(0,1);
  lcd.print(message2);
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not Found, go to 152.100.1.1");
}

void setup() {
  Serial.begin(115200);
  
  Serial.println(WiFi.softAP(ssid, password) ? "soft-AP setup": "Failed to connect"); //set up its own network
  Serial.println(WiFi.softAPIP());
  while(!(WiFi.softAPIP()== ap_local_IP)){
    WiFi.softAPConfig(ap_local_IP, ap_gateway, ap_subnet);    
  } 

  delay(1000);

  Serial.println();
  Serial.print("IP Address");
  Serial.println(WiFi.localIP());

  ledcAttachPin(redPin, 1); //declare rgb
  ledcAttachPin(greenPin, 2);
  ledcAttachPin(bluePin, 3);
  ledcSetup(1,12000,8);
  ledcSetup(2,12000,8);
  ledcSetup(3,12000,8);
  pinMode(statusPin, OUTPUT);

  server.on("/", HTTP_GET, [] (AsyncWebServerRequest *request){ //main page
    request->send_P(200, "text/html", index_html);
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) { //for receiving custom colors
    if(request->hasParam(PARAM_RED)) {
      String redString = request->getParam(PARAM_RED)->value();
      red = redString.toInt();
      red_out = red;
    } else {
      Serial.println("Nothing Sent");
    }
    if(request->hasParam(PARAM_GREEN)) {
      String greenString = request->getParam(PARAM_GREEN)->value();
      green = greenString.toInt();
      green_out = green;
    } else {
      Serial.println("Nothing Sent");
    }
    if(request->hasParam(PARAM_BLUE)) {
      String blueString = request->getParam(PARAM_BLUE)->value();
      blue = blueString.toInt();
      blue_out = blue;
    } else {
      Serial.println("Nothing Sent");
    }
    Serial.println();
    Serial.print(red);
    Serial.print(green);
    Serial.print(blue);
    request->send(200, "text", "Go to 152.100.1.1"); //in case I enter request manually
  });

  server.on("/send", HTTP_GET, [] (AsyncWebServerRequest *request) { //for recieving custom messages
    if(request->hasParam(PARAM_MESSAGE_ONE)) {
      message1 = request->getParam(PARAM_MESSAGE_ONE)->value();
    } else {
      Serial.println("Nothing Sent");
    }
    if(request->hasParam(PARAM_MESSAGE_TWO)) {
      message2 = request->getParam(PARAM_MESSAGE_TWO)->value();
    } else {
      Serial.println("Nothing Sent");
    }
    Serial.println(message1);
    Serial.print(message2);
  });

  server.on("/off", HTTP_GET, [] (AsyncWebServerRequest *request) { //setting the bulleting to blank
    message1 = "";
    message2 = "";
    red = 0;
    green = 0;
    blue = 0;
  });

  server.on("/value", HTTP_GET, [] (AsyncWebServerRequest *request) { //debugging
    String string = String("RGB Value: ") + red + ", " + green + ", " + blue + ".<br><a href=\"/\">Return to Home Page</a>";
    request->send(200, "text/html", string); 
  });

  server.on("/red", HTTP_GET, [] (AsyncWebServerRequest *request) { //sends red value current value
    request->send(200, "text/plain", String(red_out));
  });

  server.on("/green", HTTP_GET, [] (AsyncWebServerRequest *request) { //sends blue current value
    request->send(200, "text/plain", String(green_out));
  });

  server.on("/blue", HTTP_GET, [] (AsyncWebServerRequest *request) { //sends green current value
    request->send(200, "text/plain", String(blue_out));
  });

  server.on("/updateRed", HTTP_GET, [] (AsyncWebServerRequest *request) { //sets bulletin to red and message if gets request from arduino at desk that red button was pressed
    red = 255;
    green = 0;
    blue = 0;
    red_out = red;
    green_out = green;
    blue_out = blue;
    message1 = "I'm in a meeting.";
    message2 = "";
    request->send(200, "text/plain", "ok");
  });

  server.on("/updateYellow", HTTP_GET, [] (AsyncWebServerRequest *request) { //sets bulletin to yellow and message if gets request from arduino at desk that yellow button was pressed
    red = 255;
    green = 15;
    blue = 0;
    red_out = 155;
    green_out = 155;
    blue_out = 0;
    message1 = "I'm working.";
    message2 = "";
    request->send(200, "text/plain", "ok");
  });

  server.on("/updateGreen", HTTP_GET, [] (AsyncWebServerRequest *request) { //sets bulletin to green and message if gets request from arduino at desk that green button was pressed
    red = 0;
    green = 255;
    blue = 0;
    red_out = red;
    green_out = green;
    blue_out = blue;
    message1 = "I'm available.";
    message2 = "";
    request->send(200, "text/plain", "ok");
  });
  
  server.onNotFound(notFound);
  server.begin();

  delay(1000);

  lcd.begin();
  lcd.setCursor(4,0);
  lcd.print("Website:");
  lcd.setCursor(2,1);
  lcd.print("152.100.1.1");
  lcd.setCursor(0,0);

  digitalWrite(statusPin, HIGH);
}

void loop() { //just keeps setting color and message
  setColor(red, green, blue);
  updateLcd();
  delay(2000);
}

void setColor(unsigned int red, unsigned int green, unsigned int blue) { //seting color of rgb led
  ledcWrite(1, red);
  ledcWrite(2, green);
  ledcWrite(3, blue);
}
