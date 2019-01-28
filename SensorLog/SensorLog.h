#ifndef SensorLog_H
#define SensorLog_H

#include <Arduino.h>

class SensorLog{
  
  public:
	SensorLog();
	~SensorLog(); 

	String		DBUGtext;
	String		LogWebServerIP;	
	String		DeviceName;
	bool		ConnectedTOPHPWebServer;
	bool		SAVELOGSTOWEBFILE;
	int 		sCurrentIndex;
	int 		stCurrentIndex;
	
    void 	slog(char type, float val);
	void 	stlog(char type, String Txt);
	void 	TimeZone(int TimeOffset);
	void 	setDBUGText(String T);
	void 	HaveSensorlogLibCheckSendLogMillis();
	
  private:
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
	

	int 	sMAX_LOG_INDEX;
	bool 	sTIME_TO_SEND_AND_CLEAR=false;	
	String	sTextFullLog;
	

	int 	stMAX_LOG_INDEX;
	bool 	stTIME_TO_SEND_AND_CLEAR=false;
	String	stTextFullLog;
	
	SensorDataLog SLOG;
	SensorTextLog TLOG;

	float 	GetVal(int i);
	
	void 	POSTtextFullLog(String IP);
	void 	sSendAndClearLogs();
	void 	stSendAndClearLogs();
	void 	sClear();
	void 	stClear();

};

#endif