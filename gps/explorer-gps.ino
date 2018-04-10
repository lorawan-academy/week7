#include <TheThingsNetwork.h>
#include <TinyGPS.h>

TinyGPS gps;

// Set your AppEUI and AppKey
const char *appEui = "0000000000000000";
const char *appKey = "00000000000000000000000000000000";

#define loraSerial Serial2
#define debugSerial SerialUSB

// Replace REPLACE_ME with TTN_FP_EU868 or TTN_FP_US915
#define freqPlan REPLACE_ME

TheThingsNetwork ttn(loraSerial, debugSerial, freqPlan);


void setup() 
{
  loraSerial.begin(57600);
  debugSerial.begin(9600);
  Serial.begin(9600); // connect Rx to D1 & Tx to D0

  // Wait a maximum of 10s for Serial Monitor
  while (!debugSerial && millis() < 10000)
    ;

  debugSerial.println("-- STATUS");
  ttn.showStatus();

  debugSerial.println("-- JOIN");
  ttn.join(appEui, appKey);
 
  // Set callback for incoming messages
  ttn.onMessage(message);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() 
{
  uint16_t temp = getTemperature();
  
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;
  float flat, flon;
  
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
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    SerialUSB.print("LAT=");
    SerialUSB.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    SerialUSB.print(" LON=");
    SerialUSB.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    SerialUSB.print(" SAT=");
    SerialUSB.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
    SerialUSB.print(" PREC=");
    SerialUSB.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
  }

  gps.stats(&chars, &sentences, &failed);
  SerialUSB.print(" CHARS=");
  SerialUSB.print(chars);
  SerialUSB.print(" SENTENCES=");
  SerialUSB.print(sentences);
  SerialUSB.print(" CSUM ERR=");
  SerialUSB.println(failed);
  if (chars == 0)
    SerialUSB.println("** No characters received from GPS: check wiring **");


  uint32_t lat = flat * 1000000;
  uint32_t lon = flon * 1000000;

  SerialUSB.println(lat);
  SerialUSB.println(lon);


  byte payload[10];
  payload[0] = highByte(temp);
  payload[1] = lowByte(temp);
  
  payload[2] = lat >> 24;
  payload[3] = lat >> 16;
  payload[4] = lat >> 8;
  payload[5] = lat;
  
  payload[6] = lon >> 24;
  payload[7] = lon >> 16;
  payload[8] = lon >> 8;
  payload[9] = lon;
  
  ttn.sendBytes(payload, sizeof(payload));

  delay(10000);
}

uint16_t getTemperature()
{
  //10mV per C, 0C is 500mV
  float mVolts = (float)analogRead(TEMP_SENSOR) * 3300.0 / 1023.0;
  int temp = (mVolts - 500) * 10;
  
  debugSerial.print((mVolts - 500) / 10);
  debugSerial.println(" Celcius");
  return int(temp);
}

void message(const uint8_t *payload, size_t size, port_t port)
{
  if (payload[0] == 0) {
    digitalWrite(LED_BUILTIN, LOW);
  }

  else {
    digitalWrite(LED_BUILTIN, HIGH);
  }

}
