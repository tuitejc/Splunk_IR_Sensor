#include <Adafruit_MLX90640.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"


Adafruit_MLX90640 mlx;
float frame[32 * 24]; // buffer for full frame of temperatures

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status


// YOUR SPLUNK IP ADDRESS HERE
IPAddress server(55,55,55,555);  // numeric IP for Google (no DNS)
String payload = ""; //payload of data

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;


void setup() {
  //while (!Serial) delay(10);
  Serial.begin(9600);
  delay(100);

  setupWifi();

  Serial.println("Adafruit MLX90640 Simple Test");
  if (! mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
    Serial.println("MLX90640 not found!");
    while (1) delay(10);
  }
  Serial.println("Found Adafruit MLX90640");

  Serial.print("Serial number: ");
  Serial.print(mlx.serialNumber[0], HEX);
  Serial.print(mlx.serialNumber[1], HEX);
  Serial.println(mlx.serialNumber[2], HEX);

  //mlx.setMode(MLX90640_INTERLEAVED);
  mlx.setMode(MLX90640_CHESS);
  Serial.print("Current mode: ");
  if (mlx.getMode() == MLX90640_CHESS) {
    Serial.println("Chess");
  } else {
    Serial.println("Interleave");
  }

  mlx.setResolution(MLX90640_ADC_18BIT);
  Serial.print("Current resolution: ");
  mlx90640_resolution_t res = mlx.getResolution();
  switch (res) {
    case MLX90640_ADC_16BIT: Serial.println("16 bit"); break;
    case MLX90640_ADC_17BIT: Serial.println("17 bit"); break;
    case MLX90640_ADC_18BIT: Serial.println("18 bit"); break;
    case MLX90640_ADC_19BIT: Serial.println("19 bit"); break;
  }

  mlx.setRefreshRate(MLX90640_2_HZ);
  Serial.print("Current frame rate: ");
  mlx90640_refreshrate_t rate = mlx.getRefreshRate();
  switch (rate) {
    case MLX90640_0_5_HZ: Serial.println("0.5 Hz"); break;
    case MLX90640_1_HZ: Serial.println("1 Hz"); break;
    case MLX90640_2_HZ: Serial.println("2 Hz"); break;
    case MLX90640_4_HZ: Serial.println("4 Hz"); break;
    case MLX90640_8_HZ: Serial.println("8 Hz"); break;
    case MLX90640_16_HZ: Serial.println("16 Hz"); break;
    case MLX90640_32_HZ: Serial.println("32 Hz"); break;
    case MLX90640_64_HZ: Serial.println("64 Hz"); break;
  }
}

void loop() {
  //delay(10000);
  Serial.println("Starting Loop...");
  if (mlx.getFrame(frame) != 0) {
    Serial.println("Failed");
    return;
  }
  Serial.println();
  Serial.println();
 
  //reset the payload
   // payload = "{\"sourcetype\":\"MLX90640_IRSensor\",\"index\":\"irevents\",\"event\":{\"frame\":{";  
  String tempString = "";
   payload = "{\"sourcetype\":\"MLX90640_IRSensor\",\"index\":\"irevents\",\"event\":{\"frame\":[";
  
  for (uint8_t h = 0; h < 24; h++) {
      //tempString.concat("\"Y");tempString.concat(String(h+1));tempString.concat("\":[");
      tempString.concat("[");
    for (uint8_t w = 0; w < 32; w++) {
      //float t = frame[h * 32 + w];
      tempString.concat(String(frame[h * 32 + w],1));
      //determine whether to add comma
      if(w == 31){  
          //nothing      
        }
      else{
          tempString.concat(",");     
        }
     }
     if(h==23){
       tempString.concat("]");
     }
     else {
        tempString.concat("],");
     }
  } 
   payload.concat(tempString);tempString = "";   
   payload.concat("]}}");
  // payload.concat("}}}");
  
   
   connectToHEC();
   
   Serial.println("Finished Loop.  Starting over again.");
}

void printWifiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}
void connectToHEC(){
  client.stop();
  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(server, 8088)) { 
    Serial.println("connected to HEC");
    // Make a HTTP request:
    client.println("POST /services/collector HTTP/1.1");    
    //key
    client.print("Authorization: Splunk ");client.println(SECRET_KEY);
    //needs to have the length in header
    client.print("Content-Length: ");
    client.println(payload.length());
    Serial.print("Payload Size:"); Serial.println(payload.length()); 
    //needs to have a blank line here between header and payload
    client.println();

    //split the payload up into chunks of 2000 too big to send as one giant payload
    int chunk = 0;
    while (chunk < payload.length())
    {
      if(chunk > payload.length()){
         chunk = payload.length();
      }
      Serial.print(payload.substring(chunk, chunk+2000));
      client.print(payload.substring(chunk, chunk+2000));
      chunk = chunk + 2000;
    }
    Serial.println();
    client.println();    

    //Read the response back from the HEC
    while(client.connected()) {
      while (client.available()) {      
      char c = client.read();
      Serial.write(c);
      client.flush();
      }    
    }
    client.stop();
  }
  else{
    Serial.println("Connection failed!");
  }  
 
  client.stop();
}

void setupWifi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  printCurrentNet();
  printWifiData();

}
