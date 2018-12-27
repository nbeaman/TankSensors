#ifndef DoxLED_H
#define DoxLED_H

class DoxLED{

  public:
	DoxLED();
	~DoxLED();

	void LED_Clear();
	void LED_Center_Blue(bool ON);
	void LED_FIND_LIGHT_FOR_DOx();
	void LED_sensor_return_code_Fade(int code);
	void LED_Alert(char TooWhat);
	void LED_smoov(int L1, int L2, int color);
	void LED_Show_Group_Find_Color(int color);
	
	bool LED_GROUPFIND_ON;
};

#endif
