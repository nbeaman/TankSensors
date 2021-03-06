#ifndef SensorGroup_H
#define SensorGroup_H

#include <Arduino.h>

class SensorGroup{
  
	public:
		SensorGroup();
		~SensorGroup();

		String	sgDBUGtext;
		String 	PHPWebServerIP;
		String	TSsensorMAC;
		String	MyGroupIP[30];
		int		MyGroupIPcurrentIndex=-1;

		bool 	IsInList(String SearchIP);

		void	Remove(String gRemoveMemberIP);
		void	Add(String gNewMemberIP);
		void 	LoadMyGroupIParray();
		void	SendGroupCommand(String gfSubURL);
		
		void MyGroupIP_ADD(String gNewIP);
		void MyGroupIP_REMOVE(String gRemoveIP);

	private:
	

	
	
};
#endif	