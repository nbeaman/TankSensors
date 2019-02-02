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
	String		DeviceMAC;
	bool		ConnectedTOPHPWebServer;
	bool		SAVELOGSTOWEBFILE;
	
    void 	slog(char type, float val);
	void 	stlog(char type, String Txt);
	void 	TimeZone(int TimeOffset);
	void 	setDBUGText(String T);
	void 	HaveSensorlogLibCheckSendLogMillis();
	void 	sSendAndClearLogs();
	void 	stSendAndClearLogs();
	
	int 		sCurrentIndex;
	int 		stCurrentIndex;
	String		stTextFullLog;
	
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
	

	
	unsigned long 	LastTimeTriedPHPwebserverMillis;
	int 			sMAX_LOG_INDEX;
	bool 			sTIME_TO_SEND_AND_CLEAR=false;	
	String			sTextFullLog;
	
	unsigned long	OnyLogSensorDataEverMillis;
	unsigned long	LastSensorDataLoggedMillis;	

	int 	stMAX_LOG_INDEX;
	bool 	stTIME_TO_SEND_AND_CLEAR=false;
	
	
	SensorDataLog SLOG;
	SensorTextLog TLOG;

	float 	GetVal(int i);
	
	void 	POSTtextFullLog(String sURL, char sensorType);


};

#endif