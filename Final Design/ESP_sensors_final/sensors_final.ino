#include <WiFi.h>
#include <DHT.h>
#include "DFRobot_OxygenSensor.h"
WiFiServer server(80);                // Creates a server that listens for incoming connections on the specified port

//Server connect to WiFi Network
const char* ssid = "MaryBrownsUltraFastWiFi";
const char* password = "chickengogogo";
int count=0;
#define PIR_SENSOR_PIN_1 26
#define PIR_SENSOR_PIN_2 27
#define HUMI_TEMP_SENSOR_PIN 33
#define HALL_SENSOR_PIN 32
#define DHTPIN HUMI_TEMP_SENSOR_PIN
#define DHTTYPE DHT11
#define Oxygen_IICAddress ADDRESS_3
#define COLLECT_NUMBER  10
DHT dht(DHTPIN, DHTTYPE);
DFRobot_OxygenSensor oxygen;

int detectionState = 0;
unsigned long firstDetectionTime = 0;
unsigned long secondDetectionTime = 0;
int peopleCount = 0;
int HallState = 0;
int a = 0;
//=====================================================================================================================================

void setup() 
{
  Serial.begin(115200);                 // Opens serial port, then sets data rate (bps)
  
  WiFi.mode(WIFI_STA);                  
  WiFi.begin(ssid, password);           // Connect to WiFi
 
  // Wait for connection  
  Serial.println("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED) // While not connected, print ... to serial monitor
  {   
    delay(500);
    Serial.print(".....");
    delay(500);
  }

  // Once connected, print to serial monitor WiFi details
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  

  // Start listening for clients
  server.begin();
  Serial.print("Open Telnet and connect to IP:");
  Serial.print(WiFi.localIP());
  Serial.print(" on port ");
  Serial.println(80);

//Set the pinmode of each sensors's GPIO port
  pinMode(PIR_SENSOR_PIN_1, INPUT);
  pinMode(PIR_SENSOR_PIN_2, INPUT);
  pinMode(HALL_SENSOR_PIN, INPUT);
// Start the DHT11 sensor
  dht.begin();
  // Set up the oxygen sensor 
  while(!oxygen.begin(Oxygen_IICAddress)){
    Serial.println("I2c device number error !");
    delay(1000);
  }
  Serial.println("I2c connect success !");
  
}

//=====================================================================================================================================

void loop() 
{
  // listen for incoming clients
  WiFiClient client = server.available();
  
  if (client) {
    //Client handling code
    if(client.connected())
    {
      Serial.println("Client Connected");
    }
    
    while(client.connected()){      
       // Get Humidity and Temperature.
       float humidity = dht.readHumidity();
       float temperature = dht.readTemperature();
       //client.println(a);
       if (isnan(humidity) || isnan(temperature)) {
         Serial.println("Failed to read from DHT sensor!");
       } 
       // Read data from two PIR sensors
       int PIR1 = digitalRead(PIR_SENSOR_PIN_1);
       int PIR2 = digitalRead(PIR_SENSOR_PIN_2);

      // get the time when either PIR sensor got high voltage
      if (PIR1 == HIGH) {
        firstDetectionTime = millis();
      } 
      if (PIR2 == HIGH) {
        secondDetectionTime = millis();
      }

      // Compare the time difference and reduce the failure detection.
      if (firstDetectionTime - secondDetectionTime >500 && firstDetectionTime - secondDetectionTime < 10000) {
        detectionState = 1;
        // Detection state = 1 for someone enter the room.
      } else if (secondDetectionTime - firstDetectionTime >500 && secondDetectionTime - firstDetectionTime < 10000) {
        detectionState = 2;
        // Detection state = 2 for someone left the room.
      }

      int hallSensorState = digitalRead(HALL_SENSOR_PIN);
    if (hallSensorState == HIGH) {
        HallState = 3;
        // Hallstate = 3 for door close. For seperate the detectionState
      } else {
        HallState = 4;
        // HallState = 4 for door open.
      }

      // Collect oxygen sensor data
      float oxygenData = oxygen.getOxygenData(COLLECT_NUMBER);

      // Send all data to LabView
      client.println(humidity);
      delay(50);
      client.println(temperature); 
      delay(50);
      client.println(detectionState);
      delay(50);
      client.println(HallState);
      delay(50);    
      client.println(oxygenData);
      delay(50);    
       
    }
    delay(100);
    client.stop();
    Serial.println("Client disconnected");    

  }
}
