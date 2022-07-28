/********************************************************************/
/* DS18B20_OLED_I2C.ino
 * Author: Gabriel Christopher
 * Date: 2022/07/28
 * Function: Displays the coolant temperature and controls the speed of the fan accordingly. 
 * Miniature model of a radiator.
 * This code is for a 128x64 pixel Monochrome OLED display 
 * based on SSD1306/SH1106 drivers using I2C to communicate.
 * 3 pins are required to interface (two I2C and one reset).
 * Version: 1.1 */
/********************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <OneWire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Fonts/Waukegan_LDO5pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans6pt7b.h>

#define FAN 5            // initialize pin for the fan
#define BUZZER 11        // initialize pin for the buzzer
#define Relay 9          // initialize pin for the relay

// Let's create the OLED display
// uncomment the below lines if you are using an SH1106 based display
/*
#define OLED_RESET -1           // Reset pin # (or -1 if sharing Arduino reset pin)
#define i2c_Address 0x3c        // initialize with the I2C addr 0x3c Typically Adafruit OLED's
Adafruit_SH1106G d1 = Adafruit_SH1106G(128, 64, &Wire, OLED_RESET);  //let's define our display
*/
// uncomment the below lines if you are using an SSD1306 based display
/*
#define OLED_RESET  4           // Reset pin # (or -1 if sharing Arduino reset pin)
#define i2c_Address 0x3D        // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 d1(128, 64, &Wire, OLED_RESET);
*/
//--------------------------------------------------------------------------------------------

byte i,present=0,type_s,data[12],addr[8];
float celsius, fahrenheit;
boolean buzzer_state=0,buzzer_state2=0,has_shown=0;
int xx=40,yy=10,tt=55;
const int onDuration = 150;
const int periodDuration = 50;
unsigned long lastPeriodStart;

