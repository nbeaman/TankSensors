#include "SensorLog.h" //include the declaration for this class 

#include <string.h>


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
	char * 			Txt[450][15];	
};

SensorDataLog SLOG;
SensorTextLog TLOG;


//<<constructor>> setup the LED, make pin 13 an OUTPUT
SensorLog::SensorLog(){
	sCurrentIndex=0;
	sMAX_LOG_INDEX=450;
	stCurrentIndex=0;
	stMAX_LOG_INDEX=450;	
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

void SensorLog::stlog(unsigned int when, char type, char * Txt[15]){
      stCurrentIndex++;
	  if( stCurrentIndex > stMAX_LOG_INDEX ) stTIME_TO_SEND_AND_CLEAR=true;
      TLOG.when[stCurrentIndex] 	= when;
      TLOG.type[stCurrentIndex] 	= type;
      strncpy(*TLOG.Txt[stCurrentIndex],*Txt,15);
}

void SensorLog::sClear(){
	sCurrentIndex=0;
}

void SensorLog::stClear(){
	stCurrentIndex=0;
}