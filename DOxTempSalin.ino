#include <WiFi.h>                 //WiFi
#include <AsyncTCP.h>             //WebServer
#include <ESPAsyncWebServer.h>    //WebServer
#include <Wire.h>                 //enable I2C
#include <LiquidCrystal_I2C.h>    //LCD
#include <Button.h>               //Button
#include <HTTPClient.h>

//============[ CODE IS FOR WHICH SENSOR ]==========
#define CODE_FOR_DOx_DEVICE           true
#define CODE_FOR_TEMPSALINITY_DEVICE  false
// REMEMBER: Change LED_NUMBER_OF_LEDS in DoxLED.cpp  // 7 for NeoPixel Jewel, 12 for NeoPixel Ring.
//==================================================
//-------------
#if (CODE_FOR_DOx_DEVICE)
#include <DoxConfig.h>
#else
#include <TempSalinConfig.h>
#endif
//==================================================

#include <DoxLED.h>               // My custom library that uses FastLED.h
DoxLED DoxLED;

#include <SensorLog.h>
SensorLog SENSORLOG;


//-----------[ DBUG ]-------------------------------
const int DBUG = 0;               // Set this to 0 for no serial output for debugging, 1 for moderate debugging, 2 for FULL debugging to see serail output in the Arduino GUI.
//--------------------------------------------------

//-----------[ SENSOR LIMITS ]----------------
float   GV_DOx_TOOHIGHVALUE         = 19.00;
float   GV_DOx_TOOLOWVALUE          = 1.00;
float   GV_TEMP_TOOHIGHVALUE        = 81.00;
float   GV_TEMP_TOOLOWVALUE         = 40.00;
float   GV_SALIN_TOOHIGHVALUE       = 999.00;
float   GV_SALIN_TOOLOWVALUE        = -1.00;
String  GV_NOTIFY_ON                = "Y";
//--------------------------------------------

//-----------[ LOOP SWITCH VARS ]-------------
bool  GV_WEB_REQUEST_IN_PROGRESS            = false;
bool  GV_READ_REQUEST_IN_PROGRESS           = false;
bool  GV_THIS_IS_A_SERIAL_COMMAND           = false;
bool  GV_QUERY_SENSOR_NAME_ON_NEXT_COMMAND  = true;   // Loop tests to see if we need to query the DOx for it's name.  Set to true to do this first thing
int   GV_FIND                               = 0;
bool  GV_GROUPFIND                          = false;
int   GV_GROUPCOLOR;
bool  GV_BOOTING_UP                         = true;
bool  GV_TEMPSALIN_CALTEMP_REMOTE_DOx       = false;
bool  GV_TEMPSALIN_CALSALIN_REMOTE_DOx      = false;
char  GV_TEMPSALIN_ALTERNATE                = 'T';    // used for Temp & Salinity sensor.  Alters between 'T' and 'S'.  Device was running slow when both were read at the same time.
//---------------------------------------------

//BUTTON
Button button1(2);                                    // Connect button between pin 2 and GND
int     GV_LCD_MAIN_TEXT_INDEX  = 0;                  // Index of array of text to show in top left of LCD
String  GV_LCD_MAIN_TEXT[4];                          // Sensor name, Alert HIGH/LOW values, IP Address, MAC Address.  Depending on the button.
//-----
 
// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);                   // set the LCD address to 0x27 for a 16 chars and 2 line
const bool ClearLCD = true, NoClearLCD = false, PrintSerial = true, NoSerial = false;  // Used for LCD_DISPLAY

//I2C
String GV_SENSOR_DATA           = "";                 // Holds latest reading from D.O. Circuit.
float  GV_DOX,GV_TEMP,GV_SALIN;
String GV_SENSOR_RESPONSE       = "NONE";             // Holds latest response from D.O. Circuit.
String GV_WEB_RESPONSE_TEXT     = "";

//Timing for LOOP switches
unsigned long SensorAutoReadingMillis     = millis();   // Stores milliseconds since last D.O. reading.
unsigned long HeartBeatMillis             = millis();
unsigned long WebServerRestartMillis      = millis();
unsigned long LastNotificationSentMillis  = millis(); 
char          HeartBeat                   = ' ';
//----------------------------------------

