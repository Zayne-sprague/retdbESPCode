//Set up MESH Connection and WIFI connection
#include "painlessMesh.h"
#include "IPAddress.h"

//SPECIAL VARS
#define _DEBUG            true
String last_message = "";


#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

#define   STATION_SSID     "HesTooCoolForHisShoes"
#define   STATION_PASSWORD "1a2b3c4d5e"
#define   STATION_PORT     5000

//INPUT OUTPUTS
#define TRIGGER_PIN 4
#define EMPTY_PIN 2 
#define OCCUPIED_PIN 19

#define TRANSMITTER 5

//IP ADDRESS FOR HUB
uint8_t   station_ip[4] =  {192,168,1,8}; // IP of the server
uint32_t   bridge_id = -1;

bool is_bridge = false;
bool start_bridge_check = false;

int cycles_looking_for_bridge = 0;

uint8_t previous_val = 0;
uint8_t val = 0;

// prototypes
void receivedCallback( uint32_t from, String &msg );
void sendMessage();
void ask_for_bridge();

//Initial Mesh
painlessMesh  mesh;
Scheduler userScheduler;
Scheduler checkForBridge;

//Task for every sensor
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
Task checkForBridgeTask( TASK_SECOND * 5, TASK_FOREVER, &ask_for_bridge);


void setup() {
  Serial.begin(115200);
  mesh.setDebugMsgTypes( ERROR );  // We only want error logs for now
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on

  //SET UP TRIGGER FOR SENSOR
  pinMode(TRIGGER_PIN, INPUT);
  pinMode(OCCUPIED_PIN, OUTPUT);
  pinMode(EMPTY_PIN, OUTPUT);
  pinMode(TRANSMITTER, OUTPUT);

  digitalWrite(OCCUPIED_PIN, 1);
  digitalWrite(EMPTY_PIN, 1);
  digitalWrite(TRANSMITTER, 1);

  delay(100);
  
  digitalWrite(OCCUPIED_PIN, 0);
  digitalWrite(TRANSMITTER, 0);

  //DEFAULT INITIALIZATION

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);

  userScheduler.addTask( taskSendMessage );
  checkForBridge.addTask( checkForBridgeTask );

  taskSendMessage.enable();
  checkForBridgeTask.enable();

  pt("Setup");

}

void loop() {
  // put your main code here, to run repeatedly:
  userScheduler.execute(); // it will run mesh scheduler as well
  checkForBridge.execute(); 
  
  if(start_bridge_check && !is_bridge){
    pt("Checking for Bridge");
    cycles_looking_for_bridge += 1;
  }

  if(cycles_looking_for_bridge > 1000000 && !is_bridge){
    pt("No bridge detected... switching to bridge");
    switch_to_bridge();
  }

  
  mesh.update();
}


void ask_for_bridge(){

  if (!is_bridge){
    pt("Asking for bridge");
    
    start_bridge_check = true;
    
    String msg = "WHOSE BRIDGE";
    mesh.sendBroadcast( msg );  
    pt("Wheres the bridge?");
  }
}
  


void switch_to_bridge(){
    pt("Switching to bridge");
    
    is_bridge = true;
    cycles_looking_for_bridge = 0;
    start_bridge_check = false;
    
    mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, STA_AP, WIFI_AUTH_WPA2_PSK, 1);
    mesh.stationManual(STATION_SSID, STATION_PASSWORD, STATION_PORT, station_ip);
    mesh.onReceive(&receivedCallback);
    IPAddress myAPIP = IPAddress(mesh.getAPIP().addr);
    Serial.println("My AP IP is " + myAPIP.toString());
    String msg = "BRIDGE DETECTED";
    mesh.sendBroadcast( msg );
    pt("Switched to a bridge");
}
  
void sendMessage() {
  /*String msg = "Hello from node ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));*/

  pt("Checking inputs");
  
  if(is_bridge){
    String msg = "STILL HERE";
    mesh.sendBroadcast(msg);
    pt("I'm Still Here yo!");
  }

  digitalWrite(TRANSMITTER, 1);
  val = digitalRead(TRIGGER_PIN);

  if (val != previous_val){
    previous_val = val;
    String msg = "";
  
    if (val > 0.8){
      
      msg = "{\"Temperature\": 81,\"Humidity\": 50 }";

        digitalWrite(OCCUPIED_PIN, 1);
        digitalWrite(EMPTY_PIN, 0);

    }else{
      msg = "empty";

      digitalWrite(OCCUPIED_PIN, 0);
      digitalWrite(EMPTY_PIN, 1);

    }
    //msg += mesh.getNodeId();
    if( bridge_id != -1 && !is_bridge ){
        mesh.sendSingle( bridge_id, msg );
        pt("Node sent message");
        pt(msg);
    }else if (is_bridge){
      mesh.sendBroadcast( msg );
      pt("Bridge sent message");
      pt(msg);
    }else{
    
      pt("No bridge");
    }

  }
  
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
  digitalWrite(TRANSMITTER, 0);
}

void receivedCallback( uint32_t from, String &msg ) {
  if(is_bridge){
    Serial.printf("\nbridge: Received from %u msg=%s\n", from, msg.c_str());
  }else{
     Serial.printf("\Node: Received from %u msg=%s\n", from, msg.c_str());
  }
  String message;
  if(msg == "STILL HERE" && from != bridge_id){
    cycles_looking_for_bridge = 0;
    start_bridge_check = false;
    bridge_id = from;

    pt("Got message from bridge");
  }else if(msg == "BRIDGE DETECTED"){
    cycles_looking_for_bridge = 0;
    start_bridge_check = false;
    bridge_id = from;

    pt("New bridge found");
  }else if(msg ==  "WHOSE BRIDGE"){
    if (is_bridge){
      message = "STILL HERE";
      mesh.sendBroadcast( message );

      pt("I'm the bridge yo");
    }
  }else if (is_bridge){
          mesh.sendBroadcast( msg );
  }
}

void pt(String msg){
  if (_DEBUG && msg != last_message){
    last_message = msg;
    Serial.print("DEBUG: ");
    Serial.println( msg );
  }
}


