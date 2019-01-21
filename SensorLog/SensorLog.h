#ifndef SensorLog_H
#define SensorLog_H

#include <Arduino.h>

class SensorLog{
  
  public:
	SensorLog();
	~SensorLog(); 

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
	String		DBUGtext;
	String		LogWebServerIP;	
	String		DeviceName;
	
	int 	sCurrentIndex;
	int 	sMAX_LOG_INDEX;
	bool 	sTIME_TO_SEND_AND_CLEAR=false;	
	String	sTextFullLog;
	
	int 	stCurrentIndex;
	int 	stMAX_LOG_INDEX;
	bool 	stTIME_TO_SEND_AND_CLEAR=false;
	String	stTextFullLog;
	
	SensorDataLog SLOG;
	SensorTextLog TLOG;

    void 	slog(char type, float val);
	void 	stlog(char type, String Txt);
	float 	GetVal(int i);
	void 	POSTtextFullLog(String IP, char sORst);
	int 	TimeZone(int TimeOffset);
	void 	sSendAndClearLogs();	
	void 	sClear();
	void 	stClear();

};

#endif