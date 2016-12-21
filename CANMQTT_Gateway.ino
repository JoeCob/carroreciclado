//  CAN to Ethernet
//  Jan 28th, 2014
//  Written by: Cory J. Fowler

#include <mcp_can.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Change these for your network!
byte mac[] = {0x00, 0x55, 0x66, 0xEE, 0xFF, 0xFF};
IPAddress      ip(10, 100, 50, 233);
IPAddress    server(10, 100, 50, 210);

unsigned long rxId;
byte len = 0;
byte rxBuf[8];
char buffer[50];

MCP_CAN CAN0(9);                                   // Set CS to pin 9

EthernetClient ethClient;
PubSubClient client(ethClient);

long lastReconnectAttempt = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

boolean reconnect() {
  if (client.connect("arduinoClient")) {
    // Once connected, publish an announcement...
    client.publish("CONTROL","CANBRIDGE_CONNECT");
    // ... and resubscribe
    //client.subscribe("inTopic");
  }
  return client.connected();
}



void setup()
{
  Serial.begin(115200);
//  CAN0.begin(CAN_250KBPS);                         // init CAN Bus with 250kb/s baudrate
  CAN0.begin(MCP_ANY, CAN_250KBPS, MCP_16MHZ);     // init CAN Bus with 250kb/s baudrate at 16MHz with Mask & Filters Disabled
  CAN0.setMode(MCP_NORMAL);                        // Set operation mode to normal so the MCP2515 sends acks to received data.
  pinMode(2, INPUT);                               // Setting pin 2, MCP2515 /INT, to input mode


  client.setServer(server, 1883);
  client.setCallback(callback);
  Ethernet.begin(mac,ip);                          // Initialize Ethernet
  
  Serial.println("CAN to Ethernet...");
}

void loop()
{
    if(!digitalRead(2))                            // If pin 2 is low, read receive buffer
    {
      CAN0.readMsgBuf(&rxId, &len, rxBuf);         // Read Data: rxID = Message ID, len = Data Length, buf = Data Byte(s)

      // Use an array to buffer in case ethernet is failing? 
      // Any way of using call backs for CANBUS? 

      
      sprintf(buffer, "%.8lX:%.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n\r",
              rxId, rxBuf[0], rxBuf[1], rxBuf[2], rxBuf[3], rxBuf[4], rxBuf[5], rxBuf[6], rxBuf[7]);

     // UDP.beginPacket(dest, remPort);
     // UDP.write(buffer);
     // UDP.endPacket();
     if (!client.connected()) {
        long now = millis();
        if (now - lastReconnectAttempt > 5000) {
          lastReconnectAttempt = now;
          // Attempt to reconnect
          if (reconnect()) {
             lastReconnectAttempt = 0;
          }
        }
      } else {
    // Client connected
      client.publish ( "CANBUS", buffer );
      client.loop();
      } 
    }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
