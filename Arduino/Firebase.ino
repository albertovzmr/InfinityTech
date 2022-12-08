#include <DHT.h>
#include <DHT_U.h>

#include <FirebaseFS.h>
#include <Firebase_ESP_Client.h>
#include <Utils.h>
#include <common.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#include <ESP8266WiFi.h>

// Insert your network credentials
//#define WIFI_SSID "ALBERTOVAZQUEZ"
//#define WIFI_PASSWORD "1223334444"

// Insert Firebase project API Key
#define API_KEY "AIzaSyB4MoZIjewkVsP1UfZ8P52l48p0ekPhoU0"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://data-sensors-18eca-default-rtdb.firebaseio.com/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

#define DHTPIN 2                                            // Digital pin connected to DHT11
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE); 

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

const int AOUTpin=0;  //Entrada analógica
const int DOUTpin=4;  //Entrada digital
int limit;
int value;

#define         MQ1                       (0)     //define la entrada analogica para el sensor
#define         RL_VALOR             (5)     //define el valor de la resistencia mde carga en kilo ohms
#define         RAL       (9.83)  // resistencia del sensor en el aire limpio / RO, que se deriva de la                                             tabla de la hoja de datos
#define         GAS_LP                      (0)
String inputstring = "";                                                        //Cadena recibida desde el PC
float           LPCurve[3]  =  {2.3,0.21,-0.47};
float           Ro           =  10;

const char* ssid = "ALBERTOVAZQUEZ";
const char* password = "1223334444";
const char* host = "192.168.0.17";
const int port = 80;
const int watchdog = 5000;
unsigned long previousMillis = millis();

void setup(){
  Serial.begin(9600);

  pinMode(DOUTpin, INPUT); 
  
  dht.begin(); 

  while(!Serial) { }

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  //config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  //config.database_url = DATABASE_URL;

  /* Sign up */
  /*if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }*/

  /* Assign the callback function for the long running token generation task */
  /*config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);*/

  Serial.println("Iniciando ...");
   //configuracion del sensor
  Serial.print("Calibrando...\n");
  Ro = Calibracion(MQ1);                        //Calibrando el sensor. Por favor de asegurarse que el sensor se encuentre en una zona de aire limpio mientras se calibra
  Serial.print("Calibracion finalizada...\n");
  Serial.print("Ro=");
  Serial.print(Ro);
  Serial.print("kohm");
  Serial.print("\n");

}

void loop(){

  value= analogRead(AOUTpin); //leemos señal analógica
  limit= digitalRead(DOUTpin);  //leemos señal digital
  Serial.println("Monóxido de carbono: ");
  Serial.println(value);
  String fireMono = String(value);

  Serial.print("LP: ");
  float ppm = porcentaje_gas(lecturaMQ(MQ1)/Ro,GAS_LP);
  Serial.println(ppm);
  String fireLP = String(ppm);

  delay(200);
  
  float h = dht.readHumidity();       
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {                                   
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.print("Humedad: ");  
  Serial.print(h);
  String fireHumid = String(h);// + String("%");                   //Humidity integer to string conversion
  
  Serial.print("%  Temperatura: ");  
  Serial.print(t);  
  Serial.println("°C ");
  String fireTemp = String(t);// + String("°C");                  //Temperature integer to string conversion
  //delay(5000);

  /*Firebase.RTDB.pushString(&fbdo, "/grapHumedad/Humedad", fireHumid);
  Firebase.RTDB.pushString(&fbdo, "/grapTemperatura/Temperature", fireTemp);
  Firebase.RTDB.pushString(&fbdo, "/grapCO/Monoxido", fireMono);
  Firebase.RTDB.pushString(&fbdo, "/grapLP/GasLP", fireLP);*/

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis > watchdog) {
    previousMillis = currentMillis;
    WiFiClient client;

    if (!client.connect(host, port)) {
      Serial.println("Falló al conectar");
      return;
    }
    
    String url = "/CarSensors/index.php?temp=";
    url += fireTemp;
    url += "&hum=";
    url += fireHumid;
    url += "&mono=";
    url += fireMono;
    url += "&lp=";
    url += fireLP;
    Serial.println('Datos enviados: ' + url);
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
      "Host: " + host + "\r\n" + 
      "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
          Serial.println(">>> Client Timeout !");
          client.stop();
          return;
      }
    }

    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }

  }

  delay(5000);

}

float calc_res(int raw_adc)
{
  return ( ((float)RL_VALOR*(1023-raw_adc)/raw_adc));
}
 
float Calibracion(float mq_pin){
  int i;
  float val=0;
    for (i=0;i<50;i++) {                                                                               //tomar múltiples muestras
    val += calc_res(analogRead(mq_pin));
    delay(500);
  }
  val = val/50;                                                                                         //calcular el valor medio
  val = val/RAL;
  return val;
}
 
float lecturaMQ(int mq_pin){
  int i;
  float rs=0;
  for (i=0;i<5;i++) {
    rs += calc_res(analogRead(mq_pin));
    delay(50);
  }
rs = rs/5;
return rs;
}
 
int porcentaje_gas(float rs_ro_ratio, int gas_id){
   if ( gas_id == GAS_LP ) {
     return porcentaje_gas(rs_ro_ratio,LPCurve);
   }
  return 0;
}
 
int porcentaje_gas(float rs_ro_ratio, float *pcurve){
  return (pow(10, (((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}