#include "SensorGroup.h" //include the declaration for this class 

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <Arduino.h>

const int MAXmyGroupEntries = 30;

String 	PHPWebServerIP;
String	TSsensorMAC;
String	MyGroupIP[MAXmyGroupEntries];
int		MyGroupIPcurrentIndex=-1;

//<<constructor>> 
SensorGroup::SensorGroup(){
	PHPWebServerIP="10.0.0.28";
}
 
//<<destructor>>
SensorGroup::~SensorGroup(){/*nothing to destruct*/}

void	SensorGroup::Add(String gNewMemberIP){
	
	int httpResponseCode;
	String	gURL = "http://" + PHPWebServerIP + "/SensorGroup.php?action=add?gIP=" + gNewMemberIP;
	HTTPClient http;
	http.begin(gURL);
	http.addHeader("Content-Type", "text/plain");             //Specify content-type header			
	httpResponseCode = http.POST(gURL);
	http.end();  //Close connection	

	MyGroupIP_ADD(gNewMemberIP);
}

void	SensorGroup::Remove(String gRemoveMemberIP){
	
	int httpResponseCode;
	String	gURL = "http://" + PHPWebServerIP + "/SensorGroup.php?action=remove?gIP=" + gRemoveMemberIP;
	HTTPClient http;
	http.begin(gURL);
	http.addHeader("Content-Type", "text/plain");             //Specify content-type header			
	httpResponseCode = http.POST(gURL);
	http.end();  //Close connection	

	MyGroupIP_REMOVE(gRemoveMemberIP);
}

bool SensorGroup::IsInList(String SearchIP){
	bool vReturn=false;
	for(int i=0; i<=MyGroupIPcurrentIndex ;i++){
		if(MyGroupIP[i]==SearchIP){
			vReturn=true;
			i=MyGroupIPcurrentIndex+1;
		}
	}
	return vReturn;
}

void SensorGroup::MyGroupIP_ADD(String gNewMemberIP){
	if(!IsInList(gNewMemberIP)){
		if(MyGroupIPcurrentIndex < (MAXmyGroupEntries - 1)) MyGroupIPcurrentIndex++;	// If the array is full, just replace the last entry
		MyGroupIP[MyGroupIPcurrentIndex] = gNewMemberIP;
	}	
}

void SensorGroup::MyGroupIP_REMOVE(String gRemoveIP){
	bool MoveDown=false;
	int		mi;
	
	if(MyGroupIPcurrentIndex !=-1){											// There is somthing in MyGroupIP array
		for(int i=0; i <= MyGroupIPcurrentIndex; i++){						// Run through MyGroupIP to find the IP to remove
			if(MyGroupIP[i]==gRemoveIP && MoveDown==false) MoveDown=true;	// If we find the IP and not already in shift IP's down mode (MoveDown: true), do the following
																			// if NOT, just keep searching (do nothing and go on to inc i
			if(MoveDown){													// Yes - in shift IP array: move the next IP in the list to the current i position
				mi = i + 1;													// mi set to next item after i
				if(mi > MyGroupIPcurrentIndex){								// is there a next IP entry after i in the list?
					MyGroupIP[i]="";										// if NO, blank the last entry and decreese MyGroupIPcurrentIndex
					MyGroupIPcurrentIndex--;
				}else{														// if YES, move the preceeding IP entry to the current i spot/index
					MyGroupIP[i]=MyGroupIP[mi];
				}
			}
		}		
	}
}


