//---------------[ WEB SERVER SETUP]------
AsyncWebServer server(80);                // Setup Web Server Port.
const char* PARAM_MESSAGE = "command";    // HTTP_GET parameter to look for.
String MACaddress;
String TEMPSALIN_REMOTEDOX_IP;
//----------------------------------------

//================================================================================================
//===========================================[ SETUP ]============================================
//================================================================================================
void setup() {
  Serial.begin(115200);
  
  //I2C
  Wire.begin();                //enable I2C port for the button(s)
  //BUTTON
  button1.begin();
  // LCD
  lcd.begin();                  //initialize the lcd

  //-----------------------------[ WEB SERVER ]-----------------------------------------------

  WiFi.begin(config_WIFI_SSID, config_WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    LCD_DISPLAY("Wifi..", 0, 0, ClearLCD, PrintSerial);
    LCD_DISPLAY(config_WIFI_SSID, 0, 1, NoClearLCD, PrintSerial);
  }
  lcd.clear();
  
  IPAddress IP=WiFi.localIP();
  GV_LCD_MAIN_TEXT[3]=String(IP[0]) + '.' + String(IP[1]) + '.' + String(IP[2]) + '.' + String(IP[3]);
  Serial.println(GV_LCD_MAIN_TEXT[3]);
  
  byte mac[6];
  WiFi.macAddress(mac);
  MACaddress   = String(mac[5],HEX) + String(mac[4],HEX) + String(mac[3],HEX) + String(mac[2],HEX) + String(mac[1],HEX) + String(mac[0],HEX);
  GV_LCD_MAIN_TEXT[0] = MACaddress;
  Serial.println( GV_LCD_MAIN_TEXT[0] );

  GV_LCD_MAIN_TEXT[2]= "H" + String(GV_DOx_TOOHIGHVALUE) + " L" + String(GV_DOx_TOOLOWVALUE);

  SENSORLOG.LogWebServerIP = config_SensorLogPHPwebserver;
  SENSORLOG.SAVELOGSTOWEBFILE = true;
  
  //---------------------------------[ Sensor Web Page ]-----------------------------------------
  //-----------------[ / ]----------------------
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "DoxMAX: " + String(GV_DOx_TOOHIGHVALUE) + "<BR>DoxMIN: " + String(GV_DOx_TOOLOWVALUE) + "<br>" + MACaddress);
    });
      
  //-----------------[ /read ]------------------
  server.on("/read", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", GV_WEB_RESPONSE_TEXT);
    });
    
  //-----------------[ /set  ]------------------
  server.on("/send", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String message;
    int w=0;
    GV_WEB_REQUEST_IN_PROGRESS = true;               // set this to tell the auto DOx read loop we want to talk to the DOx sensor from the micro PC's web page.  I assume this is an Interupt.
    if (request->hasParam(PARAM_MESSAGE)) {
      message = request->getParam(PARAM_MESSAGE)->value();
    } else {
      message = "No message sent";
    }
    while (GV_READ_REQUEST_IN_PROGRESS){             // wait until the auto DOx read ends so we can have a turn sending this request sent from the micro PC's web page to the DOx sensor
      w++;
      if (w < 1000) break;
      }
      
    SendCommandToSensorAndSetReturnGVVariables(message);

    request->send(200, "text/plain", GV_WEB_RESPONSE_TEXT);

    GV_WEB_REQUEST_IN_PROGRESS = false;
    });
    
  //-----------------[  /findon  ]---------------
  server.on("/findon", HTTP_GET, [](AsyncWebServerRequest * request) {
    GV_FIND=1;
    request->send(200, "text/plain", "Find On");
    });
 
  //-----------------[  /findoff  ]--------------
  server.on("/findoff", HTTP_GET, [](AsyncWebServerRequest * request) {
    GV_FIND=0;
    request->send(200, "text/plain", "Find Off");
    }); 

  //-----------------[ /set  ]-------------------
  server.on("/set", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String message;
    if (request->hasParam(PARAM_MESSAGE)) {
      message = request->getParam(PARAM_MESSAGE)->value();
    } else {
      message = "No message sent";
    }
    request->send(200, "text/plain", "Veriable Set");

    SetVariableFromWebRequest(message);
    });

 //-----------------[ /groupfind ]----------------
  server.on("/groupfind", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String message;
    if (request->hasParam(PARAM_MESSAGE)) {
      message = request->getParam(PARAM_MESSAGE)->value();
    } else {
      message = "No message sent";
    }
    request->send(200, "text/plain", "Group Find On");

    GV_GROUPCOLOR=message.toInt(); 
    GV_GROUPFIND=true;
    DoxLED.LED_GROUPFIND_ON  = true; 
    });    

  //---------------[ /groupfindoff ]---------------
  server.on("/groupfindoff", HTTP_GET, [](AsyncWebServerRequest * request) {
    GV_GROUPFIND=false;
    DoxLED.LED_GROUPFIND_ON  = false;
    request->send(200, "text/plain", "Group Find Off");
    });
    
  server.begin();

  SENSORLOG.TimeZone(config_TimeZone);
  //---------------------------------------------------------------------------------------------
}
//================================================================================================
//=========================================[ SETUP: END ]=========================================
//================================================================================================


