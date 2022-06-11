/*
This uses FastLED and ArtnetWifi to use an ESP8266 NodeMCU to connect to a WiFi near you (edit credentials) and be able to receive DMX over ArtNet. 
This was build of some example online, but I forgot where it was. I will add some credits, whenever I find it back!

There is some logic to bounce between SSIDs, so you can collaborate with people and also have it connect to either your closed off IoT WiFi network (please, just do not think about putting this into the internet. Please.) or your debugging network.
You need to connect the D6 pin of the ESP8266 to the data line of your LED Strip (WS2812 or whatever, you can also change it) and also connect the ground to all the other grounds. How you provide power to the ESP8266 is up to you :) I like step-downs and decent PSUs, but I also use this to have ArtNet Nodes for raves and light installations.
*/
#include <ArtnetWifi.h>
#include <Arduino.h>
#include <FastLED.h>
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER

// Wifi settings
const char* ssid1 = "";
const char* ssid2 = "";
const char* password1 = "";
const char* password2 = "";

// LED settings
const int fixtures = 2; // fixture count and also total of universes
const int numLeds = 144 * fixtures; // LEDs per fixture - here u need to fiddle with it a lot. 144 LEDs per fixture will put each fixture in a new universe. - we hope...
const int numberOfChannels = numLeds * 3; // Total number of channels you want to receive (1 led = 3 channels)
const byte dataPin = 6;

const int bufferLeds = floor((512 - numberOfChannels / 3));

CRGB leds[numLeds + (bufferLeds * fixtures)];

// Art-Net settings
ArtnetWifi artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;

// search SSIDs that are baked in
int searchForSsid()
{
  Serial.print("Scan start ... ");
  Serial.print("We will prioritize first WiFi! Second WiFi is a backup and dev mode!");
  int n = WiFi.scanNetworks();
  Serial.print(n);
  int targetSsidIndex = 0;
  Serial.println(" network(s) found");
  for (int i = 0; i < n; i++)
  {
    Serial.println(WiFi.SSID(i));
    if (WiFi.SSID(i) == "your first wifi ssid") {
      targetSsidIndex = i;
    } else if (WiFi.SSID(i) == "your second wifi ssid" && targetSsidIndex == 0) {
      targetSsidIndex = i;  
    }
  }
  Serial.println();
  Serial.println(targetSsidIndex);

  delay(5000);

  return targetSsidIndex;
}

// connect to wifi – returns true if successful or false if not
bool ConnectWifi(void)
{
  bool state = true;
  int i = 0;
  String password = "";
  String ssid = WiFi.SSID(searchForSsid());
  if (ssid == ssid1) {
    password = password1;
  }
  if (ssid == ssid2) {
    password = password2;
  }

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20){
      state = false;
      break;
    }
    i++;
  }
  if (state){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Connection failed.");
  }

  return state;
}

void initTest()
{
  for (int i = 0 ; i < numLeds ; i++) {
    leds[i] = CRGB(127, 0, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0 ; i < numLeds ; i++) {
    leds[i] = CRGB(0, 127, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0 ; i < numLeds ; i++) {
    leds[i] = CRGB(0, 0, 127);
  }
  FastLED.show();
  delay(500);
  for (int i = 0 ; i < numLeds ; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  int universeShift = 0;
  sendFrame = 1;
  // set brightness of the whole strip
  if (universe == 15)
  {
    FastLED.setBrightness(data[0]);
    FastLED.show();
  }

  // Store which universe has got in
  if ((universe - startUniverse) < maxUniverses) {
    universesReceived[universe - startUniverse] = 1;
  }

  for (int i = 0 ; i < maxUniverses ; i++)
  {
    if (universesReceived[i] == 0)
    {
      //Serial.println("Broke");
      sendFrame = 0;
      break;
    }
  }

  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++)
  {
    int led = i + (universe - startUniverse) * (previousDataLength / 3);
    int pos = i * 3;
    if (led < numLeds) {
      leds[led] = CRGB(data[pos], data[pos + 1], data[pos + 2]);
    }
  }
  previousDataLength = length;
  if (sendFrame)
  {
    FastLED.show();
    // Reset universeReceived to 0
    memset(universesReceived, 0, maxUniverses);
  }
}

void setup()
{
  Serial.begin(115200);
  ConnectWifi();
  artnet.begin();
  FastLED.addLeds<WS2813, dataPin, GRB>(leds, numLeds);
  initTest();

  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop()
{
  // we call the read function inside the loop
  artnet.read();
}
