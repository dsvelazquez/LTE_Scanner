#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <HardwareSerial.h>

#define FIRMWARE_VERSION 1.0

#define TFT_CS         5
#define TFT_RST        26 
#define TFT_DC         25
#define BUTTON         35

#define RADIO_RESET    34
#define RXD 9
#define TXD 10
#define DTR 34

#define TEXT_NONE       "No Signal"
#define TEXT_POOR       "Poor"
#define TEXT_FAIR       "Fair"
#define TEXT_GOOD       "Good"
#define TEXT_EXCELLENT  "Excellent"

typedef struct{
    String CSQText;
    String SINRText;
    String RSRPText;
    String RSRQText;
    String RSSIText;
    String Network;
}text_Details;

text_Details Details;
 
// For 1.44" and 1.8" TFT with ST7735 (including HalloWing) use:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
    
void setup(void) {
  setCpuFrequencyMhz(80); 

  pinMode(BUTTON, INPUT);    // declare pushbutton as input
  //pinMode(DTR, OUTPUT);
  //digitalWrite(DTR, LOW);    // Set DTR low

  
  Serial.begin(9600);
  Serial.print("Hello! Adafruit ST77XX rotation test");

  tft.initR(INITR_144GREENTAB);   // initialize a ST7735
  Serial.println("init");

  tft.setTextWrap(false); // Allow text to run off right edge
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(tft.getRotation()+1);

  Serial1.begin(115200);  // UART1 on ESP32

  //digitalWrite(RADIO_RESET, LOW);
  //delay(1500);
  //digitalWrite(RADIO_RESET, HIGH);
  delay(30000);            // Delay 30 seconds. This is necessary for the Sprint radio to boot up.
}

void loop(void) {

 // Display contents to LCD on button press
 if(digitalRead(BUTTON) == HIGH)
 {
    dispLTE();
 }

}

// TYPE:
// RSSI = 3, RSRP = 2, RSRQ = 4, SINR = 15
String getParameter(int type)
{
  String response, parameter;
  int orig, newpos, count=0, i=0, SINRint;
    
  Serial1.write("AT#RFSTS\r");              // Get SNR
  Serial1.flush();
  delay(50);
     
  while (Serial1.available()) {             // Receive data
      response = Serial1.readString();
  }

  // Parse string for SNR
  orig = response.indexOf(',');    // First instance
  parameter = response.substring(0,orig);
  while(count != type)
  {
    newpos = response.indexOf(',', orig +1);      // look for next token
    parameter = response.substring(orig + 1, newpos);   // Extract substring
    orig = newpos;                                // Set new to original

    count++;
  }

  return parameter;
}

String getRSRP()
{
  String _RSRP, text;
  String response;
  int orig, newpos, count=0, i=0, _RSRPInt;

  _RSRP = getParameter(2);                           // 2 = RSRP
  _RSRPInt = _RSRP.toInt();                          // Convert RSRP value string to int

  if(_RSRPInt < -100)
    Details.RSRPText = TEXT_POOR;
  else if(_RSRPInt < -90)
    Details.RSRPText = TEXT_FAIR;
  else if(_RSRPInt <= -80)
    Details.RSRPText = TEXT_GOOD;
  else
    Details.RSRPText = TEXT_EXCELLENT;

       
  _RSRP = "RSRP: " + _RSRP;
  
  return _RSRP;
}

String getRSRQ()
{
  String _RSRQ, text;
  String response;
  int orig, newpos, count=0, i=0, _RSRQInt;

  _RSRQ = getParameter(4);                           // 2 = RSRQ
  _RSRQInt = _RSRQ.toInt();                          // Convert RSRP value string to int

  if(_RSRQInt < -100)
    Details.RSRQText = TEXT_POOR;
  else if(_RSRQInt < -90)
    Details.RSRQText = TEXT_FAIR;
  else if(_RSRQInt <= -80)
    Details.RSRQText = TEXT_GOOD;
  else
    Details.RSRQText = TEXT_EXCELLENT;

       
  _RSRQ = "RSRQ: " + _RSRQ;
  
  return _RSRQ;
}
String getRSSI()
{
  String _RSSI, text;
  String response;
  int orig, newpos, count=0, i=0, _RSSIInt;

  _RSSI = getParameter(3);                           // 3 = RSSI
  _RSSIInt = _RSSI.toInt();                          // Convert RSSI value string to int

  if(_RSSIInt <= -95)
    Details.RSSIText = TEXT_NONE;
  else if(_RSSIInt < -85)
    Details.RSSIText = TEXT_POOR;
  else if(_RSSIInt < -75)
    Details.RSSIText = TEXT_FAIR;
  else if(_RSSIInt <= -65)
    Details.RSSIText = TEXT_GOOD;
  else
    Details.RSSIText = TEXT_EXCELLENT;

       
  _RSSI = "RSSI: " + _RSSI;
  
  return _RSSI;
  
}