//Variables used in LOOP
char computerdata[20];           //we make a 20 byte character array to hold incoming data from a pc/mac/other.
byte received_from_computer = 0; //we need to know how many characters have been received.
char incoming_data[20];                //we make a 20 byte character array to hold incoming data from the D.O. circuit.
byte in_char = 0;                //used as a 1 byte buffer to store inbound bytes from the D.O. Circuit.
int time_ = 600;                 //used to change the delay needed depending on the command sent to the EZO Class D.O. Circuit.
String DO;                        //char pointer used in string parsing.
int i = 0;
char R = 'r';
char *ReadDOx = &R;

//================================================================================================
//==========================================[ LOOP ]==============================================
//================================================================================================
void loop() {
  
  if( CODE_FOR_TEMPSALINITY_DEVICE && GV_TEMPSALIN_CALTEMP_REMOTE_DOx ){
        TEMPSALIN_CalTemp_RemoteDOx( TEMPSALIN_REMOTEDOX_IP );
        GV_TEMPSALIN_CALTEMP_REMOTE_DOx = false;
  }
      
  if( CODE_FOR_TEMPSALINITY_DEVICE && GV_TEMPSALIN_CALTEMP_REMOTE_DOx ){ 
        TEMPSALIN_CalSalin_RemoteDOx( TEMPSALIN_REMOTEDOX_IP );
        GV_TEMPSALIN_CALSALIN_REMOTE_DOx = false;   
  }

  if (GV_FIND){                                                         // GV_FIND is set to true if web page ipaddress/findon is called.  Nothing is supposed to function while this is happening.
    while( GV_FIND == 1 ){                                              // GV_FIND is set to false if web page ipaddress/findoff is called.
      if (CODE_FOR_DOx_DEVICE){
        DoxLED.LED_FIND_LIGHT_FOR_DOx();
      }
    }
    DoxLED.LED_Clear();
  }

  if (GV_GROUPFIND){
    DoxLED.LED_Clear();   
    while(DoxLED.LED_GROUPFIND_ON){
      DoxLED.LED_Show_Group_Find_Color(GV_GROUPCOLOR);
    }
    DoxLED.LED_Clear();
  }

  if (GV_QUERY_SENSOR_NAME_ON_NEXT_COMMAND){              // query DOx name if this is set to true, then display it on the LCD screen.  This is set to run on boot-up and when DOx name was changed. 
    if (CODE_FOR_DOx_DEVICE){
      SendCommandToSensorAndSetReturnGVVariables("name,?\0");
    }
    if (CODE_FOR_TEMPSALINITY_DEVICE){
      SendCommandToSensorAndSetReturnGVVariables(String(config_SalinitySensorAddress) + ":name,?\0");
    }    
    
    //Serial.println(GV_SENSOR_DATA);
    GV_SENSOR_DATA.remove(0,6);
    GV_LCD_MAIN_TEXT[1]=GV_SENSOR_DATA;
    GV_LCD_MAIN_TEXT_INDEX=1;
    LCD_DISPLAY(GV_LCD_MAIN_TEXT[GV_LCD_MAIN_TEXT_INDEX],0,0,ClearLCD,PrintSerial);
    // SensorLog
    SENSORLOG.DeviceName =  GV_LCD_MAIN_TEXT[1];
    
    GV_QUERY_SENSOR_NAME_ON_NEXT_COMMAND = false;
  }

  if (Serial.available() > 0) {                                           //if data is holding in the serial buffer
    received_from_computer = Serial.readBytesUntil(13, computerdata, 20); //we read the data sent from the serial monitor(pc/mac/other) until we see a <CR>. We also count how many characters have been received.
    computerdata[received_from_computer] = 0;                             //stop the buffer from transmitting leftovers or garbage.
    computerdata[0] = tolower(computerdata[0]);                           //we make sure the first char in the string is lower case.
    if (computerdata[0] == 'c' || computerdata[0] == 'r')time_ = 600;     //if a command has been sent to calibrate or take a reading we wait 600ms so that the circuit has time to take the reading.
    else time_ = 300;                                                     //if not 300ms will do

    GV_THIS_IS_A_SERIAL_COMMAND=true;                                     //set gloabal indicator that the next command to the DO cercuit is from the Serial Monitor.
    String str_serial_command=computerdata;                               //convert char array to String type for the function that follows
    SendCommandToSensorAndSetReturnGVVariables(str_serial_command);                    //send the command received by the serial monitoring device.
  }
  
  if (((millis() - SensorAutoReadingMillis) > 2000) && !GV_WEB_REQUEST_IN_PROGRESS){
  
    GV_READ_REQUEST_IN_PROGRESS=true;
    if (CODE_FOR_DOx_DEVICE){
      SendCommandToSensorAndSetReturnGVVariables(ReadDOx);
      LCD_DISPLAY(GV_SENSOR_DATA, 0, 1, NoClearLCD, PrintSerial);
      LCD_DISPLAY("  ", GV_SENSOR_DATA.length(), 1, NoClearLCD, NoSerial);
      SENSORLOG.slog('D', GV_SENSOR_DATA.toFloat()); Serial.print("SENSORLOG.sCurrentIndex"); Serial.println(String(SENSORLOG.sCurrentIndex));
    }
    if (CODE_FOR_TEMPSALINITY_DEVICE){
      if (GV_TEMPSALIN_ALTERNATE == 'T') {                                             // b/c device was running slow due to two long reads at the same time.  Alertnate between reads.
        SendCommandToSensorAndSetReturnGVVariables(String(config_TemperatureSensorAddress) + ":r");
        LCD_DISPLAY(GV_SENSOR_DATA + "   ", 0, 1, NoClearLCD, PrintSerial);
        GV_TEMPSALIN_ALTERNATE = 'S';                                                   // next reading will be Salinity.
      } else {
        SendCommandToSensorAndSetReturnGVVariables(String(config_SalinitySensorAddress) + ":r");
        LCD_DISPLAY(GV_SENSOR_DATA + " ", 13-GV_SENSOR_DATA.length(), 1, NoClearLCD, PrintSerial);
        GV_TEMPSALIN_ALTERNATE = 'T';                                                  // next reading will be Temp.
      }
    }
    SensorAutoReadingMillis = millis();
    GV_READ_REQUEST_IN_PROGRESS=false;
  }
  
  if ((millis() - WebServerRestartMillis) > 500000) {
    if(DBUG) Serial.println("WebServer re-begin");
    WebServerRestartMillis = millis();
    server.begin();
  }
  
  SensorHeartBeat();

  BUTTON_WasItPressed_ChangeLCD();

  if(GV_BOOTING_UP) { 
    SENSORLOG.stlog('S',"BOOTUP1" + String(SENSORLOG.stCurrentIndex));Serial.print("SENSORLOG.stCurrentIndex"); Serial.println(String(SENSORLOG.stCurrentIndex)); 
    SENSORLOG.stlog('S',"BOOTUP2" + String(SENSORLOG.stCurrentIndex));Serial.print("SENSORLOG.stCurrentIndex"); Serial.println(String(SENSORLOG.stCurrentIndex));
    SENSORLOG.stlog('S',"BOOTUP3" + String(SENSORLOG.stCurrentIndex));Serial.print("SENSORLOG.stCurrentIndex"); Serial.println(String(SENSORLOG.stCurrentIndex));
    SENSORLOG.stlog('S',"BOOTUP4" + String(SENSORLOG.stCurrentIndex));Serial.print("SENSORLOG.stCurrentIndex"); Serial.println(String(SENSORLOG.stCurrentIndex)); 
  }

  SENSORLOG.HaveSensorlogLibCheckSendLogMillis();   // if logs have not reached their MAX (making Sensorlog.h) you still

  GV_BOOTING_UP=false;
  
}
//================================================================================================
//=========================================[ LOOP: END ]==========================================
//================================================================================================

