# Wifi Localization sketch

The sketch is built upon the use of the following hardware and components:

* Sodaq ExpLoRer
* ESP8266-01
* [*FTDI USB to TTL logic*](https://www.amazon.com/HiLetgo-FT232RL-Converter-Adapter-Breakout/dp/B00IJXZQ7C/ref=pd_lpo_vtph_147_lp_img_3/131-9012372-9811114?_encoding=UTF8&psc=1&refRID=PH0S6PBC54E7YFZYEKZD)
* Breadboard
* 5 male-female, 4 female-female wires and 1 male-male wire.

### Index
---
* Hookup guide
* Important values/calculations
* Flow of the program
* Functions

### Hookup guide
---
##### **Connecting the ExpLoRer**

The Sodaq ExpLoRer is a development board that has a potential of 3.3 volts on all pins except (ofcourse) the 5 volt pin. So it is possible to connect the esp directly to the Sodaq board. The wiring can be seen in the image down below. 

<img src="resources/sodaq-esp-connection.png" alt="Temporary image sodaq-esp" width="700" align="middle">

##### **Communication**

Let's start communicating with the device. To be able to communicate with the ESP module, it is necessary to set the correct baud rate. It is likely that the ESP module you are using comes preconfigured with a baud rate of 115200. It is however not possible to configure the module with the ExpLoRer board as the serials ports of the ExpLoRer only go up to 57600.

A way to find out the baud rate of the module, is to try an edited passthrough sketch from the TTN library.

```Arduino
#define debugSerial SerialUSB
#define espSerial Serial

void setup()
{
	while (!debugSerial)
		;

	debugSerial.begin(9600);

	delay(500);

	//Possible baud rates to try: 9600, 19200, 38400, 57600. Don’t forget to change the baud rate in the serial monitor accordingly
	espSerial.begin(9600);
	
	debugSerial.println(F("Check if the communication with your ESP module is working by entering the following command 'AT'. Response should be ‘OK’ "));
	debugSerial.println(F("Check the baudrate of your ESP module by entering the following command 'AT+CIOBAUD?' "));
}

void loop()
{
	while (debugSerial.available())
	{
		espSerial.write(debugSerial.read());
	}
	while (espSerial.available())
	{
		debugSerial.write(espSerial.read());
	}
}
```

There are allot of ‘AT’ commands you can send to the device, to configure or check settings. A good reference guide can be found here: [AT commands guide](https://room-15.github.io/blog/2015/03/26/esp8266-at-command-reference/)

A good response looks like this.

<img src="resources/ATOK.png" alt="Image of a good response" width="1000" align="middle">

If you are only getting back garbage values, like seen here.

<img src="resources/WrongBaudrate.png" alt="Image of a garbage response" width="1000" align="middle">

Some extra hardware will be needed. For our use case, a FTDI programmer was used. There are other methods to configure the ESP. Like comunicate with it through an [*arduino*](http://www.martyncurrey.com/arduino-to-esp8266-serial-commincation/), but we will only be looking at the FTDI one. 

*NOTE: It is worth mentioning that if the baudrate is indeed 115200 the example used with the arduino is not going to work, as it is using the softserial library, which can only go up to 57600. 
A work around would be to use a mega which has multiple hardware serials, which can handle the 115200 baudrate.*

With the FTDI it is possible to connect the ESP directly to a pc, and issue commands.

Here is how you connect the FTDI:
 
<img src="resources/FDTI-to-ESP.png" alt="FTDI hookup" width="700" align="middle">
 
When the ESP is connected to your pc through your FTDI, it is possible to communicate with the device. This can be done with your preferred serial terminal program, *E.G.* Putty, Termite, Minicom. To name a few. 
It is also possible to use the arduino serial monitor. The only parameter you need to know is the serial port with which the ESP is connected and the baud rate. 
The serial port can be found in the Arduino IDE, under the `tools section`,

<img src="resources/ChoosePort.png" alt="Image of Arduino choose port" width="300" align="middle">

or in the `device manager` (Windows)

<img src="resources/DeviceManager.png" alt="Image of device manager" width="300" align="middle">

On Mac the easiest way is to open up the terminal and type in the following command: ```ls /dev/cu.*```.
this should list the ports available.

The different baud rates can be tried until one that works is found.

### Important values/calculations
---
`#define HEX_CHAR_TO_NIBBLE(c) ((c >= 'a') ? (c - 'a' + 0x0A) : (c - '0'))`

`#define HEX_PAIR_TO_BYTE(h, l) ((HEX_CHAR_TO_NIBBLE(h) << 4) + HEX_CHAR_TO_NIBBLE(l))`

The above calculations turn a hex value into a byte value. In the `HEX_PAIR_TO_BYTE` the `HEX_CHAR_TO_NIBBLE` is called. Here a single char is evaluated and changed accordingly.

`#define ESP_SERIAL_SPEED 9600`

The standard baudrate of most ESP8266-01 module is 115200. This can be permanently changed by communicating to the device directly and using the following command: **AT+UART_DEF=9600,8,1,0,0** (Note: The default speed cannot be used with the explorer board, as its serial port can only handle up to 57600).

`#define ACCESS_POINTS 3`

`#define SF_WIFI_BSSID_SIZE 6`

`byte aps[ACCESS_POINTS][SF_WIFI_BSSID_SIZE];`

The above declarations are another important part of the code. They define the number of Bssid's that are scanned for. By changing the value of `ACCESS_POINTS` you can scan for more Bssid's.


### Flow of the program
---
* At the start of the loop a connection with the ESP is tested. The explorer sets the ESP into host mode with the following command: `AT+CWMODE=1` .
* If the connection is not working. It retries until it does. The `waitForOK` function is used for this, see the chapter: *Functions* for more information.
* When a connection is confirmed it continues and sends the command to scan for WiFi networks `AT+CWLAP`.
* Then it stays in a loop as long as `millis() < timeout && bssidNumber < ACCESS_POINTS` is true.
* The `readLine` function is then called, see the chapter: *Functions* for more information.
* When the string comes back, it checks if there is a valid response. Which is `+CWLAP`
* The string is then further processed. By breaking it in smaller tokens.
* The fourth token is the Bssid.
* The fourth token is then proofed, if this is successful the Bssid is put into an array.
* This is done by using the `HEX_PAIR_TO_BYTE`.
* If all the Bssid’s have been collected, the array is put into the payload array through the `sendBssid` function.
* The payload is sent to the console and after a delay of 5 minutes everything is repeated.

### Functions
---
The following functions are used in the sketch: `WaitForOK`, `readLine`, `sendBssid`.

```Arduino
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
```

The `waitForOK` function is used at the start of the loop to check if the communication with the ESP is up and running. It puts the data that is sent by the ESP in a buffer which is then checked if it contains the *OK* string by using the `strncmp` function. At the very end of a response, from the ESP these two characters are always present. So, these make a good end marker.

```Arduino
char *readLine(uint32_t waitTime)
{
	uint32_t timeout = millis() + waitTime;
	while (millis() < timeout)
	{
		uint8_t l = espSerial.readBytesUntil('\n', buffer, SF_ESP8266_BUFFER_SIZE);
		if (l > 0)
		{
			buffer[l - 1] = '\0';
			return buffer;
		}
		delay(1);
	}
	return NULL;
}
```

The `readLine` function is similar to the `waitForOk` function. The biggest difference being the return of the buffer, instead of checking it in the function. The buffer is returned because the string that is sent from the ESP this time contains the information we need for the Wifi-localization.

```Arduino
void sendBssid()
{
	byte payload[payloadLength];
	uint8_t payloadByte = 0;
	for (bssidNumber = 0; bssidNumber < ACCESS_POINTS; bssidNumber++)
	{
		for (bssidByte = 0; bssidByte < SF_WIFI_BSSID_SIZE; bssidByte++, payloadByte++)
		{
			payload[payloadByte] = aps[bssidNumber][bssidByte];
		}
	}
	ttn.sendBytes(payload, sizeof(payload));
	delay(300000);
}
```

The `sendBssid` function is used to send the collected Bssid’s over LoRa to the TTN console. By going through two for loops it fills out the payload with the `bssidByte` at the right places.