// Use string tokenizer to retrieve SNR, after 15 commas
String getSINR()
{
  String SINR, text;
  String response;
  int orig, newpos, count=0, i=0;
  float SINRint;

  SINR = getParameter(15);                           // 15 = SINR
  //
  SINR = SINR.substring(0,3);                       // Parse out the 'OK'
  SINRint = SINR.toFloat();                           // Convert SNR value string to int
  SINRint = -20 + .2*SINRint;                       // Convert to dB
  
  if(SINRint < 0)
    Details.SINRText = TEXT_POOR;
  else if(SINRint < 13)
    Details.SINRText = TEXT_FAIR;
  else if(SINRint < 20)
    Details.SINRText = TEXT_GOOD;
  else //if(SINRint <= 250)
    Details.SINRText = TEXT_EXCELLENT;
 
  SINR = "SINR: " + String(SINRint);
  
  return SINR;
}

String getNetName()
{
  String response;
  int num;
  
  Serial1.write("AT#CGMM\r");              // Get Radio Model Name
  Serial1.flush();
  delay(50);
     
  while (Serial1.available()) {
      response = Serial1.readString();
  }

  if(response.substring(17,27) == "LE910C1-NS")
    Details.Network = "SPRINT";
  else
    Details.Network = "NONE";
  
  return Details.Network;
}

void dispLTE()
{
  String nresponse;
  int num;
  
  tft.fillScreen(ST77XX_BLACK);

  // TITLE
  tft.setCursor(25, 10);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("4G LTE SCANNER");

  // FIRMWARE VERSION
  tft.setCursor(50, 25);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("V");
  tft.setCursor(55, 25);
  tft.println(FIRMWARE_VERSION);

  // RSRP
  tft.setCursor(2, 45);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);  
  tft.println(getRSRP());
  
  // RSRP Response
  tft.setCursor(70, 45);
  tft.setTextSize(1);
  if(Details.RSRPText == TEXT_EXCELLENT)
    tft.setTextColor(ST77XX_GREEN);
  else if(Details.RSRPText == TEXT_GOOD)
    tft.setTextColor(ST77XX_YELLOW);
  else if(Details.RSRPText == TEXT_FAIR)
    tft.setTextColor(ST77XX_ORANGE);
  else if(Details.RSRPText == TEXT_POOR)
    tft.setTextColor(ST77XX_RED);
    
  tft.println(Details.RSRPText); // print RSRP value and test: Poor, Ok, Good, Excellent

  // RSRQ
  tft.setCursor(2, 60);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);  
  tft.println(getRSRQ());
  
  // RSRQ Response
  tft.setCursor(70, 60);
  tft.setTextSize(1);
  if(Details.RSRQText == TEXT_EXCELLENT)
    tft.setTextColor(ST77XX_GREEN);
  else if(Details.RSRQText == TEXT_GOOD)
    tft.setTextColor(ST77XX_YELLOW);
  else if(Details.RSRQText == TEXT_FAIR)
    tft.setTextColor(ST77XX_ORANGE);
  else if(Details.RSRQText == TEXT_POOR)
    tft.setTextColor(ST77XX_RED);
    
  tft.println(Details.RSRQText); // print RSRQ value and test: Poor, Ok, Good, Excellent
  
  // RSSI
  tft.setCursor(2, 75);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);  
 // nresponse = response.substring(15,17);  // GET CSQ value
 // num = nresponse.toInt();                // Convert CSQ value string to int
  tft.println(getRSSI());
  
  //  RSSI RESPONSE
  tft.setCursor(70, 75);
  //tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
  if(Details.RSSIText == TEXT_EXCELLENT)
    tft.setTextColor(ST77XX_GREEN);
  else if(Details.RSSIText == TEXT_GOOD)
    tft.setTextColor(ST77XX_YELLOW);
  else if(Details.RSSIText == TEXT_FAIR)
    tft.setTextColor(ST77XX_ORANGE);
  else if(Details.RSSIText == TEXT_POOR)
    tft.setTextColor(ST77XX_RED);
  else if(Details.RSSIText == TEXT_NONE)
    tft.setTextColor(ST77XX_WHITE);
    
  tft.println(Details.RSSIText); // print RSSI value and test: Poor, Fair, Good, Excellent
  
  
  // SINR
  tft.setCursor(2, 90);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println(getSINR());

  // SINR RESPONSE
  tft.setCursor(70, 90);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
  if(Details.SINRText == TEXT_EXCELLENT)
    tft.setTextColor(ST77XX_GREEN);
  else if(Details.SINRText == TEXT_GOOD)
    tft.setTextColor(ST77XX_YELLOW);
  else if(Details.SINRText == TEXT_FAIR)
    tft.setTextColor(ST77XX_ORANGE);
  else if(Details.SINRText == TEXT_POOR)
    tft.setTextColor(ST77XX_RED);
  
  tft.println(Details.SINRText);// Print SNR dB value and text: Edge, Mid Cell, Good, Excellent

  // NETWORK NAME
  tft.setCursor(2, 105);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("Network: ");

  // NETWORK NAME RESPONSE
  tft.setCursor(70, 105);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println(getNetName());
  // Print network name. This usualy coincides with the radio module attached to the LTE Scanner 
  
}