//Functions:
bool TEMPSALIN_CalTemp_RemoteDOx( String SensorIPToCalibrate ){
  HTTPClient http;
  float Celsius = ( GV_TEMP -32 ) * (0.555555);
  Serial.print("Celsius: ");Serial.println(Celsius);
  String URLtext = "http://" + SensorIPToCalibrate + "/send?command=T," + String(Celsius);
  http.begin(URLtext);
  Serial.println(URLtext);
  DoxLED.LED_SendCalToDOx();
  int httpResponseCode = http.GET();
  Serial.println(httpResponseCode);
  delay(2000);
  return true;
}

bool TEMPSALIN_CalSalin_RemoteDOx( String SensorIPToCalibrate ){
  HTTPClient http;  
  String URLtext = "http://" + SensorIPToCalibrate + "/send?command=S," + String(GV_SALIN);
  http.begin(URLtext);
  Serial.println(URLtext);
  DoxLED.LED_SendCalToDOx();  
  int httpResponseCode = http.GET();
  Serial.println(httpResponseCode);
  delay(2000);
  return true;  
}

void SendAlert_IFTTT(String eventName, String val1, String val2){
  // A "bad" reading can be a sensor reading out-of-range or just blank or bad data.  We want a second "bad" reading to occure within 15 minutes of the first before sending an alert
  // to the user(s).  On first "bad" reading, check to see if/when the last alert was sent.  If it's over 30 minutes, set LastNotificationSentMillis to "now". 

  if((millis() - LastNotificationSentMillis) > 1800000){          // greater than 30 minutes means the sensor has just started its alert mode (sensor reading out-of-rang).  We want to
                                                                  // wait 15 minutes before sending the FIRST alert, so set LastNotificationSentMillis to "now".  So if it finds another
                                                                  // out of rang reading within 15 minutes, this function will continue on to its "else" code below.
    LastNotificationSentMillis=millis();
  } else {
    if((millis() - LastNotificationSentMillis) > 900000){         // 15 minutes
      HTTPClient http;
      Serial.println("Notification Sent");
      http.begin("https://maker.ifttt.com/trigger/" + eventName + "/with/key/dOO4GGcvxO_pBa0QPwHN19");
      //http.addHeader("Content-Type", "text/plain");             //Specify content-type header
      http.addHeader("Content-Type", "application/json");
  
      String JSONtext = "{ \"value1\" : \"" + val1 + "\", \"value2\" : \"" + val2 + "\" }";
      Serial.println(JSONtext);
  
      int httpResponseCode = http.POST(JSONtext);   //Send the actual POST request
      // * could check for response - on my todo list * // 
  
      LastNotificationSentMillis = millis();
    }    
  }
}

