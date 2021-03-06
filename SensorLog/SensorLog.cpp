#include "SensorLog.h" //include the declaration for this class 

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <Arduino.h>

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// PUBLIC variables - accessable by app using this library
String		CodeIsFor;						// this will be set to "DOx" or "TempSalin" by the sketch using this library
bool		GotLogFor_T = false;
bool		GotLogFor_S = false;
String		DBUGtext;						// to pass dbug info for testing to app using this library
String		LogWebServerIP;					// IP address of PHP Webserver with php code to save logs
String		DeviceName = "NotAssigned";		// Sensor Device name
String		DeviceMAC;
bool		ConnectedTOPHPWebServer = true; // trie of PHP Webserver is connectable/available
bool		SAVELOGSTOWEBFILE = false;		// true if you want to save logs

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

unsigned long 	LastTimeTriedPHPwebserverMillis;
unsigned long	OnyLogSensorDataEverMillis;
unsigned long	LastSensorDataLoggedMillis;

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
	OnyLogSensorDataEverMillis = 30000;
	LastSensorDataLoggedMillis=millis();
	sLastTimeLogSent=millis();
	stLastTimeLogSent=millis();
	LastTimeTriedPHPwebserverMillis=millis();
	ConnectedTOPHPWebServer	=	true;
}
 
//<<destructor>>
SensorLog::~SensorLog(){/*nothing to destruct*/}

void SensorLog::TimeZone(int TimeOffset){
	timeClient.begin();
	timeClient.setTimeOffset(TimeOffset);       //US Eastern time is 3600 * (-5) = -18000
	timeClient.update();	
}

void SensorLog::HaveSensorlogLibCheckSendLogMillis(){
	if((millis() - sLastTimeLogSent) > 300000) if(sCurrentIndex != -1) sSendAndClearLogs();
	if((millis() - stLastTimeLogSent) > 300000) if(stCurrentIndex != -1) stSendAndClearLogs();
}

void SensorLog::POSTtextFullLog(String SensorURL, char logType){
	
	if(SAVELOGSTOWEBFILE){									// code using library will set this to true or false depending on if they have a php web server to save logs.
		
		if(ConnectedTOPHPWebServer || ((millis() - LastTimeTriedPHPwebserverMillis) > 300000)){		// try every 15 minutes to connect to php web server if we were unsuccessfull before.
			LastTimeTriedPHPwebserverMillis=millis();
			int httpResponseCode;
			HTTPClient http;
			http.begin(SensorURL);
			http.addHeader("Content-Type", "text/plain");             //Specify content-type header
			
			if(logType == 's') httpResponseCode = http.POST(SensorURL);
			else {
				httpResponseCode = http.POST(SensorURL);
				setDBUGText(SensorURL);
			}

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
		sTextFullLog= sTextFullLog + String(SLOG.val[i]) + "%0A";
	}
	POSTtextFullLog("http://" + LogWebServerIP + "/TextFullLog.php?devicename=" + DeviceName + "&MAC=" + DeviceMAC + "-DATA&logdata=" + URLcharacters(sTextFullLog), 's');
	
	sLastTimeLogSent=millis();
	
	sCurrentIndex=-1;
	
}

void SensorLog::slog(char type, float val){
	String T;
	int i;
	bool LogIt  = false;
      
	if( (sCurrentIndex >= 3) || ((millis() - sLastTimeLogSent) >= 300000) ){ 
		 if(sCurrentIndex != -1) sSendAndClearLogs();  // send log every 10 minutes or when index reaches max.
	}

	if((millis() - LastSensorDataLoggedMillis) > OnyLogSensorDataEverMillis){
		
		if(CodeIsFor == "DOx"){
			LogIt = true;
		}else{
			if(type == 'T') { GotLogFor_T=true; LogIt = true; }
			if(GotLogFor_T && type == 'S') { GotLogFor_S=true; LogIt = true; }	
		}
			
		if(LogIt){
			sCurrentIndex =  sCurrentIndex + 1;
			  
			T = timeClient.getFormattedTime();
			for(i=0; i<9; i++){
				SLOG.when[sCurrentIndex][i]=T[i];
			}
			SLOG.when[8][i]='\0';
			SLOG.type[sCurrentIndex] 	= type;
			SLOG.val[sCurrentIndex] 	= val;			
		}


		if(CodeIsFor == "DOx"){
			LastSensorDataLoggedMillis=millis();		// set timer for next log entry for DOx
		} else {

			if(GotLogFor_T && GotLogFor_S) {			// For TempSalinity Sensor do not reset log timer until both Temp and Salinity are logged (in that order).
				LastSensorDataLoggedMillis=millis();	
				GotLogFor_T = false;
				GotLogFor_S = false;
			}
		}

	}

}

String SensorLog::URLcharacters(String STR){
	STR.replace(" ","%20");
	return STR;
}

void SensorLog::stSendAndClearLogs(){
	int sti,si;
	char temp[15];
	stTextFullLog="";
	
	for(sti=0; sti <= stCurrentIndex; sti++){
		
		stTextFullLog= stTextFullLog + String(TLOG.when[sti]) + ",";
		stTextFullLog= stTextFullLog + String(TLOG.type[sti]) + ",";
		for(si=0; si<14; si++){
			temp[si]=TLOG.Txt[sti][si];
		}
		temp[14]='\0';
		stTextFullLog= stTextFullLog + String(temp) + "%0A";
	}
	POSTtextFullLog("http://" + LogWebServerIP + "/TextFullLog.php?devicename=" + DeviceName + "&MAC=" + DeviceMAC + "-INFO&logdata=" + URLcharacters(stTextFullLog), 't');
	
	stLastTimeLogSent=millis();
	
	stCurrentIndex=-1;
	
}

void SensorLog::stlog(char type, String Txt){
	  String T;
	  int i;
      
	  if( (stCurrentIndex == 3) || ((millis() - stLastTimeLogSent) >= 300000) ){  // send log every 5 minutes or when index reaches max.
		  if(stCurrentIndex != -1) stSendAndClearLogs();  
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