OneWire  ds(2);  // (a 4.7K resistor is necessary)
//---------------------------------------------------------------------------------------------------------------------
// hex array needed for the icons.
const unsigned char PROGMEM fan [] = {     // storing the array in progmem to save dynamic ram
0x00, 0x00, 0x00, 0x00, 0x1E, 0x78, 0x3E, 0xFC, 0x3E, 0xFC, 0x3C, 0xFC, 0x3F, 0xDC, 0x1F, 0xC0,
0x03, 0xF8, 0x3B, 0xFC, 0x3F, 0x3C, 0x3F, 0x7C, 0x3F, 0x7C, 0x1E, 0x78, 0x00, 0x00, 0x00, 0x00
};
//--------------------------------------------------------------------------------------------------------------------
const unsigned char PROGMEM warning[] = {
0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x02, 0x40, 0x02, 0x40, 0x04, 0x20, 0x00, 0x00, 0x08, 0x10,
0x00, 0x00, 0x10, 0x08, 0x20, 0x04, 0x20, 0x04, 0x40, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00
};
//-----------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------
const unsigned char PROGMEM coolantfull [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80,
0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00,
0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00,
0x00, 0x00, 0x01, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80,
0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00,
0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFC, 0x00, 0x00,
0x00, 0x00, 0x01, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80,
0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00,
0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFC, 0x00, 0x00,
0x00, 0x00, 0x01, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80,
0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00,
0x07, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xF8, 0x00, 0x00,
0x00, 0x00, 0x1F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFC, 0x00, 0x00, 0x3F, 0xFC, 0x3F, 0xFC,
0x3F, 0xFC, 0x1F, 0xFE, 0x3F, 0xFC, 0x7F, 0xF8, 0x00, 0x00, 0x1F, 0xF8, 0x00, 0x00, 0x00, 0x00,
0x1F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x40, 0x03, 0x80, 0xF8, 0x1F, 0x01, 0xC0, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x7F,
0x0F, 0xF0, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
//---------------------------------------------------------------------------------------------------------------
//const unsigned char* arr[]= {f0,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14};
void setup(void){
  pinMode(FAN,OUTPUT);
  pinMode(BUZZER,OUTPUT);
  pinMode(Relay,OUTPUT);
  digitalWrite(Relay,0);
  analogWrite(FAN,0);
  digitalWrite(BUZZER,0);
  delay(250);                                    // wait for the OLED to power up
  //d1.begin(SSD1306_SWITCHCAPVCC, i2c_Address); // uncomment if you are using an SSD1306 based display
  //d1.begin(i2c_Address, true);                 // uncomment if you are using an SH1106 based display        
  d1.clearDisplay();                             // clear the buffer
  d1.setTextColor(SH110X_WHITE);                 // SSD1306_WHITE for SSD1306 & SH110X_WHITE for SH1106 
}
void loop(void){
  show_stats();
  check_coolant_temp();
  calculate_temp();
}
//--------------------------------------------------------------------
void show_stats(){
  d1.setContrast(0);                             // dim display
  d1.clearDisplay();
  d1.setFont();
  d1.setTextSize(2);
  d1.setCursor(98,20);
  d1.println((char)248);                         // degree character
  d1.drawBitmap(5,15,coolantfull,48,48,1);
  d1.setTextSize(1);
  d1.setFont(&FreeSans18pt7b);
  d1.setCursor(55,43);
  d1.print(int(celsius));
  d1.setTextSize(1);
  d1.setFont(&FreeSans12pt7b);
  d1.setCursor(107,43); //70,32
  d1.println("C");
  d1.display();
}
void calculate_temp(){
  if ( !ds.search(addr)) {
    d1.drawBitmap(0,0,warning,16,16,1);
    d1.setFont(&Waukegan_LDO5pt7b);
    d1.setCursor(7,11);
    d1.println('!');
    d1.display();
    ds.reset_search();
    delay(250);
    return;
  }
  if(ds.search(addr)){
    d1.clearDisplay();
    return;
  }
  if (OneWire::crc8(addr, 7) != addr[7]) { 
    Serial.println("CRC is not valid!");
    return;
  }
  ds.reset();
  ds.select(addr);
  ds.write(0x44);         // start conversion, with normal power mode at the end    
  //ds.write(0x44,1);     // start conversion, with parasite power mode at the end        
  delay(750);               
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);           // Read Scratchpad
  
  for ( i = 0; i < 9; i++) {   
    data[i] = ds.read();
  }
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3;
    if (data[7] == 0x10) {
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
}
//------------------------------------------------------------------------
void check_coolant_temp(){
  // Default (<185 F)
  if (celsius<30){
    analogWrite(FAN,0);
  }
  // Case-1 (185 F)
  if (celsius>=30 && celsius<32){
    analogWrite(FAN,85);
    d1.drawBitmap(112,0,fan,16,16,1);//112
    d1.display();
    if (buzzer_state==0){
       tone(BUZZER, 880,150);
       delay(100);
       tone(BUZZER, 1760,150); 
       delay(100);
       tone(BUZZER,0,150);  
       buzzer_state=1; 
       buzzer_state2=0; 
  }}
  // Case-2 (190 F)
  if (celsius>=32 && celsius<34) {
    analogWrite(FAN,140);
    d1.drawBitmap(112,0,fan,16,16,1);
    d1.display();   
  }
  // Case-3 (195 F)
  if (celsius>=34 && celsius<36) {
    analogWrite(FAN,170);
    d1.drawBitmap(112,0,fan,16,16,1);
    d1.display();
  }/*
  // Case-4 (200 F)
  if (celsius>=36 && celsius<40) {
    analogWrite(FAN,255);
    d1.drawBitmap(112,0,fan,16,16,1);
    d1.display();
    if (buzzer_state2==0){
       tone(BUZZER, 1760,150); 
       delay(100);
       tone(BUZZER,0,150);  
       buzzer_state=0;
       buzzer_state2=1;       
  }}*/
  // Case-5 (240 F)
  if (celsius>=36) {
    shut_down();
  }
}
//---------------------------------------------------------------------
void shut_down(){
    int c=60;
    d1.setContrast(0x7F);   // bright display
    d1.clearDisplay();
    d1.setFont(&FreeSans9pt7b);
    d1.setCursor(40, 25);
    d1.println("Engine");
    d1.setCursor(27, 50);
    d1.println("Overheat!");
    d1.display();
    for(int j=0;j<=8;j++){
    tone(BUZZER,1397,225); 
    delay(250);
    noTone(BUZZER);
    }
    //-------------------------------
    d1.clearDisplay();
    for(int i=60;i>=1;i--){
      if(i>10){
      d1.setFont(&FreeSans6pt7b);
      d1.setTextSize(1);
      d1.setCursor(1, 8);
      d1.println("Turn Off the Engine in..");
      d1.setFont(&FreeSans18pt7b);
      d1.setTextSize(2);
      d1.setCursor(27, 62);
      d1.println(i);
      d1.display();
      delay(999);
      d1.clearDisplay();
      }
      if (i<10){
        d1.setCursor(1,8);
        d1.setFont(&FreeSans6pt7b);
        d1.setTextSize(1);
        d1.setCursor(1, 8);
        d1.println("Turn Off the Engine in..");
        d1.setFont(&FreeSans18pt7b);
        d1.setTextSize(2);
        d1.setCursor(45, 62);
        d1.println(i);
        d1.display();
        delay(999);
        d1.clearDisplay();}
      if (i==c){
        if (millis() - lastPeriodStart >= periodDuration) {
            lastPeriodStart += periodDuration;
            tone(BUZZER, 698, onDuration); //1397
            c-=10;
            //d1.clearDisplay();
            continue;}} 
    }
    //-------------------------------
    d1.clearDisplay();
    d1.setFont(&Waukegan_LDO5pt7b);
    d1.setTextSize(1);
    d1.setCursor(3, 25); //35,40
    d1.print("Shutting Down in..");
    d1.setCursor(50, 50);//35,40
    d1.print("3");
    d1.display();
    tone(BUZZER, 1976,150); 
    delay(900);
    noTone(BUZZER);
    d1.print("2");
    d1.display();
    tone(BUZZER, 1976,150); 
    delay(900);
    noTone(BUZZER); 
    d1.print("1");
    d1.display();
    tone(BUZZER, 1976,150); 
    delay(900);
    noTone(BUZZER);
    tone(BUZZER, 1976,1000);
    delay(999);
    digitalWrite(Relay,1);
}
//------------------------------------------------------
//3.699+0.25=3.949s
//59.94+3.949=63.889s
