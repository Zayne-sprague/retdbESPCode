//************************************************************
// this is a simple example that uses the painlessMesh library to 
// connect to a node on another network. Please see the WIKI on gitlab
// for more details
// https://gitlab.com/painlessMesh/painlessMesh/wikis/bridge-between-mesh-and-another-network
//************************************************************
#include "painlessMesh.h"
#include "IPAddress.h"


#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

#define   STATION_SSID     "HesTooCoolForHisShoes"
#define   STATION_PASSWORD "1a2b3c4d5e"
#define   STATION_PORT     5000

#define MBEDTLS_ERR_X509_CERT_VERIFY_FAILED               -0x2700  /**< Certificate verification failed, e.g. CRL, CA or signature check failed. */

char HOST_ADDRESS[]="a2p4epskwgtf6v.iot.us-east-1.amazonaws.com";
char CLIENT_ID[]= "3ca7ac1b5d5c41398a682ad0b8b9f28b";
char TOPIC_NAME[]= "myTopic/getPayload";

uint8_t   station_ip[4] =  {192,168,1,8}; // IP of the server


// prototypes
void receivedCallback( uint32_t from, String &msg );

painlessMesh  mesh;

void setup() {
  Serial.begin(115200);
  mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages


  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, STA_AP, WIFI_AUTH_WPA2_PSK, 1);
  mesh.stationManual(STATION_SSID, STATION_PASSWORD, STATION_PORT, station_ip);
 
  mesh.onReceive(&receivedCallback);
  IPAddress myAPIP = IPAddress(mesh.getAPIP().addr);
  Serial.println("My AP IP is " + myAPIP.toString());


}
int tick = 100000;
int tock = 0;
void loop() {
  mesh.update();

  if(tock > tick){
    tock = 0;
    IPAddress myIP = getlocalIP();
    //Serial.println("My IP is " + myIP.toString());
  }
  tock += 1;
}


IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP().addr);
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("\nbridge: Received from %u msg=%s\n", from, msg.c_str());
}
