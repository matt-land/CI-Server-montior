

/**
http://bamboo.sparefoot.com/rest/api/latest/plan/MYFOOT-CI
enabled = "false"
<isActive>false
<isBuilding>
**/


/**
http://bamboo.sparefoot.com/rest/api/latest/result/MYFOOT-CI
buildState>SuccessFul/Failed
enabled="false"



STATE="Successful"
lifeCycleState="Finished"


/*
  Web client

 This sketch connects to a website (http://www.google.com)
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe, based on work by Adrian McEwen

 */

#include <SPI.h>
#include <Ethernet.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
const int PIXELS = 40;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "bamboo.sparefoot.com";    // name address for Google (using DNS)

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;
char* projects[] = {"MYFOOT-CI", "GENESIS-CI", "PITA-CI"};
int projectCount = 3;
int lastStatus[3];
int status[3];



void setup() {

for (int i = 0; i < projectCount; i++) {
  lastStatus[i] = -1;
  status[i] = -1;
}
  //turn off sd card
  //pinMode(4,OUTPUT);
  //digitalWrite(4,HIGH);

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

   // Initialize all pixels to 'off'
  strip.begin();
  strip.show();

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");
  Serial.println("v.1.02");
  // if you get a connection, report back via serial:

}

void loop()
{
  // if there are incoming bytes available
  // from the server, read them and print them:


  //bool builds[6];
  //bool status;

  for (int i = 0; i < projectCount; i++) {

    Serial.print(String(i+1));
    Serial.print(" of ");
    Serial.print(projectCount);
    Serial.print(" ");
    Serial.print(projects[i]);
    Serial.print(" ");
    status[i] = getStatus(projects[i]);
    if (status[i] == 1) {

      Serial.println("Enabled");
      if (lastStatus[i] == 1) {
        // was hot and still hot
      } else if (lastStatus[i] == 0) {
        //project went thaw
        theaterChase(strip.Color(255, 0, 0), 50); // White
      }

    } else if (status[i] == 0) {

      Serial.println("Disabled");
      if (lastStatus[i] == 1) {
        //project frooze
        theaterChase(strip.Color(0, 0, 255), 50); // blue
      } else if (lastStatus[i] == 0) {
        //was frooze stayed frooze
      }
    } else {
      if (lastStatus[i] == -1) {
              Serial.print("Unknown");
      } else if (lastStatus[i] == 1) {
            Serial.print("Enabled");
      } else {
          Serial.print("Disabled");
      }
      Serial.println(" (Stale)");
    }
    if (status[i] != -1) { //only update if we got a valid status
      lastStatus[i] = status[i];

    }
  }
  //turn off the dance lights
  displayNormal();
  Serial.print("Sleeping ");
  for (int i = 0; i < 10; i++) { //sleep for next round
    delay(500);
    Serial.print(".");
  }
  Serial.println();


  // if the server's disconnected, stop the client:

}

void displayNormal()
{
  for (int i = 0; i < PIXELS; i++) {
    strip.setPixelColor(i, 0);
  }
  strip.show();
}


int getStatus(String project)
{
  //listener.reserve(1024);
  int returnVal = -1;

  client.stop();

  if (client.connect(server, 80)) {
      //Serial.println("connected");
      // Make a HTTP request:
      String url = "GET /rest/api/latest/result/";
      url.concat(project);
      url.concat(" HTTP/1.1");
      client.println(url);
      client.println("Host: bamboo.sparefoot.com");
      client.println("User-Agent: arduino-ethernet");
      client.println("Connection: close");
      client.println();

  } else {
      Serial.println();
      Serial.println("connection failed");
      return returnVal;
  }
  delay(100);
  String listener;
   //this makes it freakin work
  char openXml = '<';
  char closeXml = '>';
  while (client.available()) {
    char inchar = client.read();
    //Serial.print(inchar);
    listener += inchar;

    if (listener.indexOf("false") > 0) {
         // Serial.print("False");
         // Serial.println(listener.indexOf("false"));
         returnVal = 0;

    } else if (listener.indexOf("true") > 0) {
         // Serial.print("True");
         // Serial.println(listener.indexOf("true"));
         returnVal = 1;
    }
    //kill the string if we didn't see anything goods
    if (inchar == closeXml) {
          //Serial.println(listener);
          listener.remove(0, listener.length());
    }

  }

    client.stop();


  return returnVal;
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

