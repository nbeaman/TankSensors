#include "SensorLog.h" //include the declaration for this class 

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <string.h>

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

int 		sCurrentIndex=0;
int 		sMAX_LOG_INDEX=450;
bool 		sTIME_TO_SEND_AND_CLEAR=false;

int 		stCurrentIndex=0;
int 		stMAX_LOG_INDEX=450;
bool 		stTIME_TO_SEND_AND_CLEAR=false;	

struct SensorDataLog{
	unsigned long 	when[450];
	char 			type[450];
	float 			val[450];	
}; 

struct SensorTextLog{
	unsigned long 	when[450];
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

	timeClient.begin();
	timeClient.setTimeOffset(-18000);       //US Eastern time is 3600 * (-5)	
}
 
//<<destructor>>
SensorLog::~SensorLog(){/*nothing to destruct*/}

float SensorLog::GetVal(int i){
	return	SLOG.val[i];
}

void SensorLog::slog(unsigned int when, char type, float val){
      sCurrentIndex++;
	  if( sCurrentIndex > sMAX_LOG_INDEX ) sTIME_TO_SEND_AND_CLEAR=true;
      SLOG.when[sCurrentIndex] 	= when;
      SLOG.type[sCurrentIndex] 	= type;
      SLOG.val[sCurrentIndex] 	= val;
}

void SensorLog::stlog(unsigned int when, char type, char Txt[15]){
      stCurrentIndex++;
	  if( stCurrentIndex > stMAX_LOG_INDEX ) stTIME_TO_SEND_AND_CLEAR=true;
      TLOG.when[stCurrentIndex] 	= when;
      TLOG.type[stCurrentIndex] 	= type;
      strncpy(TLOG.Txt[stCurrentIndex],Txt,15);
	  TLOG.Txt[stCurrentIndex][14]='\0';						// make sure the last character in the char array is a NULL. This makes the text only 14 chars long.
}

void SensorLog::sClear(){
	sCurrentIndex=0;
}

void SensorLog::stClear(){
	stCurrentIndex=0;
}