/****************************
*       Libraries           *
****************************/
#include <TheThingsNetwork.h>


/****************************
*       Declarations        *
****************************/

// Used for changing the Bssid hex format to binary. Putting one pair into one byte
#define HEX_CHAR_TO_NIBBLE(c) ((c >= 'a') ? (c - 'a' + 0x0A) : (c - '0'))
#define HEX_PAIR_TO_BYTE(h, l) ((HEX_CHAR_TO_NIBBLE(h) << 4) + HEX_CHAR_TO_NIBBLE(l))

// Parameters for the esp stream/buffer
#define SF_ESP8266_DEFAULT_WAIT 5000
#define SF_ESP8266_BUFFER_SIZE 128


// The standard speed of the ESP-01 module is 115200. This can be changed with the following command AT+CIOBAUD= *Desired serial speed e.g. 9600* 
//(Note: The default speed can not be used with the explorer board, as its serial port can only handle up to 57600
#define ESP_SERIAL_SPEED 9600

// Total nummber of access points you want to read/use, and Bssid address size (used for the size of the array)
#define ACCESS_POINTS 3
#define SF_WIFI_BSSID_SIZE 6

// Set your AppEUI and AppKey
const char *appEui = "70B3D57ED0007DCD";
const char *appKey = "ED3122117C4341D72A08EF875A0EEA30";

//const char *appEui = "0000000000000000";
//const char *appKey = "00000000000000000000000000000000";

// Replace REPLACE_ME with TTN_FP_EU868 or TTN_FP_US915
#define freqPlan TTN_FP_EU868

// Define the used serials
#define debugSerial SerialUSB
#define loraSerial Serial2
#define espSerial Serial // When using the sodaq explorer connect the ESP TX to pin D0 and RX to D1 

// Prototyping the functions
char *readLine(uint32_t waitTime = SF_ESP8266_DEFAULT_WAIT);
bool waitForOK(uint32_t waitTime = SF_ESP8266_DEFAULT_WAIT);

// Declaring variables
uint32_t timeout;
uint8_t numberOfBssid = 4; // Set the number of bssid's you want to scan
uint8_t payloadLength = numberOfBssid * SF_WIFI_BSSID_SIZE; 
char buffer[SF_ESP8266_BUFFER_SIZE];
byte aps[ACCESS_POINTS][SF_WIFI_BSSID_SIZE]; // The array that holds the Bssid's

// TTN constructor
TheThingsNetwork ttn(loraSerial, debugSerial, freqPlan);

/****************************
*           Setup           *
****************************/
void setup()
{
// Wait a maximum of 10s for Serial Monitor
  while (!debugSerial && millis() < 10000);
  
  loraSerial.begin(57600);
  debugSerial.begin(ESP_SERIAL_SPEED);
  espSerial.begin(ESP_SERIAL_SPEED);
  
  debugSerial.println("-- STATUS");
  ttn.showStatus();

  debugSerial.println("-- JOIN");
  ttn.join(appEui, appKey);
}

/****************************
*           loop            *
****************************/

void loop()
{
  uint8_t bssidNumber = 0;

// command to esp, set station mode
  espSerial.println(F("AT+CWMODE=1")); 
  bool x = waitForOK();
  
  if (!x) 
  {
    debugSerial.println(F("Set mode failed"));
    goto exit;
  }

// Command to esp to get accesspoint information
  espSerial.println(F("AT+CWLAP"));

  char *line;
  timeout = millis() + 3000;
  
    while (millis() < timeout && bssidNumber < numberOfBssid) 
    {
    line = readLine();
    debugSerial.println(line);
      if (strncmp("OK", line, 2) == 0) 
      {
        debugSerial.println(F("end of access points"));
        break;
      }
      if (strncmp("+CWLAP", line, 2) != 0) 
      {
        delay(1); 
        debugSerial.println(F("No Bssid"));
        continue;
      }
      
// Break the accesspoint in tokens, to get the bssid out.    
      strtok(line, "\"");
      strtok(NULL, "\"");
      strtok(NULL, "\"");
      char *bssid = strtok(NULL, "\"");

// Check if a valid bssid string has been found     
      if (bssid && strlen(bssid) == SF_WIFI_BSSID_SIZE * 3 - 1) 
      {
      debugSerial.print(F("Bssid: ")); 
      debugSerial.println(bssid);

// putting the bssid into an array for easy access
         
      char *b = strtok(bssid, ":");
        for (uint8_t bssidByte = 0; bssidByte < SF_WIFI_BSSID_SIZE && b; bssidByte++, b = strtok(NULL, ":")) 
        {
          aps[bssidNumber][bssidByte] = HEX_PAIR_TO_BYTE(b[0], b[1]);
        }
        bssidNumber++;
      }
    }
  
    sendBssid();
  
exit:
    debugSerial.println("Restart the loop");
}

