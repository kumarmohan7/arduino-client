#include <Ethernet.h>
#include <EthernetClient.h>

#include <PubSubClient.h>
#include <ATT_IOT.h>                            //AllThingsTalk IoT library
#include <SPI.h>                                //required to have support for signed/unsigned long type.

/*
  SmartLiving Makers Arduino Demo Sketch
  version 2.0 dd 25/02/2015
  
  This file is an example sketch to deploy an Analog Actuator in the SmartLiving.io IoT platform.
  
  ### Instructions

  1. Setup the Arduino hardware
    - USB2Serial
    - Grove kit shield
    - connect an Analog Actuator to Pin A2 of the Arduino shield 

  2. Add 'iot_att' library to your Arduino Environment. [Try this guide](http://arduino.cc/en/Guide/Libraries)
  3. fill in the missing strings (deviceId, clientId, clientKey, mac) and optionally change/add the sensor & actuator names, ids, descriptions, types
     For extra actuators, make certain to extend the callback code at the end of the sketch.
  4. Upload the sketch

  ### Troubleshooting


*/

// Enter below your client credentials. 
//These credentials can be found in the configuration pane under your device in the smartliving.io website 

char deviceId[] = ""; // Your device id comes here
char clientId[] = ""; // Your client id comes here;
char clientKey[] = ""; // Your client key comes here;


ATTDevice Device(deviceId, clientId, clientKey);            //create the object that provides the connection to the cloud to manager the device.
char httpServer[] = "api.smartliving.io";                   // HTTP API Server host                  
char mqttServer[] = "broker.smartliving.io";		    // MQTT Server Address


// Define PIN numbers for assets

int AnalogActuator = 2;                                        // Analog Actuator is connected to pin A02 on grove shield 

//required for the device
void callback(char* topic, byte* payload, unsigned int length);
EthernetClient ethClient;
PubSubClient pubSub(mqttServer, 1883, callback, ethClient);

void setup()
{
         
  pinMode(AnalogActuator, OUTPUT);					            // initialize the digital pin as an output.         
  Serial.begin(9600);							            // init serial link for debugging
  
  byte mac[] = {0x90, 0xA2, 0xDA, 0x0D, 0xE1, 0x3E};        // Adapt to your Arduino MAC Address  
  if (Ethernet.begin(mac) == 0)                             // Initialize the Ethernet connection:
  { 
    Serial.println(F("DHCP failed,end"));
    while(true);                                    		//we failed to connect, halt execution here. 
  }
  delay(1000);                                          	//give the Ethernet shield a second to initialize:
  
  if(Device.Connect(&ethClient, httpServer))					            // connect the device with the IOT platform.
  {
    Device.AddAsset(AnalogActuator, "YourAnalogActuatorname", "Analog Actuator Description", true, "integer");   // Create the Digital Actuator asset for your device
    Device.Subscribe(pubSub);						            // make certain that we can receive message from the iot platform (activate mqtt)
  }
  else 
    while(true);                                                                
}
						       
void loop()
{
  Device.Process(); 
}

// Callback function: handles messages that were sent from the iot platform to this device.
void callback(char* topic, byte* payload, unsigned int length) 
{ 
  String msgString; 
  {                                                           //put this in a sub block, so any unused memory can be freed as soon as possible, required to save mem while sending data
	char message_buff[length + 1];	                      //need to copy over the payload so that we can add a /0 terminator, this can then be wrapped inside a string for easy manipulation
	strncpy(message_buff, (char*)payload, length);        //copy over the data
	message_buff[length] = '\0';		              //make certain that it ends with a null			
		  
	msgString = String(message_buff);
	msgString.toLowerCase();			      //to make certain that our comparison later on works ok (it could be that a 'True' or 'False' was sent)
  }
  int* idOut = NULL;
  {	                                                      //put this in a sub block, so any unused memory can be freed as soon as possible, required to save mem while sending data
	int pinNr = Device.GetPinNr(topic, strlen(topic));
	
	Serial.print("Payload: ");                            //show some debugging
	Serial.println(msgString);
	Serial.print("topic: ");
	Serial.println(topic);

        if (pinNr == AnalogActuator)  
	{
	  if (msgString == "false") {
            digitalWrite(AnalogActuator, LOW);		     //change the actuator status to false
            idOut = &AnalogActuator;		                        
	  }
	  else if (msgString == "true") {
	    digitalWrite(AnalogActuator, HIGH);              //change the actuator status to true
            idOut = &AnalogActuator;
	  }
	}
  }
  if(idOut != NULL)                                           //Let the iot platform know that the operation was succesful
    Device.Send(msgString, *idOut);    
}

