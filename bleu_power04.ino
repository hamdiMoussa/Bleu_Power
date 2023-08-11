//----------------------------------------Include the NodeMCU ESP8266 Library
//----------------------------------------see here: https://www.youtube.com/watch?v=8jMr94B8iN0 to add NodeMCU ESP12E ESP8266 library and board (ESP8266 Core SDK)
 #include <WiFi.h>
#include <WiFiClientSecure.h>
//----------------------------------------
//----------------------------------------Include the DHT Library
#include "DHT.h"
//----------------------------------------

#define DHTTYPE DHT22 //--> Defines the type of DHT sensor used (DHT11, DHT21, and DHT22), in this project the sensor used is DHT11.

const int DHTPin = 5; //--> The pin used for the DHT11 sensor is Pin D1 = GPIO5
DHT dht(DHTPin, DHTTYPE); //--> Initialize DHT sensor, DHT dht(Pin_used, Type_of_DHT_Sensor);

#define ON_Board_LED 2  //--> Defining an On Board LED, used for indicators when the process of connecting to a wifi router

#include <SoftwareSerial.h>

SoftwareSerial mySerial(16, 17);  // RX, TX

//----------------------------------------SSID and Password of your WiFi router.
//Things to change
const char * ssid = "Redmi";
const char * password = "moussa07";
//----------------------------------------

//----------------------------------------Host & httpsPort
const char* host = "script.google.com";
const int httpsPort = 443;
//----------------------------------------

WiFiClientSecure client; //--> Create a WiFiClientSecure object.

String GAS_ID = "AKfycbzZsDV1e4DBvj_D7OlQsSV3NJasnwEM82se8400h0f63tCuGknc42Tn0m8dpM2exI0"; //--> spreadsheet script ID

