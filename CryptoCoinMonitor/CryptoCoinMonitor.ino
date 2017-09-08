#include <SPI.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
 
Adafruit_SSD1306 display = Adafruit_SSD1306();
 
#if defined(ESP8266)
  #define BUTTON_A 0
  #define BUTTON_B 16
  #define BUTTON_C 2
  #define LED      0
#elif defined(ESP32)
  #define BUTTON_A 15
  #define BUTTON_B 32
  #define BUTTON_C 14
  #define LED      13
#elif defined(ARDUINO_STM32F2_FEATHER)
  #define BUTTON_A PA15
  #define BUTTON_B PC7
  #define BUTTON_C PC5
  #define LED PB5
#elif defined(TEENSYDUINO)
  #define BUTTON_A 4
  #define BUTTON_B 3
  #define BUTTON_C 8
  #define LED 13
#elif defined(ARDUINO_FEATHER52)
  #define BUTTON_A 31
  #define BUTTON_B 30
  #define BUTTON_C 27
  #define LED 17
#else // 32u4, M0, and 328p
  #define BUTTON_A 9
  #define BUTTON_B 6
  #define BUTTON_C 5
  #define LED      13
#endif
 
#if (SSD1306_LCDHEIGHT != 32)
 #error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

const char* ssid     = "---";
const char* password = "---";
const char* host = "blockchain.info";
const int httpsPort = 443;

unsigned long count;
float btc;
float eth;
boolean refreshDisplay;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "8C 4C 5B 6C F9 6D 99 89 3D FC 6B F4 48 6E F6 77 A8 FF 5C 11";
WiFiClientSecure client;

void setup() {  
  Serial.begin(115200);
  
  Serial.println("Crypto Monitor Starting...");
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.clearDisplay();
  display.setCursor(0,0);
  display.display();
  display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.print("Crypto Monitor");
  display.display();
  delay(2000);
 
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
 
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("Connecting to WiFi");
  display.display();
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.print(".");
    display.display();
  }
  display.println("\nConnected!");
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("IP: " + WiFi.localIP());
  display.println("Waiting for server...");
  display.display();

  count = millis()-(20*60*1000);
  btc = 0.0;
  eth = 0.0;
  refreshDisplay = true;
}
 
 
void loop() {
  //if (! digitalRead(BUTTON_B)) display.print("B");
  //if (! digitalRead(BUTTON_C)) display.print("C");
  if (! digitalRead(BUTTON_A)) 
  {
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println("IP:"); 
    display.setTextSize(1);
    display.println(WiFi.localIP());
    display.display();
    refreshDisplay = true;
  }
  else
  {
    if (millis() - count > (10*60*1000))
    {
      count = millis();
      btc = 1/getCoinValue("/tobtc?currency=USD&value=1");
      eth = 1/getCoinValue("/toeth?currency=USD&value=1");
      refreshDisplay = true;
    }
    
    if (refreshDisplay)
    {
      display.clearDisplay();
      display.setCursor(0,0);
      display.setTextSize(1); display.print("BTC: "); display.setTextSize(2); display.println(btc);
      display.setTextSize(1); display.print("ETH: "); display.setTextSize(2);display.println(eth);
      display.display();
      refreshDisplay = false;
    }
  }

  yield();
}

float getCoinValue(String url)
{
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return 0.0;
  }
  else
  {
    if (!client.verify(fingerprint, host)) 
      return 0.0;
      
    //String url = "/tobtc?currency=USD&value=1";
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: bitcoinAlert\r\n" +
                 "Content-Type: text/html;charset=UTF-8\r\n" +
                 "Connection: close\r\n\r\n");
  
    while (client.connected()) 
    {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        client.readStringUntil('\n');
        break;
      }
    }
  
    String val = client.readStringUntil('\n');
    val.trim();
    return val.toFloat();
  }
}

