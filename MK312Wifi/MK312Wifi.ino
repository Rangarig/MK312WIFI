/* 
 *  MK-312 Wifi interface
 *  This project is hosted here: https://github.com/Rangarig/MK312WIFI
 */

#include "ESP8266WiFi.h"
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include <WiFiManager.h>        // https://github.com/tzapu/WiFiManager
#include <WiFiUdp.h>

#define version 1.2
#define ap_name "MK312CONFIG-AP"

// The port to listen to UDP to, so devices can find the interface
#define udpdiscoveryport 8842
#define commport 8843

#define ledpin 1 //- pin to radio led (we use TX, since the system will output garbage on start, and we do not want to confuse the mk312)
#define rxpin 0 // rx pin to be used by the software implementation
#define resetwifipin 3 // The pin that needs to be pushed to ground to reset the wifi settings
#define txpin 2 // tx pin to be used by the software implementation

#define fail_checksum 1
#define fail_handshake_a 2
#define fail_handshake_b 3
#define fail_handshake_c 4
#define fail_reply 5
#define fail_peekreply 10
#define fail_pokereply 11

char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1]; //buffer to hold incoming packet,
bool statusled = false;

WiFiServer wifiServer(commport); // The wifiserver, that sends data and receives controls
WiFiUDP Udp; // The UDP Server that is used to tell the client the IP Address

void setStatusLed(bool status) {
  if (status) {
    digitalWrite(ledpin, HIGH);   // turn the LED on (HIGH is the voltage level)
  }
  else {
    digitalWrite(ledpin, LOW);   // turn the LED off (LOW is the voltage level)
  }
}

SoftwareSerial mySerial(rxpin, txpin, false); // RX, TX

byte mk312key = 0; // The key to be used to talk to the mk312
#define mk312hostkey 0 // The host key to be transmitted to the mk312
byte wifikey = 0;  // The key used when we are talked to from wifi

// Writes a byte to the mk312
void mk312write(byte b) {
  mySerial.write(b);
}

// Writes an encrypted byte to the mk312
void mk312write_enc(byte b) {
  mySerial.write(b ^ mk312key);
}

// Waits for a byte from the mk312 and returns it
int mk312read() {
  unsigned long timeout = millis() + 1000; // We wait for one second until we go into errorstate
        
  while (mySerial.available() == 0) {
    delay(10);
    if (millis() > timeout)
      return -1;
  } 
  return mySerial.read();
}

// Flashes an error code until the end of time
void errorstate(byte e) {
  while (true) {
    for (byte i=0;i<e;i++) {
      digitalWrite(ledpin, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(300);
      digitalWrite(ledpin, LOW);
      delay(150); 
    }
    delay(2000);
  }
}

// Internal Poke Command
void poker(int addr, byte b) {
  byte lo = addr >> 8;
  byte hi = addr & 0xff;
  mk312write_enc(0x4d);
  mk312write_enc(lo);
  mk312write_enc(hi);
  mk312write_enc(b);
  mk312write_enc((0x4d + lo + hi + b) % 256);
  if (mk312read()!=0x06) errorstate(fail_pokereply);
}

// Peeks an address from the memory of the device
byte peeker(int addr) {
  byte lo = addr >> 8;
  byte hi = addr & 0xff;
  mk312write_enc(0x3c);
  mk312write_enc(lo);
  mk312write_enc(hi);
  mk312write_enc((0x3c + lo + hi) % 256);
  if (mk312read()!=0x22) errorstate(fail_peekreply);
  byte val = mk312read();
  byte chk = mk312read();
  if (((val + 0x22) % 256) != chk) errorstate(fail_checksum);
  return val;
}

// Write to screen
void writeText(const char myMsg[]) {
  int len = strlen(myMsg);

  // display the message text...
  for (int i=0;i<len;i++) {
    poker(0x4180,myMsg[i]);
    poker(0x4181,64+i);
    poker(0x4070,0x13);
    while (peeker(0x4070) != 0xff) delay(1);
  }

  // ... then clear the rest of the screen's line with spaces
  for (int i=len;i<16;i++) {
    poker(0x4180,' ');
    poker(0x4181,64+i);
    poker(0x4070,0x13);
    while (peeker(0x4070) != 0xff) delay(1);
  }
}

// initializes wifi
void wifi_setup() {
  WiFiManager wifiManager;
  wifiManager.setDebugOutput(false);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.autoConnect(ap_name);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    statusled = !statusled;
    setStatusLed(statusled);
  }

  setStatusLed(false); 
  Udp.begin(udpdiscoveryport);
  wifiServer.begin();
}

