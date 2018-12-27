#ifndef SensorLog_H
#define SensorLog_H
  
class SensorLog{
  
  public:
	SensorLog();
	~SensorLog(); 
	
	int CurrentIndex;
	int MAX_LOG_INDEX;

    void slog(unsigned int lwhen, char ltype, float lval);
	
};

#endif

