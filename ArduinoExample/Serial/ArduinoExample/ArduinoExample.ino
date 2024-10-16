#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#ifdef ARDUINO_AVR_UNO_WIFI_REV2
#include <Arduino_LSM6DS3.h>  // the IMU used on the Uno Wifi rev 2
#endif

#include "SerialChatGPT.h"

// NeoPixel setup
#define PIN 13  // Pin where NeoPixel is connected
#define NUMPIXELS 8  // Number of NeoPixels in the strip (you can change this depending on how many you have)
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Variables
bool ledState = false;
float motorPosition = 1.0;
int motorSpeed = 0;
int imuValue = 5;
unsigned long previousMillisShake = 0;  // This is used to keep track of notify frequencies
String storedString = "bas";

void setup() {
  Serial.begin(115200); // don't change the baud rate!
  pinMode(LED_BUILTIN, OUTPUT);

  pixels.begin();  // Initialize the NeoPixel library.
  pixels.show();   // Initialize all pixels to 'off'.

#ifdef ARDUINO_AVR_UNO_WIFI_REV2
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1)
      ;
  }
#endif
}

void loop() {
  unsigned long currentMillis = millis();  // we will use this to keep track of notify frequency
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    processCommand(command);
  }



#ifdef ARDUINO_AVR_UNO_WIFI_REV2
  // code for making the IMU data available over bluetooth
  if (IMU.accelerationAvailable()) {
    float acceleration[3];
    // x, y, z
    IMU.readAcceleration(acceleration[0], acceleration[1], acceleration[2]);
    float value = acceleration[0];
    value = value * 10000;
    imuValue = int(value);
    // check if the device is shaken, and make it available over BLE
    if (abs(acceleration[0]) > 2.0 || abs(acceleration[1]) > 2.0 || abs(acceleration[2]) > 2.0) {
      if (currentMillis - previousMillisShake >= 2000) {  // only allow notfications maximum every 2 seconds
                                                          // high G value indicates a shake event
        notify("shake", true);
        previousMillisShake = currentMillis;
      }
    }
  }
#endif
}

void set_LED(bool state) {
  ledState = state;
  //digitalWrite(LED_BUILTIN, state ? HIGH : LOW);

  if ( state == true ){
    for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 0, 0));  
  }
  pixels.show();
  } else {
        for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));  
  }
  pixels.show();
  }
}

void get_LED() {
  notify("get_LED", ledState);
}

void lightStripColor(String color) {
    int firstComma = color.indexOf(',');
    int secondComma = color.indexOf(',', firstComma + 1);

    // Extract the individual components (R, G, B) and convert them to integers
    int red = color.substring(0, firstComma).toInt();
    int green = color.substring(firstComma + 1, secondComma).toInt();
    int blue = color.substring(secondComma + 1).toInt();

        for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(red, green, blue));
    }
    pixels.show();  // Update the strip to reflect the new colors
  }

void set_motor_position(float position) {
  motorPosition = position;
  // Add code to set motor position
}             

void get_motor_position() {
    notify("get_motor_position", motorPosition);
  // Add code to set motor position
}             

void set_motor_speed(int speed) {
  motorSpeed = speed;
  // Add code to set motor speed
}

void get_IMU() {
  notify("get_IMU", imuValue);
}

void set_String(String str) {
  storedString = str;
}

void get_String() {
  notify("get_String", storedString);
}
                                
// {"function_name", "writeDataType", function}
Command commandFunctions[] = {
  { "set_LED", "bool", set_LED},
  { "lightStripColor", "string", lightStripColor},
  { "get_LED", "none", get_LED},
  { "set_motor_position", "float", set_motor_position},
  { "set_motor_speed", "int", set_motor_speed},
  { "get_motor_position", "none", get_motor_position},
  { "get_IMU", "none", get_IMU},
  { "set_String", "string", set_String},
  { "get_String", "none", get_String}

};

// Define the number of commands
const int numCommands = sizeof(commandFunctions) / sizeof(commandFunctions[0]);