String StringTo14chars(String S){
  int l=S.length();
  String TempStr;
  if(l > 14) S.substring(0,14);
  if(l < 14){
    for (int si=l; si<=14; si++){
      TempStr.concat(" ");
    }
    S.concat(TempStr);
  }
  return S;
}

void SetVariableFromWebRequest(String SetVarCommand){
  
  SetVarCommand.toLowerCase();
  Serial.println(SetVarCommand);
  int colonCharIndex = SetVarCommand.indexOf(':');
  String tempStr = SetVarCommand.substring(0,colonCharIndex);
  tempStr.toLowerCase();
  if(tempStr == "doxmax"){
    String tempVal = SetVarCommand.substring(colonCharIndex+1,SetVarCommand.length()); 
    GV_DOx_TOOHIGHVALUE = tempVal.toFloat();
    GV_LCD_MAIN_TEXT[2]= "H" + String(GV_DOx_TOOHIGHVALUE) + " L" + String(GV_DOx_TOOLOWVALUE);
    if(GV_LCD_MAIN_TEXT_INDEX == 2) LCD_DISPLAY(StringTo14chars(GV_LCD_MAIN_TEXT[GV_LCD_MAIN_TEXT_INDEX]),0,0,NoClearLCD,PrintSerial);
  }   
  if(tempStr == "doxmin"){
    String tempVal = SetVarCommand.substring(colonCharIndex+1,SetVarCommand.length()); 
    GV_DOx_TOOLOWVALUE = tempVal.toFloat();
    GV_LCD_MAIN_TEXT[2]= "H" + String(GV_DOx_TOOHIGHVALUE) + " L" + String(GV_DOx_TOOLOWVALUE);
    if(GV_LCD_MAIN_TEXT_INDEX == 2) LCD_DISPLAY(StringTo14chars(GV_LCD_MAIN_TEXT[GV_LCD_MAIN_TEXT_INDEX]),0,0,NoClearLCD,PrintSerial);
  }
  if(tempStr == "tempmax"){
    String tempVal = SetVarCommand.substring(colonCharIndex+1,SetVarCommand.length()); 
    GV_TEMP_TOOHIGHVALUE = tempVal.toFloat();
  }   
  if(tempStr == "tempmin"){
    String tempVal = SetVarCommand.substring(colonCharIndex+1,SetVarCommand.length()); 
    GV_TEMP_TOOLOWVALUE = tempVal.toFloat();
  } 
     if(tempStr == "salinmax"){
    String tempVal = SetVarCommand.substring(colonCharIndex+1,SetVarCommand.length()); 
    GV_SALIN_TOOHIGHVALUE = tempVal.toFloat();
  }   
  if(tempStr == "salinmin"){
    String tempVal = SetVarCommand.substring(colonCharIndex+1,SetVarCommand.length()); 
    GV_SALIN_TOOLOWVALUE = tempVal.toFloat();
  }
  if(tempStr == "notify"){
    String tempVal = SetVarCommand.substring(colonCharIndex+1,SetVarCommand.length());
    if(tempVal == "y") GV_NOTIFY_ON="Y";
    else GV_NOTIFY_ON="N";
  } 
  if(tempStr == "doxcaltemp"){
    Serial.println("DOxCal");
    String tempVal = SetVarCommand.substring(colonCharIndex+1,SetVarCommand.length());
    TEMPSALIN_REMOTEDOX_IP = tempVal; 
    GV_TEMPSALIN_CALTEMP_REMOTE_DOx = true;
  }
  if(tempStr == "doxcalsalin"){
    Serial.println("DOxCal");
    String tempVal = SetVarCommand.substring(colonCharIndex+1,SetVarCommand.length()); 
    TEMPSALIN_REMOTEDOX_IP = tempVal;
    GV_TEMPSALIN_CALSALIN_REMOTE_DOx = true;
  }  
}

