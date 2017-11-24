/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/light/0 will set the GPIO2 low,
 *    http://server_ip/light/1 will set the GPIO2 high
 *    http://server_ip/status will show GPIO2 state
 *  server_ip is the IP address of the ESP8266 module, will be 
 *  printed to Serial when the module is connected.
 *  save last relay state in config.json for use of if esp lose power
 */

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "FS.h"

const char* ssid = "wifi_ssid";
const char* password = "wifi_pass";

boolean loadDataSuccess = false;

const int buttonPin = 2; // gpio 2
const int relayPin = 0; // gpio 0
int relayState;     
int saveme=0;       
int changeme=0;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

// Variables will change:
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
IPAddress ip(192,168,0,135);  //Node static IP
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

bool loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

  relayState = long(json["relayState"]);
  
  // Real world application would store these values in some variables for
  // later use.

  Serial.print("Loaded relayState: ");
  Serial.println(relayState);
  return true;
}

void saveConfig() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["relayState"] = relayState;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
  }
  Serial.print("Save to config: ");
  Serial.println(relayState);
  json.printTo(configFile);
}

void setup() {
  Serial.begin(115200);
  delay(10);

  // prepare pin
  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
delay(1000);
  Serial.println("Mounting FS...");

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }


  if (!loadConfig()) {
    Serial.println("Failed to load config");
  } else {
    Serial.println("Config loaded");
  }
   digitalWrite(relayPin, relayState);
}

void loop() {
  // Check if a client has connected
  int reading = digitalRead(buttonPin);

// If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      relayState = reading;

      // only toggle the relay if the new button state is HIGH
      if (buttonState == HIGH) {
        relayState = !relayState;
        saveme = 1;
        changeme=1;
      }
    }
  }

  
  WiFiClient client = server.available();
  if (client) {
 
  
  while (client.connected()) {
            if (client.available()) {
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // Match the request
  if (req.indexOf("/light/0") != -1)
  {
    relayState = 0;
    saveme = 1;
    changeme = 1;
  }
  else if (req.indexOf("/light/1") != -1)
  {
    relayState = 1;
    saveme = 1;
    changeme = 1;
  }
  else if (req.indexOf("/status") != -1) // use for Hombebridge or verify replay state
  {
    relayState = relayState;
  }
  else {
    Serial.println("invalid request");
    client.stop();
    return;
  }


  if(saveme == 1)
  {
    saveConfig();
    saveme = 0;
  }

  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>";
  s += (relayState)?"lighton":"lightoff";
  s += "</html>";

  // Send the response to the client
  client.print(s);
  Serial.println("Client disonnected");
          delay(10);      // give the web browser time to receive the data
        client.stop(); // close the connection
          } // end if (client.available())
        } // end while (client.connected())
    } // end if (client)
if(changeme == 1) {
  // Set GPIO2 according to the request
  digitalWrite(relayPin, relayState);
  Serial.print("Set relay state to ");
  Serial.println(relayState);
  changeme = 0;
}

    
lastButtonState = reading;
}

