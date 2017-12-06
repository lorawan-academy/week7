#include <TheThingsNetwork.h>

#define HEX_CHAR_TO_NIBBLE(c) ((c >= 'a') ? (c - 'a' + 0x0A) : (c - '0'))
#define HEX_PAIR_TO_BYTE(h, l) ((HEX_CHAR_TO_NIBBLE(h) << 4) + HEX_CHAR_TO_NIBBLE(l))

// Parameters for the esp stream/buffer
#define ESP8266_DEFAULT_WAIT 5000
#define ESP8266_BUFFER_SIZE 128
#define ESP8266_SERIAL_SPEED 9600
#define ACCESS_POINTS 3
#define WIFI_BSSID_SIZE 6
#define WAIT_TO_SEND 300000

// Set your AppEUI and AppKey
const char *appEui = "0000000000000000";
const char *appKey = "00000000000000000000000000000000";

// Replace REPLACE_ME with TTN_FP_EU868 or TTN_FP_US915
#define FREQPLAN TTN_FP_EU868

// Define the used serials
#define debugSerial SerialUSB
#define loraSerial Serial2
#define espSerial Serial

// TTN constructor
TheThingsNetwork ttn(loraSerial, debugSerial, FREQPLAN);

// Function used to see if the ESP module sends back a valid response
bool waitForOK(uint32_t waitTime, char buffer[])
{
  uint32_t timeout = millis() + waitTime;
  while (millis() < timeout)
  {
    uint8_t l = espSerial.readBytesUntil('\n', buffer, ESP8266_BUFFER_SIZE);
    if (l > 0 && strncmp("OK", buffer, 2) == 0)
    {
      return true;
    }
    delay(1);
  }
  return false;
}

// Function used to read out the stream from the ESP module
char *readLine(uint32_t waitTime, char buffer[])
{
  uint32_t timeout = millis() + waitTime;
  while (millis() < timeout)
  {
    uint8_t l = espSerial.readBytesUntil('\n', buffer, ESP8266_BUFFER_SIZE);
    if (l > 0)
    {
      buffer[l - 1] = '\0';
      return buffer;
    }
    delay(1);
  }
  return NULL;
}

// Function used to send out the collected bssid's over LoRa
void sendBssid(byte aps[][WIFI_BSSID_SIZE])
{
  byte payload[ACCESS_POINTS * WIFI_BSSID_SIZE];
  uint8_t payloadByte = 0;
  for (int i = 0; i < ACCESS_POINTS; i++)
  {
    memcpy(payload, aps, 6);
  }
  ttn.sendBytes(payload, sizeof(payload));
}

void setup()
{
  // Wait a maximum of 10s for Serial Monitor
  while (!debugSerial && millis() < 10000)
    ;

  loraSerial.begin(57600);
  debugSerial.begin(ESP8266_SERIAL_SPEED);
  espSerial.begin(ESP8266_SERIAL_SPEED);

  debugSerial.println("-- STATUS");
  ttn.showStatus();

  debugSerial.println("-- JOIN");
  ttn.join(appEui, appKey);
}

void loop()
{
  uint8_t bssidCount = 0;
  uint32_t timeout;
  char buffer[ESP8266_BUFFER_SIZE];
  byte aps[ACCESS_POINTS][WIFI_BSSID_SIZE];

  // command to esp, set station mode
  espSerial.println(F("AT+CWMODE=1"));
  bool ok = waitForOK(ESP8266_DEFAULT_WAIT, buffer);

  if (!ok)
  {
    debugSerial.println(F("Set mode failed! Check if wires are connected the right way"));
    goto exit;
  }

  // Command to esp to get accesspoint information
  espSerial.println(F("AT+CWLAP"));

  char *line;
  char *previousLine;
  timeout = millis() + 3000;
  while (millis() < timeout && bssidCount < ACCESS_POINTS)
  {
    previousLine = readLine(ESP8266_DEFAULT_WAIT, buffer);
    line = readLine(ESP8266_DEFAULT_WAIT, buffer);
    debugSerial.println(line);
    if (strncmp("OK", line, 2) == 0 || strncmp("OK", previousLine, 2) == 0)
    {
      debugSerial.println(F("end of access points"));
      break;
    }
    if (strncmp("+CWLAP", line, 2) != 0 && strncmp("+CWLAP", previousLine, 2) != 0)
    {
      delay(1);
      debugSerial.println(F("No access points found"));
      continue;
    }

    // Break the accesspoint in tokens, to get the bssid out.
    strtok(line, "\"");
    strtok(NULL, "\"");
    strtok(NULL, "\"");
    char *bssid = strtok(NULL, "\"");

    // Check if a valid bssid string has been found
    if (bssid && strlen(bssid) == WIFI_BSSID_SIZE * 3 - 1)
    {
      debugSerial.print(F("Bssid: "));
      debugSerial.println(bssid);

      // putting the bssid into an array for easy access
      char *b = strtok(bssid, ":");
      for (int bssidByte = 0; bssidByte < WIFI_BSSID_SIZE && b; bssidByte++, b = strtok(NULL, ":"))
      {
        aps[bssidCount][bssidByte] = HEX_PAIR_TO_BYTE(b[0], b[1]);
      }
      bssidCount++;
    }
  }

  sendBssid(aps);
  delay(WAIT_TO_SEND);
exit:
  debugSerial.println("Restart the loop");
}