void configModeCallback (WiFiManager *myWiFiManager) {
  writeText("WifiAP");
}

// Establishes or reestablishes communication with the mk312 device
void mk312_setup() {
  // Clear potential garbage from buffer
  delay(10);
  while (mySerial.available() > 0) mySerial.read();
  
  byte rep = 00;

  byte attempts = 12;
  while (attempts > 0) {
    attempts --;
    mk312write(0x00); // Initiate handshake
    mySerial.flush();
    delay(1); // Give the system time to answer
    rep = mk312read();
    if (rep == 0x07) break;
  }

  if (attempts == 0) errorstate(fail_handshake_a); // Failed waiting for 7

  mk312key = 0x00;

  // Set encryption key
  mk312write(0x2f); // Set key command
  mk312write(0x00); // To keep things simple we will use 00 as a key
  mk312write(0x2f); // Checksum (no key, so really just the commmand)
  mySerial.flush(); // Send data

  rep = mk312read();
  byte boxkey = mk312read();
  byte check = mk312read();

  if (rep != 0x21) errorstate(fail_handshake_b); // handshake fail
  if (check != (rep + boxkey)) errorstate(fail_checksum); // checksum fail 

  // Store the encryption key for later use
  mk312key = boxkey ^ 0x55;
}

void setup() {
  // Check if WIFI needs a reset
  pinMode(resetwifipin, INPUT);

  // Initialize serial
  pinMode(ledpin, OUTPUT);
  
  mySerial.begin(19200);
  pinMode(txpin, OUTPUT);
  pinMode(rxpin, INPUT); // For some reason the ESP insists on a pullup for the RX pin, which will then not be understood, so... we rectify that.

  // Setup communications with the MK312
  mk312_setup();

  // Ready for incoming connections
  wifi_setup();
  IPAddress ip = WiFi.localIP();
  char s[16];
  sprintf(s, ">%i.%i.%i.%i", ip[0],ip[1],ip[2],ip[3]);
  writeText(s);
}

// Checks if the AP button is pressed
void checkForAP() {
    bool resetWifi = !digitalRead(resetwifipin);
    if (resetWifi) {
      writeText("WifiAP");
      WiFiManager wifiManager;
      wifiManager.startConfigPortal("OnDemandAP");
    }
}

void loop() {
  handleUDP();
  handleTCPIP();
  checkForAP();
}
bool toggle = false;

int wifiEncryption = -1; // Do we use encryption on wifi side?

// Waits for a byte from wifi and returns it
byte wifiread(WiFiClient client) {
  unsigned long timeout = millis() + 1000; // We wait for one second until we go into errorstate
        
  while (client.available() == 0) {
    delay(10);
    if (millis() > timeout)
      return -1;
  } 
  if (wifiEncryption == 1)
    return client.read() ^ wifikey; // Decrypt
  else
    return client.read();
}

