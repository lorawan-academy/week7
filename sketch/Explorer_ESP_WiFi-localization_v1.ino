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
#define DEBUGSERIAL SerialUSB
#define LORASERIAL Serial2
#define ESPSERIAL Serial

// TTN constructor
TheThingsNetwork ttn(LORASERIAL, DEBUGSERIAL, FREQPLAN);

bool waitForOK(uint32_t waitTime, char buffer[])
{
  uint32_t timeout = millis() + waitTime;
  while (millis() < timeout)
  {
    uint8_t l = ESPSERIAL.readBytesUntil('\n', buffer, ESP8266_BUFFER_SIZE);
    if (l > 0 && strncmp("OK", buffer, 2) == 0)
    {
      return true;
    }
    delay(1);
  }
  return false;
}

char *readLine(uint32_t waitTime, char buffer[])
{
  uint32_t timeout = millis() + waitTime;
  while (millis() < timeout)
  {
    uint8_t l = ESPSERIAL.readBytesUntil('\n', buffer, ESP8266_BUFFER_SIZE);
    if (l > 0)
    {
      buffer[l - 1] = '\0';
      return buffer;
    }
    delay(1);
  }
  return NULL;
}

void sendBssid(byte aps[][WIFI_BSSID_SIZE])
{
  byte payload[ACCESS_POINTS * WIFI_BSSID_SIZE];
  uint8_t payloadByte = 0;
  for (int bssidNumberCount = 0; bssidNumberCount < ACCESS_POINTS; bssidNumberCount++)
  {
    for (int bssidByte = 0; bssidByte < WIFI_BSSID_SIZE; bssidByte++, payloadByte++)
    {
      payload[payloadByte] = aps[bssidNumberCount][bssidByte];
    }
  }
  ttn.sendBytes(payload, sizeof(payload));
}

void setup()
{
  // Wait a maximum of 10s for Serial Monitor
  while (!DEBUGSERIAL && millis() < 10000)
    ;

  LORASERIAL.begin(57600);
  DEBUGSERIAL.begin(ESP8266_SERIAL_SPEED);
  ESPSERIAL.begin(ESP8266_SERIAL_SPEED);

  DEBUGSERIAL.println("-- STATUS");
  ttn.showStatus();

  DEBUGSERIAL.println("-- JOIN");
  ttn.join(appEui, appKey);
}

void loop()
{
  uint8_t bssidNumberCount = 0;
  uint32_t timeout;
  char buffer[ESP8266_BUFFER_SIZE];
  byte aps[ACCESS_POINTS][WIFI_BSSID_SIZE];

  // command to esp, set station mode
  ESPSERIAL.println(F("AT+CWMODE=1"));
  bool OK = waitForOK(ESP8266_DEFAULT_WAIT, buffer);

  if (!OK)
  {
    DEBUGSERIAL.println(F("Set mode failed"));
    goto exit;
  }

  // Command to esp to get accesspoint information
  ESPSERIAL.println(F("AT+CWLAP"));

  char *line;
  char *previousLine;
  timeout = millis() + 3000;
  while (millis() < timeout && bssidNumberCount < ACCESS_POINTS)
  {
    previousLine = readLine(ESP8266_DEFAULT_WAIT, buffer);
    line = readLine(ESP8266_DEFAULT_WAIT, buffer);
    DEBUGSERIAL.println(line);
    if (strncmp("OK", line, 2) == 0 || strncmp("OK", previousLine, 2) == 0)
    {
      DEBUGSERIAL.println(F("end of access points"));
      break;
    }
    if ((strncmp("+CWLAP", line, 2) != 0) && (strncmp("+CWLAP", previousLine, 2) != 0))
    {
      delay(1);
      DEBUGSERIAL.println(F("No access points found"));
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
      DEBUGSERIAL.print(F("Bssid: "));
      DEBUGSERIAL.println(bssid);

      // putting the bssid into an array for easy access
      char *b = strtok(bssid, ":");
      for (int bssidByte = 0; bssidByte < WIFI_BSSID_SIZE && b; bssidByte++, b = strtok(NULL, ":"))
      {
        aps[bssidNumberCount][bssidByte] = HEX_PAIR_TO_BYTE(b[0], b[1]);
      }
      bssidNumberCount++;
    }
  }

  sendBssid(aps);
  delay(WAIT_TO_SEND);
exit:
  DEBUGSERIAL.println("Restart the loop");
  delay(500);
}
