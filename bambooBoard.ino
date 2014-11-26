

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
char* projects[] = {"MYFOOT-CI", "GENESIS-CI", "PITA-CI", "API-STG"};
const int projectCount = 4;
int lastStatus[4];
int status[4];


const int buildsToScan = 3;

const int UNSET = 0;
const int SUCCESS = 1;
const int FAILURE = -1;
const int FROZEN = 2;
const int THAWED = 3;
const int SCANNED = 4;
const int SCANNING = 5;
const int ENABLED = 1;
const int DISABLED = -1;
const char OPEN_JSON = '{';
const char CLOSE_JSON = '}';

const int boardRows = 8;
const int boardColumns = 5;
int buildResult[8][5];





void setup() {

  for (int i = 0; i < projectCount; i++) {
    lastStatus[i] = UNSET;
    status[i] = UNSET;
  }

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for Leonardo only
  //}

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
  delay(250);
  Serial.println("connecting...");
  Serial.println("v.1.12");
  // if you get a connection, report back via serial:

  //init the board

}

void loop()
{
  for (int i = 0; i < projectCount; i++) {

    Serial.print(String(i+1));
    Serial.print(" of ");
    Serial.print(projectCount);
    Serial.print(" ");
    Serial.print(projects[i]);
    Serial.print(" ");

    //turn on polling pixel
    //display who we are polling
    int restore = buildResult[2*i][0];
    buildResult[2*i][0] = SCANNING;
    displayBuilds();

    int builds[buildsToScan] = {0,0,0};
    status[i] = getStatus(projects[i], builds);

    //turn off the polling pixel
    buildResult[2*i][0] = restore;
    displayBuilds();
    //set first two on freeze thaw


    //set the last 3 project builds pixels, but only if we updated successfully
    if (status[i] == ENABLED || status[i] == DISABLED) {
        for (int b = 0; b < buildsToScan; b++) {
          if (builds[b] == UNSET) {
            break; //out of data, incomplete scan
          }
          buildResult[2*i][b+2] = builds[b];
          buildResult[(2*i)+1][b+2] = builds[b];
        }
    }
    // did not update project successfully
    //so continue to next
    if (status[i] == UNSET) {
        Serial.println(" (Cached)");
        continue;
    }

    String message = (status[i]) ? " Enabled" : " Disabled";
    Serial.println(message);

    /*
     * run the dance of freeze/thaw
     */

    //project went thaw
    if (lastStatus[i] == DISABLED && status[i] == ENABLED) {
      theaterChase(strip.Color(255, 0, 0), 50); // red
    }
    //project frozen
    if (lastStatus[i] == ENABLED && status[i] == DISABLED) {
      theaterChase(strip.Color(0, 0, 255), 50); // blue
    }
    //update last status cache
    lastStatus[i] = status[i];

    //Display the walker on the project that was just updated
    buildResult[2*i][0] = (lastStatus[i] > 0) ? THAWED : FROZEN;
    buildResult[(2*i)+1][0] = (lastStatus[i] > 0) ? THAWED : FROZEN;
    buildResult[(2*i)+1][1] = (lastStatus[i] > 0) ? THAWED : FROZEN;

    for (int tdelay = 0; tdelay < 16; tdelay++) {
        switch (tdelay % 4) {
          case 0:
            buildResult[2*i][1] = (lastStatus[i] > 0) ? THAWED : FROZEN;
            buildResult[2*i][0] = SCANNED;
            break;
          case 1:
            buildResult[2*i][0] = (lastStatus[i] > 0) ? THAWED : FROZEN;
            buildResult[(2*i)+1][0] = SCANNED;
            break;
          case 2:
            buildResult[(2*i)+1][0] = (lastStatus[i] > 0) ? THAWED : FROZEN;
            buildResult[(2*i)+1][1] = SCANNED;
            break;
          case 3:
            buildResult[(2*i)+1][1] = (lastStatus[i] > 0) ? THAWED : FROZEN;
            buildResult[2*i][1] = SCANNED;
            break;
        }
        displayBuilds();
        delay(75);
    }

    //turn the last pixel back to normal
    buildResult[2*i][1] =  (lastStatus[i] > 0) ? THAWED : FROZEN;
    displayBuilds();
}




  //turn off the dance lights

//  Serial.print("Sleeping ");
//  for (int i = 0; i < 10; i++) { //sleep for next round
//    delay(1000);
//    //flickerPixels();
//    Serial.print(".");
//
//  }
//  Serial.println();


  // if the server's disconnected, stop the client:

}
/**
slee function
**/
//void flickerPixels() {
//    int totalLitPixels = 5;
//    int pixel[5] = {-1};
//    for (int i = 0; i < totalLitPixels; i++) {
//      int newPixel = rand() % strip.numPixels();
//      bool collision = false;
//      for (int j = 0; j < i+1; j++) {
//        if (pixel[j] == newPixel) { //don't add twice
//          collision = true;
//        }
//      }
//      if (collision == false) {
//        pixel[i] = newPixel;
//      }
//
//    }
//
//    uint32_t savedPixelColor[totalLitPixels];
//    for (int i = 0; i < 4;  i++) { //cycles
//      for (int j = 0; j < totalLitPixels;  j++) {
//        if (pixel[j] != -1) {
//            savedPixelColor[j] = strip.getPixelColor(pixel[j]);
//            strip.setPixelColor(pixel[j], strip.Color(rand() % 127, rand() % 127, rand() % 127));
//        }
//      }
//      strip.show();
//      delay(75);
//      for (int j = 0; j < totalLitPixels;  j++) {
//        if (pixel[j] != -1) {
//          strip.setPixelColor(pixel[j], 0);
//        }
//      }
//      strip.show();
//      delay(75);
//
//      for (int j = 0; j < totalLitPixels;  j++) {
//        if (pixel[j] != -1) {
//          strip.setPixelColor(pixel[j], savedPixelColor[j]); //set color back
//        }
//      }
//      strip.show();
//
//    }
//}