// Handles the incoming TCPIP requests
void handleTCPIP() {
  byte cmd = 0; // conmmand read
  byte val1 = 0; // Value
  byte hi = 0; // Hi address
  byte lo = 0; // Lo address
  byte chk = 0; // Checksum
  byte rep = 0; // Reply byte
  byte idx = 0; // Index to buffer
  byte readbuf[16]; // Read buffer for write byte passthrough
  long chksum = 0; // Checksum for readbuffer
  
  bool status = false;
  long next_blink = 0;
  
  WiFiClient client = wifiServer.available();
 
  if (client) {
    client.setNoDelay(true);
    //Serial.begin(19200);

    setStatusLed(true);

    wifiEncryption = 1;
    wifikey = 0;
        
    while (client.connected()) {
        WiFiClient new_client = wifiServer.available();
        if (new_client) {
          client.stop();
          client = new_client;
          wifikey = 0;
        }
        
        // Check if a control message has been sent
        while (client.available()>0) {
          cmd = wifiread(client);
          status = !status;
          setStatusLed(status);

          // ping command is replied to with 07
          if (cmd == 0x00) {
            wifikey = 0;
            client.write(0x07);
            client.flush();
            continue;
          }

          // Set key command
          if (cmd == 0x2f) { // Set key command
            //while (client.available()!=2) delay(100);
            val1 = wifiread(client);
            chk = wifiread(client);

            // If key and checksum are 0x42 encryption is disabled
            if ((val1 == 0x42) && (chk == 0x42)) {
              wifiEncryption = 0;
              client.write(0x69); // Reply code, key accepted
              client.flush();
              continue;              
            }

            if (chk != ((val1 + cmd) % 256)) {
              client.write(0x07); // Reply code, key accepted
              client.flush();
              continue;              
            }
            wifikey = val1 ^ 0x55;
            client.write(0x21); // Reply code, key accepted
            client.write(0x00); // Our own "box" key, which for simplicity will always be 0
            client.write(0x21); // The checksum
            client.flush();
            continue;
          }
          // Read byte command
          if (cmd == 0x3c) { // read byte command
            lo = wifiread(client);
            hi = wifiread(client);
            chk = wifiread(client);

            if (((cmd + lo + hi) % 256) != chk) {
              client.write(0x07); // Wrong checksum
              client.flush();
              continue;
            }

            // Command checks out, send it to device
            mk312write_enc(cmd);
            mk312write_enc(lo);
            mk312write_enc(hi);
            mk312write_enc(chk);

            // Handle reply
            byte rep = mk312read();
            byte val1 = mk312read();
            byte chk = mk312read();

            // Verify reply
            if (((rep + val1) % 256) != chk) {
              client.write(0x07); // Wrong checksum
              client.flush();
              continue;
            }

            client.write(rep);
            client.write(val1);
            client.write(chk);
            client.flush();
            continue;
          }

          // Write byte command implementation
          if ((cmd & 0x0F) == 0x0D) { // write byte command
            val1 = (cmd & 0xF0) >> 4; // Number of bytes to write

            hi = wifiread(client);
            lo = wifiread(client);

            chksum = cmd + hi + lo;

            // Intercept bytes
            for (int i = 0; i < (val1 - 3); i++) {
              readbuf[i] = wifiread(client);
              chksum += readbuf[i];
            }

            chk = wifiread(client);

            //client.write(chk);
            //client.write(chksum % 256);
            
            // Make sure checksum is ok
            if ((chksum % 256) != chk) {
              client.write(0x07); // Wrong checksum
              client.flush();
              continue;              
            }

            // Intercept key change by a poke command
            // TODO: Theoretically someone could write to 4213 by a command writing several bytes
            // But nobody would do that... right? RIGHT?
            if ((val1 == 4) && (hi == 0x42) && (lo == 0x13)) {
                wifikey = readbuf[0];
                client.write(0x06); // OK, we changed the local key
              continue;
            } 
            mk312write_enc(cmd);
            mk312write_enc(hi);
            mk312write_enc(lo);
            for (int i = 0; i < (val1 - 3); i++) {
              mk312write_enc(readbuf[i]);
            }
            mk312write_enc(chk);

            rep = mk312read();
            client.write(rep);
            client.flush();
            continue;
          }
          wifikey=0;
          client.write(0x07);
          client.flush();
          continue;
        }
    }
 
    client.stop();
  }
}

void handleUDP() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    // read the packet into packetBufffer
    int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    packetBuffer[n] = 0;

    if(strcmp(packetBuffer, "ICQ-MK312") == 0) {
      // send a reply, to the IP address and port that sent us the packet we received
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      IPAddress ip = WiFi.localIP();
      Udp.write(ip[0]);
      Udp.write(ip[1]);
      Udp.write(ip[2]);
      Udp.write(ip[3]);
      Udp.endPacket(); // "flush" the output as we're sending the packet UDP now
    }
  }

}
