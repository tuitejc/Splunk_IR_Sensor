#include <Adafruit_MLX90640.h>

#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"

Adafruit_MLX90640 mlx;
float frame[32 * 24]; // buffer for full frame of temperatures

char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS; // the Wifi radio's status

//Splunk Server IP Address Goes Here
IPAddress server(18, 237, 189, 172); 


WiFiClient client;

void setup()
{
  while (!Serial)
    delay(10);
  Serial.begin(9600);
  delay(100);

  setupWifi();

  Serial.println("Adafruit MLX90640 Simple Test");
  if (!mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire))
  {
    Serial.println("MLX90640 not found!");
    while (1)
      delay(10);
  }
  Serial.println("Found Adafruit MLX90640");

  Serial.print("Serial number: ");
  Serial.print(mlx.serialNumber[0], HEX);
  Serial.print(mlx.serialNumber[1], HEX);
  Serial.println(mlx.serialNumber[2], HEX);

  //mlx.setMode(MLX90640_INTERLEAVED);
  mlx.setMode(MLX90640_CHESS);
  Serial.print("Current mode: ");
  if (mlx.getMode() == MLX90640_CHESS)
  {
    Serial.println("Chess");
  }
  else
  {
    Serial.println("Interleave");
  }

  mlx.setResolution(MLX90640_ADC_18BIT);
  Serial.print("Current resolution: ");
  mlx90640_resolution_t res = mlx.getResolution();
  switch (res)
  {
  case MLX90640_ADC_16BIT:
    Serial.println("16 bit");
    break;
  case MLX90640_ADC_17BIT:
    Serial.println("17 bit");
    break;
  case MLX90640_ADC_18BIT:
    Serial.println("18 bit");
    break;
  case MLX90640_ADC_19BIT:
    Serial.println("19 bit");
    break;
  }

  mlx.setRefreshRate(MLX90640_2_HZ);
  Serial.print("Current frame rate: ");
  mlx90640_refreshrate_t rate = mlx.getRefreshRate();
  switch (rate)
  {
  case MLX90640_0_5_HZ:
    Serial.println("0.5 Hz");
    break;
  case MLX90640_1_HZ:
    Serial.println("1 Hz");
    break;
  case MLX90640_2_HZ:
    Serial.println("2 Hz");
    break;
  case MLX90640_4_HZ:
    Serial.println("4 Hz");
    break;
  case MLX90640_8_HZ:
    Serial.println("8 Hz");
    break;
  case MLX90640_16_HZ:
    Serial.println("16 Hz");
    break;
  case MLX90640_32_HZ:
    Serial.println("32 Hz");
    break;
  case MLX90640_64_HZ:
    Serial.println("64 Hz");
    break;
  }
}

void loop()
{
  delay(10000);
  Serial.println("Starting Loop...");
  if (mlx.getFrame(frame) != 0)
  {
    Serial.println("Failed");
    return;
  }
  Serial.println();
  Serial.println();

  char *tempString[];

  for (uint8_t h = 0; h < 24; h++)
  {
    tempString[h] = "";
    for (uint8_t w = 0; w < 32; w++)
    {
      float t = frame[h * 32 + w];

      tempString[h].concat("{\"X\":");
      tempString[h].concat(String(w));
      tempString[h].concat(",{\"Y\":");
      tempString[h].concat(String(h));
      tempString[h].concat(",{\"Temp\":");
      tempString[h].concat(String(t));
      tempString[h].concat("}");

      //determine whether to add comma
      if (w != 31 && h != 23)
      {
        tempString[h].concat(",");
      }
    }
    Serial.println(tempString[h]);
  }

  tempString[24] = "}\"}";
  connectToHEC(tempString);
  Serial.println("Finished Loop.  Starting over again.");
}

void printWifiData()
{
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

void printCurrentNet()
{
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

void printMacAddress(byte mac[])
{
  for (int i = 5; i >= 0; i--)
  {
    if (mac[i] < 16)
    {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0)
    {
      Serial.print(":");
    }
  }
  Serial.println();
}
void connectToHEC(char payloadArray[])
{
  client.stop();
  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:

  //Get total Payload Size
  int payloadsize = 0;
  for (int i = 0; i < 25; i++)
  {
    payloadsize = payloadsize + payloadArray[i].length();
  }

  if (client.connect(server, 8088))
  {
    Serial.println("connected to HEC");
    // Make a HTTP request:
    client.println("POST /services/collector HTTP/1.1");
    //key
    client.println("Authorization: Splunk 05668621-3a4a-4dac-9423-14582e748d05");
    //needs to have the length in header
    client.print("Content-Length: ");
    client.println(payloadsize);
    Serial.print("Payload Size:");
    Serial.println(payloadsize);
    //needs to have a blank line here between header and payload
    client.println();

    //initial event data
    client.print("{\"sourcetype\":\"MLX90640_IRSensor\",\"index\":\"irevents\",\"event\":\"{\"Data\":");

    //loop through array to send full payload
    for (int x = 0; x < 25; x++)
    {
      {
        Serial.print(payloadArray[x]);
        client.print(payloadArray[x]);
      }
      Serial.println();
      client.println();

      //Read the response back from the HEC
      while (client.connected())
      {
        while (client.available())
        {
          char c = client.read();
          Serial.write(c);
          client.flush();
        }
      }
      client.stop();
    }
    else
    {
      Serial.println("Connection failed!");
    }

    client.stop();
  }

  void setupWifi()
  {
    // check for the WiFi module:
    if (WiFi.status() == WL_NO_MODULE)
    {
      Serial.println("Communication with WiFi module failed!");
      // don't continue
      while (true)
        ;
    }
    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION)
    {
      Serial.println("Please upgrade the firmware");
    }

    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED)
    {
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
