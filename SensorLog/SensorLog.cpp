#include "SensorLog.h" //include the declaration for this class 

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <Arduino.h>

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

String		DBUGtext;
String		LogWebServerIP;
String		DeviceName = "NotAssigned";

int 		sCurrentIndex=0;
int 		sMAX_LOG_INDEX=3;
bool 		sTIME_TO_SEND_AND_CLEAR=false;
String		sTextFullLog;				

int 		stCurrentIndex=0;
int 		stMAX_LOG_INDEX=450;
bool 		stTIME_TO_SEND_AND_CLEAR=false;	
String		stTextFullLog;

struct SensorDataLog{
	char		 	when[450][9];
	char 			type[450];
	float 			val[450];		
};

struct SensorTextLog{
	char		 	when[450][9];
	char 			type[450];
	char 			Txt[450][15];		
};

SensorDataLog SLOG;
SensorTextLog TLOG;


//<<constructor>> setup the LED, make pin 13 an OUTPUT
SensorLog::SensorLog(){
	sCurrentIndex=-1;				// -1 b/c we want to start at index 0 for the slog function
	sMAX_LOG_INDEX=450;
	stCurrentIndex=-1;				// -1 b/c we want to start at index 0 for the stlog function
	stMAX_LOG_INDEX=450;


}
 
//<<destructor>>
SensorLog::~SensorLog(){/*nothing to destruct*/}

int SensorLog::TimeZone(int TimeOffset){
	timeClient.begin();
	timeClient.setTimeOffset(TimeOffset);       //US Eastern time is 3600 * (-5)
	timeClient.update();	
}

float SensorLog::GetVal(int i){
	return	SLOG.val[i];
}

void SensorLog::POSTtextFullLog(String IP, char sORst){
	int httpResponseCode;
	
	HTTPClient http;
    http.begin(IP);
    http.addHeader("Content-Type", "text/plain");             //Specify content-type header
	if (sORst=='s') httpResponseCode = http.POST(sTextFullLog);
	else httpResponseCode = http.POST(sTextFullLog);
	DBUGtext = String(httpResponseCode);
	http.end();  //Close connection
}

void SensorLog::sSendAndClearLogs(){
	int i,si;

	for(i=0; i <=sCurrentIndex-1; i++){
		sTextFullLog="";

		sTextFullLog= sTextFullLog + String(SLOG.when[i]) + ",";
		sTextFullLog= sTextFullLog + String(SLOG.type[i]) + ",";
		sTextFullLog= sTextFullLog + String(SLOG.val[i]) + ",";
	}
	POSTtextFullLog("http://" + LogWebServerIP + "/TextFullLog.php?devicename=" + DeviceName + "&logdata=" + sTextFullLog,'s');
	sCurrentIndex=0;
	
}

void SensorLog::slog(char type, float val){
	  String T;
	  int i;
      sCurrentIndex++;
	  if( sCurrentIndex >= 3 ) sSendAndClearLogs();
	  T = timeClient.getFormattedTime();
	  for(i=0; i<9; i++){
		  SLOG.when[sCurrentIndex][i]=T[i];
	  }
	  SLOG.when[8][i]='\0';
      SLOG.type[sCurrentIndex] 	= type;
      SLOG.val[sCurrentIndex] 	= val;
}

void SensorLog::stlog(char type, String Txt){
	  String T;
	  int i;
      stCurrentIndex++;
	  if( stCurrentIndex > stMAX_LOG_INDEX ) stTIME_TO_SEND_AND_CLEAR=true;
	  T = timeClient.getFormattedTime();
	  for(i=0; i<9; i++){
		  SLOG.when[sCurrentIndex][i]=T[i];
	  }
	  SLOG.when[8][i]='\0';
      TLOG.type[stCurrentIndex] 	= type;
	  for(i=0; i<14; i++){
		TLOG.Txt[stCurrentIndex][i]		= Txt[i];  
	  }
	  TLOG.Txt[stCurrentIndex][14]		= '\0';
	  						// make sure the last character in the char array is a NULL. This makes the text only 14 chars long.
}

void SensorLog::sClear(){
	sCurrentIndex=0;
}

void SensorLog::stClear(){
	stCurrentIndex=0;
}