String AddCarrageReturnIfNeeded(String str){   
    long int len = str.length();
    int i=0;
    bool HasCR = false;
    
    for (i=0; i < len; i++) if (str[i]=='\r') HasCR=true;   
    if (!HasCR) { str[i++]='\r'; str[i++]='\0'; }   
    if(DBUG==2){
        for (i=0; i < (str[i] != '\0'); i++){
            Serial.print(str[i]); Serial.print(":"); Serial.print(i); Serial.print(":"); Serial.print(int(str[i]));
        }
    }
    return str;    
}

void LCD_DISPLAY(String Text, int row, int col, bool xClearLCD, bool xprintSerial) { // 12 Millis
  if (xClearLCD) lcd.clear();
  lcd.setCursor(row, col); // set the cursor to column 15, line 1
  lcd.print(Text);
  if (xprintSerial) if(DBUG) Serial.println("LCD:" + Text);
}

void BUTTON_WasItPressed_ChangeLCD(){
  String LCDTEXT;
  if (button1.pressed() && !GV_BOOTING_UP){
    if(DBUG) Serial.println("Button 1 pressed");
    switch (GV_LCD_MAIN_TEXT_INDEX){
      case 0: GV_LCD_MAIN_TEXT_INDEX=1;
              LCDTEXT=GV_LCD_MAIN_TEXT[GV_LCD_MAIN_TEXT_INDEX];
              break;
      case 1: GV_LCD_MAIN_TEXT_INDEX=2;
              LCDTEXT=GV_LCD_MAIN_TEXT[GV_LCD_MAIN_TEXT_INDEX];
              break;
      case 2: GV_LCD_MAIN_TEXT_INDEX=3;
              LCDTEXT=GV_LCD_MAIN_TEXT[GV_LCD_MAIN_TEXT_INDEX];
              break;
      case 3: GV_LCD_MAIN_TEXT_INDEX=0;
              LCDTEXT=GV_LCD_MAIN_TEXT[GV_LCD_MAIN_TEXT_INDEX];
              break;
    }
    if(DBUG) Serial.println(LCDTEXT);
    LCD_DISPLAY(StringTo14chars(LCDTEXT), 0, 0, NoClearLCD, PrintSerial);                            
   }  
}

