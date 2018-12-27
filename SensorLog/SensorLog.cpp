#include "SensorLog.h" //include the declaration for this class 



int 			CurrentIndex;
int 			MAX_LOG_INDEX;

unsigned long 	_slwhen[450];
char 			_sltype[450];
float 			_slvalue[450];
 
//<<constructor>> setup the LED, make pin 13 an OUTPUT
SensorLog::SensorLog(){
	CurrentIndex=0;
	MAX_LOG_INDEX=450;
	
}
 
//<<destructor>>
SensorLog::~SensorLog(){/*nothing to destruct*/}

void SensorLog::slog(unsigned int lwhen, char ltype, float lval){
      CurrentIndex++;
	  if( CurrentIndex > MAX_LOG_INDEX ) CurrentIndex=0;
      _slwhen[CurrentIndex] 	= lwhen;
      _sltype[CurrentIndex] 	= ltype;
      _slvalue[CurrentIndex] 	= lval;
}