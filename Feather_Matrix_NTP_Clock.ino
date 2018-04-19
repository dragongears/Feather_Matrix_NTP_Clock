// Feather_Matrix_NTP_Clock.ino
// A little clock that gets the time by connecting to the internet over WiFi.
//
// All code is copyright © 2018 by Arthur J. Dahm III.
// This code is released under the Creative Commons Attribution-NonCommercial-ShareAlike 2.5 license.
// https://creativecommons.org/licenses/by-nc-sa/2.5/
//
// Based off of simplestesp8266clock.ino
// http://www.instructables.com/id/Simplest-ESP8266-Local-Time-Internet-Clock-With-OL/
//
// Libraries needed:
//  Adafruit_GFX.h: https://github.com/adafruit/Adafruit-GFX-Library
//  Adafruit_LEDBackpack.h: https://github.com/adafruit/Adafruit_LED_Backpack
//  ESP8266WiFi.h & WiFiUdp.h: https://github.com/esp8266/Arduino
//  NTPClient.h: https://github.com/arduino-libraries/NTPClient
//  Time.h & TimeLib.h:  https://github.com/PaulStoffregen/Time
//  Timezone.h: https://github.com/JChristensen/Timezone
//
//
// Create and include a C header file named "WiFi_Network.h" in the same directory with the following content:
//
// #ifndef WIFI_NETWORK_H
// #define WIFI_NETWORK_H
//
// const char* ssid = "**********";  // insert your own ssid
// const char* password = "**********";  // and password
//
// #endif /* WIFI_NETWORK_H */

#include "WiFi_Network.h"
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

// Define NTP properties
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "ca.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)

// Set up the NTP UDP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

// Create a display object
Adafruit_8x16minimatrix matrix = Adafruit_8x16minimatrix();

String date;
String t;
const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"} ;
const char * months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
const char * ampm[] = {"AM", "PM"} ;

// Digits bitmaps
static const uint8_t PROGMEM // Bitmaps are stored in program memory
  digits[][5] = {
  { B11100000,  // 0
    B10100000,
    B10100000,
    B10100000,
    B11100000 },
  { B01000000,  // 1
    B01000000,
    B01000000,
    B01000000,
    B01000000 },
  { B11100000,  // 2
    B00100000,
    B11100000,
    B10000000,
    B11100000 },
  { B11100000,  // 3
    B00100000,
    B11100000,
    B00100000,
    B11100000 },
  { B10100000,  // 4
    B10100000,
    B11100000,
    B00100000,
    B00100000 },
  { B11100000,  // 5
    B10000000,
    B11100000,
    B00100000,
    B11100000 },
  { B11100000,  // 6
    B10000000,
    B11100000,
    B10100000,
    B11100000 },
  { B11100000,  // 7
    B00100000,
    B00100000,
    B00100000,
    B00100000 },
  { B11100000,  // 8
    B10100000,
    B11100000,
    B10100000,
    B11100000 },
  { B11100000,  // 9
    B10100000,
    B11100000,
    B00100000,
    B11100000 } };

// Connection icons bitmaps
static const uint8_t PROGMEM // Bitmaps are stored in program memory
  connect[][8] = {
  { B00000000,  // Disconnected
    B00000000,
    B00100100,
    B11100111,
    B00100100,
    B00000000,
    B00000000,
    B00000000 },
  { B00000000,  // Connected
    B00000000,
    B00011000,
    B11111111,
    B00011000,
    B00000000,
    B00000000,
    B00000000 } };

void setup ()
{
  Serial.begin(115200);

  timeClient.begin();   // Start the NTP UDP client

  matrix.begin(0x70);  // pass in the address
  matrix.setTextSize(1);
  matrix.setTextWrap(false);  // we don't want text to wrap so it scrolls nicely
  matrix.setTextColor(LED_ON);
  matrix.setRotation(1);
  matrix.clear();
  matrix.writeDisplay();
  Serial.println(F("16x8 LED Mini Matrix Setup Complete"));

  // Connect to wifi
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.print(ssid);

  // Show disconnected icon
  matrix.clear();
  matrix.drawBitmap(4, 0, connect[0], 8, 8, LED_ON);
  matrix.writeDisplay();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi at ");
  Serial.print(WiFi.localIP());
  Serial.println("");

  // Show connected icon
  matrix.clear();
  matrix.drawBitmap(4, 0, connect[1], 8, 8, LED_ON);
  matrix.writeDisplay();
  delay(1000);
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED) //Check WiFi connection status
  {
    date = "";  // clear the variables
    t = "";

    // update the NTP client and get the UNIX UTC timestamp
    timeClient.update();
    unsigned long epochTime =  timeClient.getEpochTime();

    // convert received time stamp to time_t object
    time_t local, utc;
    utc = epochTime;

    // Then convert the UTC UNIX timestamp to local time
    TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -300};  //UTC - 5 hours - change this as needed
    TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -360};   //UTC - 6 hours - change this as needed
    Timezone usEastern(usEDT, usEST);
    local = usEastern.toLocal(utc);

    // now format the Time variables into strings with proper names for month, day etc
    date += days[weekday(local)-1];
    date += ", ";
    date += months[month(local)-1];
    date += " ";
    date += day(local);
    date += ", ";
    date += year(local);

    // format the time to 12-hour format with AM/PM and no seconds
    if(hourFormat12(local) < 10)  // add a zero if hour is under 10
      t += "0";
    t += hourFormat12(local);
    t += ":";
    if(minute(local) < 10)  // add a zero if minute is under 10
      t += "0";
    t += minute(local);
    t += " ";
    t += ampm[isPM(local)];

    // Display the date and time
    Serial.println("");
    Serial.print("Local date: ");
    Serial.print(date);
    Serial.println("");
    Serial.print("Local time: ");
    Serial.print(t);

    // Calculate base coordinates of digits, am/pm indicator and seconds indicator
    uint16_t timeY = isPM(local) ? 1 : 2;
    uint16_t ampmY = isPM(local) ? 7 : 0;
    uint16_t secondX = second(local) % 10;
    secondX = secondX + (secondX > 4 ? 5 : 1);

    // Display the time
    matrix.clear();

    // Draw the am/pm indicator and turn off the pixel for the second
    matrix.drawLine(0, ampmY, 6, ampmY, LED_ON);
    matrix.drawLine(9, ampmY, 15, ampmY, LED_ON);
    matrix.drawPixel(secondX, ampmY, LED_OFF);

    // Draw the digits
    if (t[0] != '0') {
      matrix.drawBitmap(0, timeY, digits[t[0] - '0'], 3, 5, LED_ON);
    }
    matrix.drawBitmap(4, timeY, digits[t[1] - '0'], 3, 5, LED_ON);
    matrix.drawBitmap(9, timeY, digits[t[3] - '0'], 3, 5, LED_ON);
    matrix.drawBitmap(13, timeY, digits[t[4] - '0'], 3, 5, LED_ON);
    matrix.writeDisplay();
  }
  else // attempt to connect to wifi again if disconnected
  {
    // Show disconnected icon
    matrix.clear();
    matrix.drawBitmap(4, 0, connect[0], 8, 8, LED_ON);
    matrix.writeDisplay();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }

    // Show connected icon
    matrix.clear();
    matrix.drawBitmap(4, 0, connect[1], 8, 8, LED_ON);
    matrix.writeDisplay();
    delay(1000);
  }

  delay(1000);    //Send a request to update every 1 sec (= 1,000 ms)
}