void SensorHeartBeat() {

  if ((millis() - HeartBeatMillis) > 1000) {
    Serial.println(SENSORLOG.SAVELOGSTOWEBFILE);
    
    if(SENSORLOG.SAVELOGSTOWEBFILE) LCD_DISPLAY("L", 13, 1, NoClearLCD, NoSerial);
    else LCD_DISPLAY(" ", 13, 1, NoClearLCD, NoSerial);
    Serial.print("DBUGText: ");Serial.println(SENSORLOG.DBUGtext);
    
    //GV_DOX,GV_TEMP,GV_SALIN
    if (CODE_FOR_DOx_DEVICE){
       if ( (GV_DOX >= GV_DOx_TOOHIGHVALUE) || (GV_DOX <= GV_DOx_TOOLOWVALUE) ) { DoxLED.LED_Alert('H'); SendAlert_IFTTT("Sensor_Value_Alert", GV_LCD_MAIN_TEXT[1], String(GV_DOX)); }
    }
    if (CODE_FOR_TEMPSALINITY_DEVICE){
       if ( (GV_TEMP >= GV_TEMP_TOOHIGHVALUE) || (GV_TEMP <= GV_TEMP_TOOLOWVALUE) ) { DoxLED.LED_Alert('H'); SendAlert_IFTTT("Sensor_Value_Alert", GV_LCD_MAIN_TEXT[1], String(GV_TEMP)); }
       if ( (GV_SALIN >= GV_SALIN_TOOHIGHVALUE) || (GV_SALIN <= GV_SALIN_TOOLOWVALUE) ) { DoxLED.LED_Alert('H'); SendAlert_IFTTT("Sensor_Value_Alert", GV_LCD_MAIN_TEXT[1], String(GV_SALIN)); }
    }
    if (HeartBeat != ' ') {
      HeartBeat = ' ';
      DoxLED.LED_Center_Blue(false);
    }
    else {
      if (CODE_FOR_DOx_DEVICE)          HeartBeat = 'D';
      if (CODE_FOR_TEMPSALINITY_DEVICE) HeartBeat = 'M';
      DoxLED.LED_Center_Blue(true);
    }
    if(DBUG==2) Serial.println(HeartBeat);
    LCD_DISPLAY(&HeartBeat, 15, 0, NoClearLCD, NoSerial);

    if(GV_NOTIFY_ON == "Y") LCD_DISPLAY("Y",15,1,NoClearLCD,NoSerial);
    else LCD_DISPLAY(" ",15,1,NoClearLCD,NoSerial);
    
    HeartBeatMillis = millis();
  }
}

