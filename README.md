# MK312 Wifi Bridge

As usual no guarantees can be given, and also if you use this to injure yourself, no responsibility can be taken

This Project is based on the ESP8266-01S  
This was created because bluetooth could not be accessed in a convinient way by the VR Headset I was using.  

## Folders:
MK312Wifi - Contains the ino file that should be used to flash the ESP  
MK312-wifi-pcb - Contains the files needed to print the PCB  
DotNetClient - Example implementation in .net (visual studio code)  

## General:
The wifi interface, once established is byte compatible to the established bluetooth interface, with some extensions, to make
custom implementations more easy.  
Included is an example C# implementation, that will be used in a unity project that this was created for. It should simplyfy talking to the device.  
However, most existing implementations should be really easy to adapt to the wifi version.  

To make things configuration free the firmware features an UDP port for automatically determining the IP Address of the WIFI adapter as well
as a TCP port for the actual communication with the device. At any time, only one client can be connected.

You can see a connection by the radio LED lighting up and then flashing as communication is in progress.

## Hardware:
Feel free to use the provided PCB layout for the connections. In case you want to build your own:  
Keep in mind that the VCC from the box is 5 Volts, so you will need to convert that to 3.3 or you will burn your ESP module.  
The Signal levels conviniently are already at 3.3 volts so we don't really need to do anything here.  
Connections are: 
|MK312|Inbetween|ESP Pin|ESP Function|
|-----|-        |----------|---------|
|GND  |         |Pin1      | GND     |
|3.3v |         |Pin8      | VCC     |
|     |         |Pin4      | CHIP_EN |  
|RX   |         |Pin5      | GPIO0   |  
|TX   |         |Pin3      | GPIO2   |  
|STATE|         |Pin2      | GPIO1   |  
|GND  | resetwifi button|Pin7| GPIO3 |  


The hardware serial port outputs a lot of garbage in the bootloader, that can confuse the MK312, so a software implementation is used that might make the used pin seem a bit odd.

## Firmware:
Upon first powerup, the device will go into an Access Point mode, which you can connect to with your cellphone, to connect it to your local wifi.
The MK312 disply will show "WifiAP"
Once a Wifi Connection is established, the device will display the IP Address on the LCD Display.
In consequent powerups the device will reconnect to the WIFI Network that was configured. If you wish to change the network to use, press the config wifi button after powerup. (not during)

The firmware will negotiate a key with the box, and then use that key continuously internally.
You can send a UDP broadcast to port 8842 containing the string "MK312-ICQ" to have the device return its IP Address
You can then create a TCP connection to that IP Address on port 8843.

### Normal encrypted mode:
you can then proceed just like you would with a serial connection, and send the 0x00 to recieve 0x07 and then do the key negotation
### Unencrypted mode:
if you wish to skip encryption, instead of the normal negotiation command you need to send 0x2f 0x42 0x42. This is an invalid checksum, but the command will be recogniced by the device.  
It will reply with 0x69. From there on you do not need to use any encryption.

From here on communication is no different from the serial communication:
https://github.com/buttplugio/stpihkal/blob/master/stpihkal/protocols/erostek-et312b.md

Serial software implementations should not notice the difference. Once connection is lost, the software should be able to reestablish the connection normaly.

## Building:

Please note that not all components need to be fitted to the front of the device. There are hints on PCB to what goes where.
Components:
1x ESP8266-01S module  
2x 100uF (104) Capacitors  
1x Diode  
1x Switch  
1x 5 pin connector, angled  
1x 2x4 pin socket, angled, alternatively 2x4 pin socket, see both screenshots.  


** Flashing:
Please keep in mind that the ESP8266-01 runs on 3.3 volts. So your serial adapter should be set in 3.3VOLTS MODE. 
The ESP8266-01 can be programmed from the arduino software like this:

|Left|Pins|Right|
|---|---|---|
|TXD|o o|VCC|  
|PRG|o o|RST|  
|   |o o|VCC|  
|GND|o o|TXD|  

To put the ESP into programming mode, keep PRG connected to ground, and make RST touch GND briefly to reset.

Setting up the Arduino software: (it might work with newer versions, these are just the versions we developed things on)
Install Arudino: https://www.arduino.cc/en/software (Version 1.8.19)
Install Boardmanager: http://arduino.esp8266.com/stable/package_esp8266com_index.json (Version 3.0.2) [Generic ESP8266 module]
(under 'file/preferences' add the path to "Additional Boards Manager URLs:", then close the dialog with okay, and select the board on "Tools / boardmanager")
Install WifiManager: https://github.com/tzapu/WifiManager (version 2.0.5-beta)
(Download the file, and add it to your libraries with Sketch/Include Library/Add. Zip library)

Then close the application, reopen it and loat the ino file.

Once all is setup correctly, you should be able to compile the accompanied .ino and flash it to the device.

Once the ESP is programmed and attached to the board you can put it into the MK312 bluetooth slot, please make sure its facing the right way.

Once the ESP Powers up, it will immediately try to negotiate with the MK312. If that fails, it will show an error message on the message LED:

1 blink: Returned Checksum is invalid  
2 blinks: handshake fail step 1  
3 blinks: handshake fail step 2  
4 blinks: handshake fail step 3  
5 blinks: unexpected reply from device  
10 blinks: unexpected reply from poke operation  
11 blinks: unexpected reply from peek operation  

Once negotiations are successful, the WIFI module will power up. On first startup it will go into AP mode.  
Look for a network called 'MK312CONFIG-AP' and connect to it with your cellpone. Then set up the WIFI Parameters.  
The module will then connect to WIFI, and display its IP adress on the MK312's display.  
At this point it is ready to be connected to.  

## USAGE

Using software implemented for it:  
- No configuration neccessary. The software will determine the IP via UDP broadcast and then connect to it. See example c# application.
- List of implementations follows

Using legacy bluetooth software:  
### Linux
In linux you can use socat to establish a connection to the device, and offer a comport for the legacy software to connect to. 
The syntax is as follows (replace [] with the corresponding values):
socat -v pty,link=/home/[user]/tcptty0,raw tcp:[IP Address shown on display]:8843  

you can then connect to /home/[user]/tcptty0 from your software.

### Windows
https://www.youtube.com/watch?v=7g6v_m208LQ
