#include "SensorLog.h" //include the declaration for this class 

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <Arduino.h>

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

String		DBUGtext;
String		LogWebServerIP;
String		DeviceName = "NotAssigned";
bool		ConnectedTOPHPWebServer = true;
bool		SAVELOGSTOWEBFILE = false;

int 		sCurrentIndex;
int 		sMAX_LOG_INDEX;
bool 		sTIME_TO_SEND_AND_CLEAR=false;
String		sTextFullLog;				

int 		stCurrentIndex;
int 		stMAX_LOG_INDEX;
bool 		stTIME_TO_SEND_AND_CLEAR=false;	
String		stTextFullLog;

unsigned long sLastTimeLogSent;
unsigned long stLastTimeLogSent;

unsigned long LastTimeTriedPHPwebserverMillis;

struct SensorDataLog{
	char		 	when[450][9];
	char 			type[450];
	float 			val[450];		
};

struct SensorTextLog{
	char		 	when[450][9];
	char 			type[450];
	String 			Txt[450];		
};

SensorDataLog SLOG;
SensorTextLog TLOG;


//<<constructor>> 
SensorLog::SensorLog(){
	stCurrentIndex	=	-1;
	sCurrentIndex	=	-1;
	sLastTimeLogSent=millis();
	stLastTimeLogSent=millis();
	LastTimeTriedPHPwebserverMillis=millis();
	ConnectedTOPHPWebServer	=	true;
}
 
//<<destructor>>
SensorLog::~SensorLog(){/*nothing to destruct*/}

void SensorLog::TimeZone(int TimeOffset){
	timeClient.begin();
	timeClient.setTimeOffset(TimeOffset);       //US Eastern time is 3600 * (-5)
	timeClient.update();	
}

void SensorLog::HaveSensorlogLibCheckSendLogMillis(){
	if((millis() - sLastTimeLogSent) > 300000) sSendAndClearLogs();
	if((millis() - stLastTimeLogSent) > 300000) stSendAndClearLogs();
}

void SensorLog::POSTtextFullLog(String sURL){
	
	if(SAVELOGSTOWEBFILE){									// code using library will set this to true or false depending on if they have a php web server to save logs.
		
		if(ConnectedTOPHPWebServer || ((millis() - LastTimeTriedPHPwebserverMillis) > 300000)){		// try every 15 minutes to connect to php web server if we were unsuccessfull before.
			LastTimeTriedPHPwebserverMillis=millis();
			int httpResponseCode;
			HTTPClient http;
			http.begin(sURL);
			http.addHeader("Content-Type", "text/plain");             //Specify content-type header
			httpResponseCode = http.POST(sTextFullLog);
			setDBUGText(String(httpResponseCode));
			if(httpResponseCode ==-1) ConnectedTOPHPWebServer=false;
			else ConnectedTOPHPWebServer=true;
			http.end();  //Close connection		
				
		}	
	}	
}

void SensorLog::setDBUGText(String T){
	DBUGtext = T;
}

void SensorLog::sSendAndClearLogs(){
	int i,si;
	sTextFullLog="";
	
	for(i=0; i <=sCurrentIndex; i++){

		sTextFullLog= sTextFullLog + String(SLOG.when[i]) + ",";
		sTextFullLog= sTextFullLog + String(SLOG.type[i]) + ",";
		sTextFullLog= sTextFullLog + String(SLOG.val[i]) + ",";
	}
	POSTtextFullLog("http://" + LogWebServerIP + "/TextFullLog.php?devicename=" + DeviceName + "&logdata=" + sTextFullLog);
	
	sLastTimeLogSent=millis();
	
	sCurrentIndex=-1;
	
}

void SensorLog::stSendAndClearLogs(){
	int i,si;
	char temp[15];
	stTextFullLog="";
	
	for(i=0; i <=stCurrentIndex; i++){
		
		stTextFullLog= stTextFullLog + String(TLOG.when[i]) + ",";
		stTextFullLog= stTextFullLog + String(TLOG.type[i]) + ",";
		for(si=0; si<14; si++){
			temp[si]=TLOG.Txt[i][si];
		}
		temp[14]='\0';
		stTextFullLog= stTextFullLog + String(temp) + ",";
	}
	POSTtextFullLog("http://" + LogWebServerIP + "/TextFullLog.php?devicename=" + DeviceName + "-TEXT&logdata=" + stTextFullLog);
	
	stLastTimeLogSent=millis();
	
	stCurrentIndex=-1;
	
}

void SensorLog::slog(char type, float val){
	  String T;
	  int i;
      
	  if( (sCurrentIndex >= 3) || ((millis() - sLastTimeLogSent) >= 300000) ){ 
		  if(sCurrentIndex !=- 1) sSendAndClearLogs();  // send log every 10 minutes or when index reaches max.
	  }

	  sCurrentIndex =  sCurrentIndex + 1;
	  
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
      
	  if( (stCurrentIndex >= 3) || ((millis() - stLastTimeLogSent) >= 300000) ){
		  if(stCurrentIndex !=- 1) stSendAndClearLogs();  // send log every 10 minutes or when index reaches max.
	  }

	  stCurrentIndex = stCurrentIndex + 1;

	  T = timeClient.getFormattedTime();
	  for(i=0; i<9; i++){
		  TLOG.when[stCurrentIndex][i]	=	T[i];
	  }
	  TLOG.when[8][i]='\0';
      TLOG.type[stCurrentIndex] 		= type;
	  TLOG.Txt[stCurrentIndex] 			= Txt;
	  // make sure the last character in the char array is a NULL. This makes the text only 14 chars long.
}

void SensorLog::sClear(){
	sCurrentIndex=0;
}

void SensorLog::stClear(){
	stCurrentIndex=0;
}