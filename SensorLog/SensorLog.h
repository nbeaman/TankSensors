#ifndef SensorLog_H
#define SensorLog_H
  
class SensorLog{
  
  public:
	SensorLog();
	~SensorLog(); 

struct SensorDataLog{
	unsigned long 	when[450];
	char 			type[450];
	float 			val[450];	
};

struct SensorTextLog{
	unsigned long 	when[450];
	char 			type[450];
	char  			Txt[450][15];	
};

	int 	sCurrentIndex;
	int 	sMAX_LOG_INDEX;
	bool 	sTIME_TO_SEND_AND_CLEAR=false;	

	int 	stCurrentIndex;
	int 	stMAX_LOG_INDEX;
	bool 	stTIME_TO_SEND_AND_CLEAR=false;

	SensorDataLog SLOG;
	SensorTextLog TLOG;

    void 	slog(unsigned int lwhen, char ltype, float lval);
	void 	stlog(unsigned int lwhen, char ltype, char Txt[15]);
	float 	GetVal(int i);
	void 	sClear();
	void 	stClear();

};

#endif