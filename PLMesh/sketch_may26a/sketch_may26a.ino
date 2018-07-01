//************************************************************
// this is a simple example that uses the painlessMesh library
//
// 1. sends a silly message to every node on the mesh at a random time between 1 and 5 seconds
// 2. prints anything it receives to Serial.print
//
//
//************************************************************
#include "painlessMesh.h"
#include "IPAddress.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

#define   STATION_SSID     "HesTooCoolForHisShoes"
#define   STATION_PASSWORD "1a2b3c4d5e"
#define   STATION_PORT     5000

#define TRIGGER_PIN 4
#define EMPTY_PIN 2
#define OCCUPIED_PIN 19
#define TRANSMITTER 5

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );


// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
    Serial.printf("Changed connections %s\n",mesh.subConnectionJson().c_str());
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);

  //SET UP TRIGGER FOR SENSOR
  pinMode(TRIGGER_PIN, INPUT);
  pinMode(OCCUPIED_PIN, OUTPUT);
  pinMode(EMPTY_PIN, OUTPUT);

  digitalWrite(OCCUPIED_PIN, 1);
  digitalWrite(EMPTY_PIN, 1);
  digitalWrite(TRANSMITTER, 1);

  delay(100);
  
  digitalWrite(OCCUPIED_PIN, 0);
  digitalWrite(TRANSMITTER, 0);

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
}


void loop() {
  //Serial.print("HELLO!");
  userScheduler.execute(); // it will run mesh scheduler as well


  
  mesh.update();
}



uint8_t previous_val = 0;
uint8_t val = 0;


void sendMessage() {
  /*String msg = "Hello from node ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));*/

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
    mesh.sendBroadcast( msg );
    Serial.print("Sent message:");
    Serial.println(msg);
  }
  
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));
  digitalWrite(TRANSMITTER, 0);
}