void SendCommandToSensorAndSetReturnGVVariables(String command) {

  
  char    Ccommand[20];
  byte    code = 0;                   //used to hold the I2C response code.
  String  SensorCommand;
  int     channel;
  
  if (CODE_FOR_DOx_DEVICE){
    SensorCommand = command;
    channel = config_DOxSensorAddress;
  }
  if (CODE_FOR_TEMPSALINITY_DEVICE){
      int colonCharIndex = command.indexOf(':');
      String tempStr = command.substring(0,colonCharIndex);
      channel = tempStr.toInt();
      SensorCommand = command.substring(colonCharIndex+1,command.length());
      //Serial.println(channel);
     // Serial.println(SensorCommand);
  }

  SensorCommand[0] = tolower(SensorCommand[0]);
  command=AddCarrageReturnIfNeeded(SensorCommand);
  SensorCommand.toCharArray(Ccommand,20);
  Wire.beginTransmission(channel);                                 //call the circuit by its ID number.
  Wire.write(Ccommand);                                            //transmit the command that was sent through the serial port.
  Wire.endTransmission();                                          //end the I2C data transmission.
 
  if (DBUG){ Serial.print("DOc command:("); Serial.print(Ccommand); Serial.print(")"); }

  if (CODE_FOR_DOx_DEVICE){
    if (Ccommand[0] == 'c' || Ccommand[0] == 'r' || Ccommand[0] == 'n')time_ = 600;     //if a command has been sent to calibrate or take a reading we wait 600ms so that the circuit has time to take the reading.
    else time_ = 300;                                                                   //if not 300ms will do
  }
   if (CODE_FOR_TEMPSALINITY_DEVICE){
      time_ = 600;
  }                                            
    
  //if (strcmp(computerdata, "sleep") != 0) {  //if the command that has been sent is NOT the sleep command, wait the correct amount of time and request data.
  //if it is the sleep command, we do nothing. Issuing a sleep command and then requesting data will wake the D.O. circuit.

  delay(time_);                     //wait the correct amount of time for the circuit to complete its instruction.

  Wire.requestFrom(channel, 20, 1); //call the circuit and request 20 bytes (this may be more than we need)
  code = Wire.read();               //the first byte is the response code, we read this separately.

  switch (code) {                   //switch case based on what the response code is.
    case 1:                         //decimal 1.
      if(DBUG==2) Serial.println("Success");    //means the command was successful.
      GV_SENSOR_RESPONSE="Success";
      break;                        //exits the switch case.
    case 2:                         //decimal 2.
      if(DBUG==2) Serial.println("Failed");     //means the command has failed.
      GV_SENSOR_RESPONSE="Failed \0";
      GV_SENSOR_DATA="";
      break;                        //exits the switch case.
    case 254:                      //decimal 254.
      if(DBUG==2) Serial.println("Pending");   //means the command has not yet been finished calculating.
      GV_SENSOR_RESPONSE="Pending\0";
      GV_SENSOR_DATA="";
      break;                       //exits the switch case.
    case 255:                      //decimal 255.
      if(DBUG==2) Serial.println("No Data");   //means there is no further data to send.
      GV_SENSOR_RESPONSE="No Data\0";
      GV_SENSOR_DATA="";
      break;                       //exits the switch case.
  }
   
  while (Wire.available()) {       //are there bytes to receive.
    in_char = Wire.read();         //receive a byte.
    incoming_data[i] = in_char;          //load this byte into our array.
    if(DBUG==2) Serial.println(incoming_data[i]);
    i += 1;                        //incur the counter for the array element.
    if (in_char == 0) {            //if we see that we have been sent a null command.
      i = 0;                       //reset the counter i to 0.
      Wire.endTransmission();      //end the I2C data transmission.
      break;                       //exit the while loop.
    }
  }
  if (Ccommand[0] != 'r') DoxLED.LED_sensor_return_code_Fade(code);
  
  GV_SENSOR_DATA = incoming_data;
  
  // set GV_DOX,GV_TEMP,GV_SALIN
  if(CODE_FOR_DOx_DEVICE) GV_DOX = GV_SENSOR_DATA.toFloat();
  if(CODE_FOR_TEMPSALINITY_DEVICE && channel==config_SalinitySensorAddress) GV_SALIN  = GV_SENSOR_DATA.toFloat();
  if(CODE_FOR_TEMPSALINITY_DEVICE && channel==config_TemperatureSensorAddress) GV_TEMP = GV_SENSOR_DATA.toFloat();

  GV_WEB_RESPONSE_TEXT=GV_SENSOR_DATA + "," + GV_SENSOR_RESPONSE;

  if (Ccommand[0] == 'n' && Ccommand[5] != '?'){                        // someone sent the name,TheName to the DO cercuit (not just querying the name,?).
    GV_QUERY_SENSOR_NAME_ON_NEXT_COMMAND = true;    
  }
  
  if ( DBUG==2 || (DBUG==1 && GV_THIS_IS_A_SERIAL_COMMAND) ){
      Serial.println("GV_SENSOR_DATA:" + GV_SENSOR_DATA);
      Serial.println("GV_SENSOR_RESPONSE:" + GV_SENSOR_RESPONSE);
      Serial.println("GV_WEB_RESPONSE_TEXT" + GV_WEB_RESPONSE_TEXT);
  }
  if (GV_THIS_IS_A_SERIAL_COMMAND) GV_THIS_IS_A_SERIAL_COMMAND=false;   //set gloabal indicator that command from the Serial Monitor is done.

}
