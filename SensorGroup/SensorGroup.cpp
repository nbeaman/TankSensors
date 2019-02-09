#include "SensorGroup.h" //include the declaration for this class 

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <Arduino.h>

const int MAXmyGroupEntries = 30;

String	sgDBUGtext="";
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

void SensorGroup::LoadMyGroupIParray(){
	int 	httpResponseCode;
	String 	payload;
	int		commaPosition;
	
	String	gURL = "http://" + PHPWebServerIP + "/SensorGroup.php?Action=load&FromMAC=" + TSsensorMAC;
	
	HTTPClient http;
	http.begin(gURL);
	http.addHeader("Content-Type", "text/plain");             //Specify content-type header			
	httpResponseCode = http.GET();
	if (httpResponseCode > 0) {
		payload = http.getString();
	}
	
	http.end();  //Close connection	
	
	payload = payload.substring(payload.indexOf("<body>") + 6, payload.indexOf("</body>"));
	payload.replace("\n","");
	payload.replace("\r","");
	//sgDBUGtext = payload;	

	for(int p=0; p < 29; p++){
		commaPosition = payload.indexOf(",");
		sgDBUGtext = sgDBUGtext + String(payload) + ",";
		if(commaPosition < 3) p=30;
		else{
			MyGroupIP[p]=payload.substring(0, commaPosition);
			payload.replace(MyGroupIP[p] + ",", "");
			MyGroupIPcurrentIndex=p;
		}
	}	
}


void	SensorGroup::Add(String gNewMemberIP){
	
	int httpResponseCode;
	String	gURL = "http://" + PHPWebServerIP + "/SensorGroup.php?Action=add&FromMAC=" + TSsensorMAC + "&SensorIP=" + gNewMemberIP;
	
	HTTPClient http;
	http.begin(gURL);
	http.addHeader("Content-Type", "text/plain");             //Specify content-type header			
	httpResponseCode = http.POST(gURL);
	http.end();  //Close connection	

	MyGroupIP_ADD(gNewMemberIP);
}

void	SensorGroup::Remove(String gRemoveMemberIP){
	
	int httpResponseCode;
	String	gURL = "http://" + PHPWebServerIP + "/SensorGroup.php?Action=remove&FromMAC=" + TSsensorMAC + "&SensorIP=" + gRemoveMemberIP;
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


















