#include <TinyGPS++.h>
#include <WiFi.h> //Connect to WiFi Network
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h> //Used in support of TFT Display
#include <string.h>  //used for some string handling and processing.

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

char network[] = "6s08";  //SSID for 6.08 Lab
char password[] = "iesc6s08"; //Password for 6.08 Lab

const uint8_t LOOP_PERIOD = 10; //milliseconds
uint32_t primary_timer = 0;
uint32_t posting_timer = 0;
float x,y,z; //variables for grabbing x,y,and z values

const char USER[] = "moises";

//Some constants and some resources:
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host

const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

const uint8_t PIN_1 = 16; //button 1
const uint8_t PIN_2 = 5; //button 2

HardwareSerial gps_serial(2);
TinyGPSPlus gps;

uint8_t old_val1; //for button edge detection!
uint8_t old_val2;
uint32_t timer;

void setup() {
  Serial.begin(115200);
  gps_serial.begin(9600,SERIAL_8N1,32,33);
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set color of font to green foreground, black background
  Serial.begin(115200); //begin serial comms
  delay(100); //wait a bit (100 ms)
  pinMode(PIN_1,INPUT_PULLUP);
  pinMode(PIN_2,INPUT_PULLUP);

  WiFi.begin(network,password); //attempt to connect to wifi
  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count<12) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.println(WiFi.localIP().toString() + " (" + WiFi.macAddress() + ") (" + WiFi.SSID() + ")");
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
  timer = millis();
  old_val1 = digitalRead(PIN_1);
  old_val2 = digitalRead(PIN_2);
}

void loop() {
  if (gps_serial.available()){
    while (gps_serial.available()) 
      gps.encode(gps_serial.read());      // Check GPS
  }
  uint8_t val1 = digitalRead(PIN_1);
  uint8_t val2 = digitalRead(PIN_2);
  if (val1 != old_val1 && val1==0){//changed and just pushed....
    Serial.println("PUSHED!");
    if(gps.location.isValid()){
      Serial.println("GOOD DATA");
      char body[200]; //for body;
      sprintf(body,"user=%s&lat=%f&lon=%f",USER,gps.location.lat(),gps.location.lng());//generate body, posting to User, 1 step
      int body_len = strlen(body); //calculate body length (for header reporting)
      sprintf(request_buffer,"POST http://608dev.net/sandbox/sc/moisest/dat/deliverable.py?%s HTTP/1.1\r\n",body);
      
      strcat(request_buffer,"Host: 608dev.net\r\n");
      strcat(request_buffer,"Content-Type: application/x-www-form-urlencoded\r\n");
      sprintf(request_buffer+strlen(request_buffer),"Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
      strcat(request_buffer,"\r\n"); //new line from header to body
      strcat(request_buffer,body); //body
      strcat(request_buffer,"\r\n"); //header
      Serial.println(request_buffer);
      do_http_request("608dev.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT,true);
      tft.fillScreen(TFT_BLACK); //fill background
      tft.setCursor(2,2,1); // set the cursor
      tft.println(response_buffer); //print the result
    }
    if (val2 != old_val2 && val2==0){//changed and just pushed....
    Serial.println("PUSHED!");
    if(gps.location.isValid()){
      Serial.println("GOOD DATA");
      char body[200]; //for body;
      sprintf(body,"user=%s&lat=%f&lon=%f",USER,gps.location.lat(),gps.location.lng());//generate body, posting to User, 1 step
      int body_len = strlen(body); //calculate body length (for header reporting)
      sprintf(request_buffer,"GET http://608dev.net/sandbox/sc/m99/lab04b/location_log.py?%s HTTP/1.1\r\n",body);
      
      strcat(request_buffer,"Host: 608dev.net\r\n");
      strcat(request_buffer,"Content-Type: application/x-www-form-urlencoded\r\n");
      sprintf(request_buffer+strlen(request_buffer),"Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
      strcat(request_buffer,"\r\n"); //new line from header to body
      strcat(request_buffer,body); //body
      strcat(request_buffer,"\r\n"); //header
      Serial.println(request_buffer);
      do_http_request("608dev.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT,true);
      tft.fillScreen(TFT_BLACK); //fill background
      tft.setCursor(2,2,1); // set the cursor
      tft.println(response_buffer); //print the result
    }
    }
  }
  old_val1 = val1; //remember for next time!
  old_val2 = val2;
  while (millis()-timer < LOOP_PERIOD);
  timer = millis();
}