void ExitDataMode()
{
  Serial1.write("+");
  Serial1.flush();
  Serial1.write("+");
  Serial1.flush();
  Serial1.write("+");
  Serial1.flush();
  delay(2000);

  //response = Serial2.read();
}
void rotateText() {
  
  //for (uint8_t i=0; i<4; i++) {
  if(digitalRead(BUTTON) == HIGH){

    delay(100);
    if(digitalRead(BUTTON) == HIGH){
      tft.fillScreen(ST77XX_BLACK);
      Serial.println(tft.getRotation(), DEC);
  
      tft.setCursor(0, 30);
      tft.setTextColor(ST77XX_RED);
      tft.setTextSize(1);
      tft.println("4G LTE SCANNER");
      tft.setTextColor(ST77XX_YELLOW);
      tft.setTextSize(2);
      tft.println("4G LTE SCANNER");
      tft.setTextColor(ST77XX_GREEN);
      tft.setTextSize(3);
      tft.println("4G LTE SCANNER");
      tft.setTextColor(ST77XX_BLUE);
      tft.setTextSize(4);
      tft.print(1234.567);
      //while (!Serial.available());
      //Serial.read();  Serial.read();  Serial.read();
    
      tft.setRotation(tft.getRotation()+1);
    }
  }
}

void rotateFillcircle(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST77XX_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.fillCircle(10, 30, 10, ST77XX_YELLOW);

    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateDrawcircle(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST77XX_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.drawCircle(10, 30, 10, ST77XX_YELLOW);
 
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();
  
    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateFillrect(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST77XX_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.fillRect(10, 20, 10, 20, ST77XX_GREEN);
 
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateDrawrect(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST77XX_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.drawRect(10, 20, 10, 20, ST77XX_GREEN);
 
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateFastline(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST77XX_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.drawFastHLine(0, 20, tft.width(), ST77XX_RED);
    tft.drawFastVLine(20, 0, tft.height(), ST77XX_BLUE);

    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateLine(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST77XX_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.drawLine(tft.width()/2, tft.height()/2, 0, 0, ST77XX_RED);
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotatePixel(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST77XX_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.drawPixel(10,20, ST77XX_WHITE);
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateTriangle(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST77XX_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.drawTriangle(20, 10, 10, 30, 30, 30, ST77XX_GREEN);
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateFillTriangle(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST77XX_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.fillTriangle(20, 10, 10, 30, 30, 30, ST77XX_RED);
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateRoundRect(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST77XX_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.drawRoundRect(20, 10, 25, 15, 5, ST77XX_BLUE);
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateFillRoundRect(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST77XX_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.fillRoundRect(20, 10, 25, 15, 5, ST77XX_CYAN);
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateChar(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST77XX_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.drawChar(25, 15, 'A', ST77XX_WHITE, ST77XX_WHITE, 1);
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}

void rotateString(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST77XX_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.setCursor(8, 25);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("Adafruit Industries");
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}
