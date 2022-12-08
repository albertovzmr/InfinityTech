#include <Servo.h>
#include <SoftwareSerial.h>
#include "DHT.h"

#define         MQ1                       (0)     //define la entrada analogica para el sensor
#define         RL_VALOR             (5)     //define el valor de la resistencia mde carga en kilo ohms
#define         RAL       (9.83)  // resistencia del sensor en el aire limpio / RO, que se deriva de la                                             tabla de la hoja de datos
#define         GAS_LP                      (0)
String inputstring = "";                                                        //Cadena recibida desde el PC
float           LPCurve[3]  =  {2.3,0.21,-0.47};
float           Ro           =  10;

const int AOUTpin=1;  //Entrada analógica
const int DOUTpin=4;  //Entrada digital
int limit;
int value;

#define DHTPIN 7  // se asigna pin al sensor de humedad
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
Servo myservo1;  // create servo object to control a servo
char unChar;
String readString; //Asignamos la palabra readString a una variable tipo cadena
String comando;

int izqA = 5; 
int izqB = 6; 
int derA = 9; 
int derB = 10; 
int vel = 255;            // Velocidad de los motores (0-255)
int estado = 'g';         // inicia detenido
int pecho = 3;            // define el pin 2 como (pecho) para el Ultrasonido
int ptrig = 2;            // define el pin 3 como (ptrig) para el Ultrasonido
int duracion, distancia;  // para Calcular distacia


const int pinBuzzer = 12;

void setup()  { 
  pinMode(pinBuzzer, OUTPUT);

  Serial.begin(9600);
  Ro = Calibracion(MQ1);                        //Calibrando el sensor. Por favor de asegurarse que el sensor se encuentre en una zona de aire limpio mientras se calibra

  pinMode(DOUTpin, INPUT);

 dht.begin(); //para el sensor temperatura
  
  myservo1.attach(11);  // Define el pin 11
  
  pinMode(derA, OUTPUT);
  pinMode(derB, OUTPUT);
  pinMode(izqA, OUTPUT);
  pinMode(izqB, OUTPUT);
  
  pinMode(pecho, INPUT);   // define el pin 2 como entrada (pecho) 
  pinMode(ptrig,OUTPUT);   // define el pin 3 como salida  (ptrig) 
  digitalWrite(ptrig, LOW);
  pinMode(13,OUTPUT);
}

void loop() {

  digitalWrite(ptrig, HIGH);   // genera el pulso de trigger por 10us
  delayMicroseconds(10);
  digitalWrite(ptrig, LOW);
  
  duracion = pulseIn(pecho, HIGH);              // Lee el tiempo del Echo
  distancia = duracion/59;           // calcula la distancia en centimetros
  //Serial.println(distancia);

  if(Serial.available()){        // lee el bluetooth y almacena en estado
      estado = Serial.read();
      comando.concat(estado);
      unChar = Serial.read();
      if(unChar=='s'){ //Si lee "s" se envía una función al servomotor
      motor1();
        }
    }

  if (estado=='a'){        // Va hacia adelante 
    analogWrite(derB, 0);     
    analogWrite(izqB, 0); 
    analogWrite(derA, vel);  
    analogWrite(izqA, vel); 
     }
  
  if(estado=='d'){          // Boton IZQ 
    analogWrite(derB, 0);     
    analogWrite(izqB, 0);
    analogWrite(izqA, 0);
    analogWrite(derA, vel);
  }
  if(estado=='c'){         // Boton Parar
    analogWrite(derB, 0);     
    analogWrite(izqB, 0); 
    analogWrite(derA, 0);    
    analogWrite(izqA, 0);
    digitalWrite(pinBuzzer, LOW);  
  }
  if(estado=='b'){          // Boton DER
    analogWrite(derB, 0);     
    analogWrite(izqB, 0); 
    analogWrite(derA, 0);  
    analogWrite(izqA, vel); 
  }

  if(estado=='e'){          // Boton Reversa
      analogWrite(derA, 0);    
      analogWrite(izqA, 0);
      analogWrite(derB, vel);  
      analogWrite(izqB, vel);   
      digitalWrite(pinBuzzer, HIGH);
      digitalWrite(13, LOW);
      delay(300);
      digitalWrite(pinBuzzer, LOW);
      delay(300);
      digitalWrite(13, HIGH);
      delay(300);
    }

  if (estado=='w') {
    digitalWrite(13,HIGH); 
  }

  if (estado=='x') {
    digitalWrite(13, LOW);
  }

  if (estado=='z') {
    //Serial.print("LP:");
    Serial.print(porcentaje_gas(lecturaMQ(MQ1)/Ro,GAS_LP) );
    //Serial.print( "ppm" );
    //Serial.print("    ");
    delay(1000);
  }

  if (estado=='y') {
    value= analogRead(AOUTpin); //leemos señal analógica
    limit= digitalRead(DOUTpin);  //leemos señal digital
    //Serial.println("");
    //Serial.print("CO value: ");
    Serial.println(value);  
    //Serial.print("Limit: ");
    //Serial.print(limit);  
    //Serial.println("");
    //delay(1000);
    delay(1000);
  }
  
     
     
     /*if (distancia <= 15 && distancia >=2){    // si la distancia es menor de 15cm
        digitalWrite(13,LOW);                 // Enciende LED
        
        analogWrite(derB, 0);                  // Parar los motores por 200 mili segundos
        analogWrite(izqB, 0); 
        analogWrite(derA, 0);    
        analogWrite(izqA, 0); 
        delay (200);
        
        analogWrite(derB, vel);               // Reversa durante 500 mili segundos
        analogWrite(izqB, vel); 
        delay(500);           
        
        analogWrite(derB, 0);                // Girar durante 1100 milisegundos   
        analogWrite(izqB, 0); 
        analogWrite(derA, 0);  
        analogWrite(izqA, vel);  
        delay(1100);

	      analogWrite(derB, 0);            // Parar los motores por 200 mili segundos
        analogWrite(izqB, 0); 
        analogWrite(derA, 0);    
        analogWrite(izqA, 0); 
        delay (200);
     }*/

     
     
     
  if(estado=='g'){          // Boton OFF, detiene los motores no hace nada 
     analogWrite(derB, 0);     
     analogWrite(izqB, 0); 
     analogWrite(derA, 0);    
     analogWrite(izqA, 0);
     }
     
  if (estado=='o'){ //imprime el valor del sensor ultrasonico
     Serial.println(distancia);
     delay(100);
  }

  if (estado=='h'){ //imprime el valor del sensor humedad
    int h = dht.readHumidity();// Lee la humedad
    Serial.println (h);
    delay(1000);
  }

  if (estado=='t'){ //imprime el valor del sensor temperatura
    int t= dht.readTemperature();//Lee la temperatura
    Serial.println (t);
    delay(1000);
  }
  
}

void motor1(){
        
  while (Serial.available()) { //El dato numérico del datos enviados por el servomotor es recibido
          //delayMicroseconds(100);                  
          char c = Serial.read();  // Se leen los caracteres que ingresan por el puerto
          readString += c;         // Cada uno de caracteres se convierte en un string
        }
    if (readString.length() >0) {   //la longitud del dato es verificado
          Serial.println(readString.toInt());  //Aquí uno envía los datos al serial y servo
          myservo1.write(readString.toInt());
          readString=""; // Limpia datos almacenados
        }
        delay(1);  //Se define el delay
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