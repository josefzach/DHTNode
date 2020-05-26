/*
  DHTNode

  Reads hourly values from the DHT sensor and send them via Wi-Fi network to the sever.

  TODO:
    - NTP request for clock sync
    - deep sleep
    - Transmit data collected
    - Soft reset if read from DHT22 fails

Programmer settings:
- Board: Generic ESP8266 Module
- Builtin LED: 2
- Upload Speed: 115200
- CPU Frequency: 80Mhz
- Crystal Frequency: 20Mhz
- Flas Size: 1MB (FS 128 OTA)
- Flash Mode: DOUT (compatible)
- Flas Frequency: 40Mhz
- Reset Mode: no dtr
- Debug port: Disabled
- Debug Level: None
- lwIP Variant: v2 lower memory
- VTables: Flash
- Exceptions: Legacy (new can return nullptr)
- Erase Flash: Only Sketch
- 
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "DHTesp.h"
//#include "DHT.h"
#include <EEPROM.h>

#define MSMT_BATCH  48
#define INTVL_SEC 900

//#define MSMT_BATCH  2
//#define INTVL_SEC 60

const short int GPIO2 = 2; //GPIO2
const short int GPIO0 = 0; //GPIO0

String nodeid;
byte countit;

DHTesp dht;

typedef struct {
  float temperature;
  float humidity;
} dhtData;

dhtData dhtDataArray[MSMT_BATCH]; //12h of data @ 4 measurements per hour

//DHT dht(GPIO2, DHT22);

void ok_blink() { 
  // successful boot, short blink 3 times
  digitalWrite(GPIO2, LOW);     // turn the LED on
  delay(100);                    // wait
  digitalWrite(GPIO2, HIGH);    // turn the LED off
  delay(100);                    // wait

  digitalWrite(GPIO2, LOW);     // turn the LED on
  delay(100);                    // wait
  digitalWrite(GPIO2, HIGH);    // turn the LED off
  delay(100);                    // wait

  digitalWrite(GPIO2, LOW);     // turn the LED on
  delay(100);                    // wait
  digitalWrite(GPIO2, HIGH);    // turn the LED off
  delay(100);                    // wait  
}

void connected_blink() { 
  // successful boot, long blink 2 times
  digitalWrite(GPIO2, LOW);     // turn the LED on
  delay(200);                    // wait
  digitalWrite(GPIO2, HIGH);    // turn the LED off
  delay(200);                    // wait

  digitalWrite(GPIO2, LOW);     // turn the LED on
  delay(200);                    // wait
  digitalWrite(GPIO2, HIGH);    // turn the LED off
  delay(200);                    // wait
}

// the setup function runs once when you press reset or power the board
void setup()
{

  // power up DHT sensor, pull DHT GND pin to low
  pinMode(GPIO0, OUTPUT);
  digitalWrite(GPIO0, LOW);
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(GPIO2, OUTPUT);
  digitalWrite(GPIO2, HIGH);    // turn the LED off
  
  delay(1000);                   // wait

  //ok_blink();
  
  Serial.begin(9600);
  Serial.setTimeout(2000);
  // Wait for serial to initialize.
  while(!Serial) { }

  EEPROM.begin( sizeof(dhtDataArray) + sizeof(countit) );
  EEPROM.get(0,countit);

  if( countit >= MSMT_BATCH ) {
    
    WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
    delay(1000);
    WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot, also needed for modem sleep
    //wifi_set_sleep_type(LIGHT_SLEEP_T);
    
    //Serial.println();

    WiFi.begin("ZyXEL7AD474", "3AH9CPHYWTH39");
  
    Serial.print(WiFi.hostname());
    
    //Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      //Serial.print(".");
    }
    //Serial.println();
  
    //connected_blink();
    
  }
 
  // Autodetect is not working reliable, don't use the following line
  // dht.setup(17);
  // use this instead: 
  dht.setup(GPIO2, DHTesp::DHT22); // Connect DHT sensor to GPIO2

  delay(5000);

  HTTPClient http;    //Declare object of class HTTPClient
  
  String getData, Link;
  int httpCode;
  String payload;

  dhtData msmtData;
  
  msmtData.humidity = dht.getHumidity();
  msmtData.temperature = dht.getTemperature();
  
  //float humidity = dht.readHumidity();
  //float temperature = dht.readTemperature();

  // isnan is not working - why?
  if (String(msmtData.temperature) == "nan" || String(msmtData.humidity) == "nan" ) {
    //Serial.println("Failed to read from DHT sensor!");
    msmtData.temperature = -1;
    msmtData.humidity = -1;
  }
    
  Serial.println(String(countit));   

  // store sensor measurement to EEPROM
  EEPROM.put(1+(countit-1)*sizeof(dhtData), msmtData);
  
  if( countit >= MSMT_BATCH ) {

    
    //Serial.print("Connected, IP address: ");
    //Serial.println(WiFi.localIP());
  
    nodeid = WiFi.hostname();

    EEPROM.get(1, dhtDataArray);
    
    //GET Data
    getData = "nodeid=" + nodeid;

    for (int i=0;i<MSMT_BATCH;i++){
      getData = getData + "&temperature[]=" + String(dhtDataArray[i].temperature);
    }
    
    for (int i=0;i<MSMT_BATCH;i++){
      getData = getData + "&humidity[]=" + String(dhtDataArray[i].humidity);
    }

    getData = getData + "&dts=" + String(INTVL_SEC);
    
    Link = "http://192.168.1.48:88/datarec.php?" + getData;
    
    http.begin(Link);     //Specify request destination
    
    httpCode = http.GET();            //Send the request
    payload = http.getString();    //Get the response payload
    
    http.end();  //Close connection

    countit = 0;
  }
  
  //Serial.println(httpCode);   //Print HTTP return code
  //Serial.println(payload);    //Print request response payload
 
  pinMode(GPIO2, OUTPUT);
  digitalWrite(GPIO2, HIGH);

  countit++;
  EEPROM.put(0,countit);
  
  
  // power down DHT sensor
  pinMode(GPIO0, OUTPUT);
  digitalWrite(GPIO0, HIGH); // pull DHT GND pin to HIGH

  EEPROM.commit();
  
  ESP.deepSleep(INTVL_SEC * 1e6); //sleep for 15min
  //delay(5000);  //sleep for five minutes
  
}


// the loop function runs over and over again forever
void loop() {

}
