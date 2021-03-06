#include <Adafruit_GPS.h>
#include <ArduinoJson.h>
// Test code for Adafruit GPS modules using MTK3329/MTK3339 driver
//
// This code shows how to listen to the GPS module in an interrupt
// which allows the program to have more 'freedom' - just parse
// when a new NMEA sentence is available! Then access data when
// desired.
//
// Tested and works great with the Adafruit Ultimate GPS module
// using MTK33x9 chipset
//    ------> http://www.adafruit.com/products/746
// Pick one up today at the Adafruit electronics shop
// and help support open source hardware & software! -ada

#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include "dht.h"
#define dht_apin A0 // Analog Pin sensor is connected to
 
dht DHT;
// Connect the GPS Power pin to 5V
// Connect the GPS Ground pin to ground
// Connect the GPS TX (transmit) pin to Digital 8
// Connect the GPS RX (receive) pin to Digital 7

// you can change the pin numbers to match your wiring:
SoftwareSerial mySerial(7, 8);
Adafruit_GPS GPS(&mySerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO  false

void setup()
{

  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
  Serial.begin(115200);
  delay(5000);
  Serial.println("Adafruit GPS library basic test!");
  Serial.println("DHT11 Humidity & temperature Sensor\n\n");

  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);

  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time

  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);
  // Ask for firmware version
  mySerial.println(PMTK_Q_RELEASE);
}

uint32_t timer = millis();
void loop()                     // run over and over again
{
  char c = GPS.read();
  DynamicJsonDocument root(1024);
 
  DHT.read11(dht_apin);
  Serial.print("Current humidity = ");
  Serial.print(DHT.humidity);
  Serial.print("%  ");
  Serial.print("temperature = ");
  Serial.print(DHT.temperature); 
  Serial.println("C  ");

  // if you want to debug, this is a good time to do it!
  if ((c) && (GPSECHO))
    Serial.write(c);

  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false

    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }

  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();
  String date_time;
  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) {
    timer = millis(); // reset the timer
    
    Serial.print("Date: ");
    Serial.print("20");
    date_time.concat("20");
    Serial.println(GPS.year, DEC); Serial.print('-');
    date_time.concat(GPS.year); date_time.concat('-');
    Serial.print(GPS.month, DEC); Serial.print('-');
    date_time.concat(GPS.month); date_time.concat('-');
    Serial.print(GPS.day, DEC);
    date_time.concat(GPS.day);
    date_time.concat(" ");
    
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
      
    Serial.print("\nTime: ");
    if (GPS.hour < 10) { Serial.print('0');  date_time.concat("0");}
    Serial.print(GPS.hour, DEC); Serial.print(':');
    date_time.concat(GPS.hour); date_time.concat(":"); 
    if (GPS.minute < 10) { Serial.print('0'); date_time.concat("0");}
    Serial.print(GPS.minute, DEC); Serial.print(':');
    date_time.concat(GPS.minute); date_time.concat(":");
    if (GPS.seconds < 10) { Serial.print('0'); date_time.concat("0");}
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    date_time.concat(GPS.seconds);
    
    if (GPS.fix) {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", ");
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);

      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
    }
  }

  Serial.println(date_time);
  root["id"] = "test";
  root["temperature"] = DHT.temperature;
  root["humidity"] = DHT.humidity;
  root["date_time"] = date_time;
  root["longitude"] = GPS.longitude;
  root["latitude"] = GPS.latitude;
  root["satellites"] = (int)GPS.satellites;
  root["gps_speed"] = GPS.speed;

  serializeJson(root, Serial);

  delay(3000);
}