void displayBuilds()
{
  uint32_t savedPixelColor = strip.Color(0,127,127);

  for (int x = 0;  x < boardRows; x++) {
    for (int y = 0; y < boardColumns; y++) {
      switch (buildResult[x][y]) {
        case FAILURE:
          savedPixelColor = strip.Color(90,0,0);
          break;
        case SUCCESS:
          savedPixelColor = strip.Color(0,90, 0);
          break;
        case FROZEN:
          savedPixelColor = strip.Color(0,0,90);
          break;
        case THAWED:
          savedPixelColor = strip.Color(90,35,6);
          break;
        case SCANNED:
          savedPixelColor = strip.Color(90,90,90);
          break;
        case SCANNING:
          savedPixelColor = strip.Color(50,50,50);
          break;
        case UNSET:
        default:
          savedPixelColor = strip.Color(15,15,15);
          break;
      }

      strip.setPixelColor(translatePoint(x,y), savedPixelColor);
    }
  }

  strip.show();
}

int translatePoint(int x, int y) {
  return y * boardRows + x  ;
}

void displayNormal()
{
  for (int i = 0; i < PIXELS; i++) {
    strip.setPixelColor(i, 0);
  }
  strip.show();
}


int getStatus(String project, int builds[5])
{
  //listener.reserve(1024);
  int returnVal = UNSET;

  //client.stop();

  if (client.connect(server, 80)) {
      //Serial.println("connected");
      // Make a HTTP request:
      String url = "GET /rest/api/latest/result/";
      url.concat(project);
      url.concat(" HTTP/1.1");
      client.println(url);
      client.println("Host: bamboo.sparefoot.com");
      client.println("User-Agent: arduino-ethernet");
      //client.println("Content-Type: application/json");
      client.println("Accept: application/json");
      client.println("Connection: close");
      client.println();

  } else {
      Serial.println();
      Serial.println("connection failed");
      return returnVal;
  }
   //this makes it freakin work
  //delay(100);
  String freezeListener;

  int buildsFound = 0;
  int noCharCount = 0;
  bool pastHeaders = false;

  while (client.connected()) {
    while (client.available()) {
      char inchar = client.read();
      //Serial.print(inchar);
      //see if frozen
      if (! pastHeaders) { //zip past headers
        if (inchar == OPEN_JSON) {
            pastHeaders = true;
        }
        continue;
      }

      freezeListener += inchar;

      //kill the string after we ind some stuff
      if (inchar == CLOSE_JSON) {
        freezeListener.remove(0, freezeListener.length());
      }

      if (returnVal == UNSET && freezeListener.indexOf("false") > 0) {
           returnVal = DISABLED;
           freezeListener.remove(0, freezeListener.length());

      } else if (returnVal == UNSET && freezeListener.indexOf("true") > 0) {
           returnVal = ENABLED;
           freezeListener.remove(0, freezeListener.length());
      }


      if (returnVal != UNSET) {
          if (buildsFound < buildsToScan && freezeListener.indexOf("\"state\":\"Successful\"") > 0) {
              builds[buildsFound] = SUCCESS;
              buildsFound++;
              freezeListener.remove(0, freezeListener.length());
              Serial.print(" Success");
          }
          if (buildsFound < buildsToScan && freezeListener.indexOf("\"state\":\"Failed\"") > 0) {
              builds[buildsFound] = FAILURE;
              buildsFound++;
              freezeListener.remove(0, freezeListener.length());
              Serial.print(" Failure");
          }
      }
      if (buildsFound >= buildsToScan && returnVal != UNSET) { //we are done
        Serial.println(" Finish");
        client.stop();
      }
      noCharCount = 0;
    }
    //break if we are stalled
    delay(10);
    noCharCount++;
    if (noCharCount > 600) {
      //Serial.println(freezeListener);
      //Serial.println(" Stalled");
      client.stop();
    }
  }
  //call it now and later
  client.stop();

  return returnVal;
  //return builds;
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<20; j++) {  //do 10 cycles of chasing
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

