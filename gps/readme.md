# Localization with GPS

## ExpLoRer + GPS tutorial
Let's add localization functionality to the ExpLoRer board by connecting a GPS module. This tutorial makes use of the uBlox GPS module which is called the GY-GPS6MV2.

This module is quite easy to connect (like most GPS modules) and very accurate if used in an outdoor environment. The downside of GPS modules is the price. This module cost around €20 and positioning is very inaccurate if used indoors. It also consumes a lot of energy.

## Connect the GPS to the ExpLoRer board:

![wiring](https://course.lorawanacademy.com/assets/courseware/v1/09de9f40ff3d865ab7e74330a9fc72d5/asset-v1:_+_+V1+type@asset+block/Artboard_1.jpg)

Install libraries
There are several open source GPS libraries available. In this tutorial we're making use of the library called tinyGPSPlus.

[Download](https://github.com/mikalhart/TinyGPS) the `TinyGPS` library and add it via: 

**Sketch > Include Library > Add .ZIP library**

### Read out GPS data in Serial Monitor
First, try reading out the GPS data and print it to the Serial Monitor. Once we can properly read out the module. We will embed the LoRaWAN connectivity. 

Below you can find the code to read out the GPS module and to print the data to the Serial Monitor. Do you understand what the code stands for?

```
#include <TinyGPS.h>

TinyGPS gps;
#define debugSerial SerialUSB

void setup()
{
  debugSerial.begin(9600);
  Serial.begin(9600);
}

void loop()
{
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (Serial.available())
    {
      char c = Serial.read();
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData)
  {
    float flat, flon;
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    debugSerial.print("LAT=");
    debugSerial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    debugSerial.print(" LON=");
    debugSerial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    debugSerial.print(" SAT=");
    debugSerial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
    debugSerial.print(" PREC=");
    debugSerial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
  }
  
  gps.stats(&chars, &sentences, &failed);
  debugSerial.print(" CSUM ERR=");
  debugSerial.println(failed);
  if (chars == 0)
    debugSerial.println("** No characters received from GPS: check wiring **");
}
```

If all went well, you should see in your Serial Monitor the data as shown below. Let's walk through the data you see:

* LAT - Latitude
* LON - Longitude
* SAT - Number of satellites
* PREC - Precision (the lower the better)
* CSUM ERR - Checksum Errors
* GPS-data

![data](
https://course.lorawanacademy.com/assets/courseware/v1/4542e9949f256a45412e00c52cc54072/asset-v1:_+_+V1+type@asset+block/GPS-data.png)


## LoRa connectivity
Now, let's add the LoRa connectivity. We only need to send the latitude and longitude

A list of to-do's:

* Set the basics, like adding the `TheThingsNetwork` library, setting the AppEUI and AppKey, define the loraSerial, define frequency plan etc.
* Create two `uint32_t`'s called `lat` and `lon` and multiply the `flat` and `flon` by 1000000. Remember we don't want to send floats, but integers instead?
* Next, create 10 bytesSend the bytes
 * The first 2 bytes are the temperature
 * 4 bytes for the latitude
 * 4 bytes for the longitude (in this example there is no need to send the number of satellites, precision, and errors).
* Having troubles? Check the end of this page to find the source file.


### Payload Formats
Put your skills into practice! We need two new variables called lat and lon. Both contain 4 bytes: the first byte needs to be shifted 24 places to the left (4 * 8 bits), the second 16 places etc. Finally, return the two variables. Try it yourself first. Having trouble? Check the bottom of this page.

It should look something like this:

![GPS Console](https://course.lorawanacademy.com/assets/courseware/v1/91066f19b0d304df264d071dbff03f7d/asset-v1:_+_+V1+type@asset+block/GPS-console.png)




## Payload Formats

```
function Decoder(bytes, port) {
  var temperature = ((bytes[0] << 8) | bytes[1]) / 100;
  var lat = ((bytes[2] << 24) | (bytes[3] << 16) | (bytes[4] << 8) | bytes[5]) / 1000000;
  var lon = ((bytes[6] << 24) | (bytes[7] << 16) | (bytes[8] << 8) | bytes[9]) / 1000000;

  return {
    temperature: temperature,
    lat: lat,
    lon: lon,
  }
}
```