//============================================================================== void setup
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(500);

  mySerial.begin(9600);  // Utiliser mySerial au lieu de Serial2
  delay(3000);
  test_sim800_module();
  delay(500);


  dht.begin();  //--> Start reading DHT11 sensors
  delay(500);
  
  WiFi.begin(ssid, password); //--> Connect to your WiFi router
  Serial.println("");



}
//==============================================================================
//============================================================================== void loop
void loop() {

  float battery_capacity_Ah = 24;

  float t = dht.readTemperature();

float V1 = (float)analogRead(36) / 4096 * 12 * 3810 / 4000;
float A1 = 50;
float time_interval = 10; // Time interval in seconds
// Calculate charge as a percentage of the battery's capacity
float charge = (A1 * time_interval) / (battery_capacity_Ah * 1.0) * 100; // Charge as a percentage
float P_charge = V1 * A1; // Charge Power in Watts
float remaining_capacity_Ah1 = battery_capacity_Ah - (A1 * time_interval / 3600); // Convert time to hours
float Temps_charge = remaining_capacity_Ah1 / A1; // Estimated time in hours


  float V2 =  (float)analogRead(34) / 4096 * 24;
  float A2 = 82;

// Calculate charge as a percentage of the battery's capacity
float decharge = (A2 * time_interval) / (battery_capacity_Ah * 1.0) * 100; // Charge as a percentage
float P_decharge = V2 * A2; // Charge Power in Watts
float remaining_capacity_Ah = battery_capacity_Ah - (A2 * time_interval / 3600); // Convert time to hours
float Temps_decharge = remaining_capacity_Ah / A2; // Estimated time in hours





    //----------------------------------------Wait for connection
  Serial.print("Connecting");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("...");
    test_sim800_module();
    delay(500);
    access_url(t, V1, V2, A1, A2, P_charge, charge, Temps_charge, P_decharge, decharge, Temps_decharge);
  }

  //----------------------------------------If successfully connected to the wifi router, the IP Address that will be visited is displayed in the serial monitor
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //----------------------------------------

  client.setInsecure();
      // Wi-Fi is connected, continue sending data
  sendData(t, V1, V2, A1, A2, P_charge, charge, Temps_charge, P_decharge, decharge, Temps_decharge);


  delay(10000);



  
}
//==============================================================================
//============================================================================== void sendData
// Subroutine for sending data to Google Sheets
void sendData(float tem, float V1, float V2, float A1, float A2, float P_charge, float charge, float Temps_charge, float P_decharge, float decharge, float Temps_decharge) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    ESP.restart();
    return;
  }
  //----------------------------------------

  //----------------------------------------Processing data and sending data
  String string_temperature =  String(tem);

  String string_V1 =  String(V1);
  String string_A1 =  String(A1);
  String string_P_charge =  String(P_charge);
  String string_charge =  String(charge);
  String string_Temps_charge =  String(Temps_charge);


  String string_V2 =  String(V2);
  String string_A2 =  String(A2); 
  String string_P_decharge =  String(P_decharge);
  String string_decharge =  String(decharge);
  String string_Temps_decharge =  String(Temps_decharge);

  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_temperature + "&V1=" + string_V1 + "&A1=" + string_A1 + "&P_charge=" + string_P_charge + "&charge=" + string_charge + "&Temps_charge=" + string_Temps_charge + "&V2=" + string_V2 + "&A2=" + string_A2+ "&P_decharge=" + string_P_decharge + "&decharge=" + string_decharge + "&Temps_decharge=" + string_Temps_decharge;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  //----------------------------------------

  //----------------------------------------Checking whether the data was sent successfully or not
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp32/Arduino CI successfull!");
  } else {
    Serial.println("esp32/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  //----------------------------------------
} 



void test_sim800_module() {
  mySerial.println("AT");
  updateSerial();
  mySerial.println("AT+CSQ");
  updateSerial();
  mySerial.println("AT+CCID");
  updateSerial();
  mySerial.println("AT+CREG?");
  updateSerial();
  mySerial.println("ATI");
  updateSerial();
  mySerial.println("AT+CBC");
  updateSerial();
}

void updateSerial() {
  delay(500);
  while (Serial.available()) {
    mySerial.write(Serial.read());  // Transférer ce que Serial a reçu vers le port mySerial
  }
  while (mySerial.available()) {
    Serial.write(mySerial.read());  // Transférer ce que mySerial a reçu vers le port Serial
  }
}

void access_url(float tem, float V1, float V2, float A1, float A2, float P_charge, float charge, float Temps_charge, float P_decharge, float decharge, float Temps_decharge) {
  String string_temperature =  String(tem);

  String string_V1 =  String(V1);
  String string_A1 =  String(A1);
  String string_P_charge =  String(P_charge);
  String string_charge =  String(charge);
  String string_Temps_charge =  String(Temps_charge);


  String string_V2 =  String(V2);
  String string_A2 =  String(A2); 
  String string_P_decharge =  String(P_decharge);
  String string_decharge =  String(decharge);
  String string_Temps_decharge =  String(Temps_decharge);

  mySerial.println("AT");
  updateSerial();
  mySerial.println("AT+CFUN?");
  updateSerial();
  mySerial.println("AT+SAPBR=3,1,CONTYPE,GPRS");
  updateSerial();

  mySerial.println("AT+CSTT=\"gprs.tn\",\"gprs\",\"gprs\"");  // Correction de la syntaxe AT+CSTT
  updateSerial();
  mySerial.println("AT+SAPBR=1,1");
  updateSerial();
  mySerial.println("AT+SAPBR=2,1");
  updateSerial();
  mySerial.println("AT+HTTPINIT");
  updateSerial();
  mySerial.println("AT+HTTPSSL=1");
  updateSerial();
  mySerial.println("AT+HTTPPARA=\"CID\",1");  // Correction de la syntaxe AT+HTTPPARA
  updateSerial();
  mySerial.println("AT+HTTPPARA=\"URL\",\"https://script.google.com/macros/s/AKfycbzZsDV1e4DBvj_D7OlQsSV3NJasnwEM82se8400h0f63tCuGknc42Tn0m8dpM2exI0/exec?temperature=" + string_temperature + "&V1=" + string_V1 + "&A1=" + string_A1 + "&P_charge=" + string_P_charge + "&charge=" + string_charge + "&Temps_charge=" + string_Temps_charge + "&V2=" + string_V2 + "&A2=" + string_A2+ "&P_decharge=" + string_P_decharge + "&decharge=" + string_decharge + "&Temps_decharge=" + string_Temps_decharge + "\"");  // Correction de la syntaxe AT+HTTPPARA
  updateSerial();
  mySerial.println("AT+HTTPACTION=0");
  updateSerial();
  mySerial.println("AT+HTTPREAD");
  updateSerial();
  mySerial.println("AT+HTTPREAD");
  updateSerial();
  mySerial.println("AT+HTTPREAD");
  updateSerial();
}


//==============================================================================