/****************************
*         Functions         *
****************************/

char* readLine(uint32_t waitTime) 
{
  uint32_t timeout = millis() + waitTime;
    while (millis() < timeout) 
    {
      uint8_t l = espSerial.readBytesUntil('\n', buffer, SF_ESP8266_BUFFER_SIZE);
        if (l > 0) 
        {
          buffer[l-1] = '\0';
          return buffer;
        }
      delay(1);
    }
  return NULL;
}

bool waitForOK(uint32_t waitTime) 
{
  uint32_t timeout = millis() + waitTime;
    while (millis() < timeout) 
    {
      uint8_t l = espSerial.readBytesUntil('\n', buffer, SF_ESP8266_BUFFER_SIZE);
        if (l > 0 && strncmp("OK", buffer, 2) == 0) 
        {
          return true;
        }
      delay(1);
    }
  return false;
}

void sendBssid() 
{
  byte payload[payloadLength];
  uint8_t payloadByte = 0;
  
  for(uint8_t bssidNumber = 0; bssidNumber < numberOfBssid; bssidNumber++)
  {
    for(uint8_t bssidByte = 0; bssidByte < SF_WIFI_BSSID_SIZE; bssidByte++, payloadByte++)
    {
      payload[payloadByte]= aps[bssidNumber][bssidByte];
    }
  }

  ttn.sendBytes(payload, sizeof(payload));
  delay(60000);
}
/*
int signalSort()
{
  for (int i = 0; i < bssidNumber; i++) {
    for (int j = i + 1; j < n; j++) {
      if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
        swap(indices[i], indices[j]);
      }
    }
  }
*/
/****************************
*     Payload function      *
****************************/
/*
 function Decoder(bytes, port) {
  var Bssid1 = [6];
  var Bssid2 = [6];
  var Bssid3 = [6];
  var i, j;
  var bit;
  
  
  var decoded = {};
  for(i = 0, j = 0; i < 6 && j < 6; i++, j++){
    bit = bytes[j].toString(16);
    if (i != 5) { 
      Bssid1[i] = pad(bit) + ":";
    }
    else {
      Bssid1[i] = pad(bit);
    }
  }
  for(i = 0, j = 6; i < 6 && j < 12; i++, j++){
    bit = bytes[j].toString(16);
    if (j != 11) {
      Bssid2[i] = pad(bit) + ":";
    }
    else {
      Bssid2[i] = pad(bit);
    }
  }
    for(i = 0, j = 12; i < 6 && j < 18; i++, j++){
     bit = bytes[j].toString(16);
    if (j != 17) {
      Bssid3[i] = pad(bit) + ":";
    }
    else {
      Bssid3[i] = pad(bit);
    }
  }
  

  decoded.Bssid1 = Bssid1.join('') ;
  decoded.Bssid2 = Bssid2.join('') ;  
  decoded.Bssid3 = Bssid3.join('') ;
  
  return decoded;
}


function pad(n) {
  return  (n < 10 || n == 'a' || n =='b' || n == 'c' || n == 'd' || n == 'e' || n == 'f' ) ? ("0" + n) : n;

}
 */
