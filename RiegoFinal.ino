#include "IRremote.h"
#include "LiquidCrystal.h"

//definiciones del termoresistor
int ThermistorPin = A3;
int Vo;
float R1 = 10000;
float logR2, R2, T;
float A = 0.001129148, B = 0.000234125, C = 0.0000000876741;
//float A = 1.009249522e-03, B = 2.378405444e-04, C = 2.019202697e-07;

// definiciones del led
int ledPin = 6;
int PotentPin = A0;
int LDRPin = A1;
int ledMoisture=A2;

//definiciones del led de alarma
#define ledAlarma 13

// definición del ir
int receiver = 3;
IRrecv irrecv(receiver);
decode_results results;
LiquidCrystal lcd(7, 8, 9, 10, 5, 12);



// Valores globales a display
// Valor de la Luz 0..100%
int valorLDR;
int ValPot = 0;
int SensorValue;
int Humidity;
int previousMillis;

// definiciones del reloj
int days;
int hours;
int minutes;
int seconds;

//int Algodón, Olivo, Tomate, Peral, Rosal, Arce Japones,  Níspero, Cactus, Orquidea
int planta = 0;
String sPlantas[9]={"Algodon", "Olivo", "Tomate", "Peral", "Naranja", "Arroz", "Patatas", "Nispero", "Remolacha" };

const int iNumArray=10;
//
int ValTemp[iNumArray]={0,0,0,0,0,0,0,0,0,0};
int FlagFirstTemp=true;
//
int ValHum[iNumArray]={0,0,0,0,0,0,0,0,0,0};
int FlagFirstHum=true;
//
int ValLux[iNumArray]={0,0,0,0,0,0,0,0,0,0};
int FlagFirstLux=true;
//
int flagAlarm=false;
int flagWatering=false;


// Average functions
int Filtra_Medicion(int iValue, int *pArrayValue, int *pFlagFirst)  {
    int iLoop;
    int Average = iValue;
    //
    if (*pFlagFirst) {
        for (iLoop = 0; iLoop<=iNumArray-1; iLoop++) {
          *(pArrayValue + iLoop) = iValue;
        }
        *pFlagFirst = false;
    }
    // 
    for (iLoop = iNumArray-2; iLoop>=0; iLoop--) {
        *(pArrayValue + 1 + iLoop) = *(pArrayValue + iLoop);
    }
    *(pArrayValue + 0) = iValue;
    // 
    Average = 0;
    for (iLoop = 0; iLoop<=iNumArray-1; iLoop++) {
        Average += *(pArrayValue + iLoop);
    }
    Average = (int) round(Average / iNumArray);
    return(Average);
}

//Función del led de emergencia
void turn_on_emergency() {
  if (flagAlarm) {
    digitalWrite(ledAlarma, HIGH);
  }else {
    digitalWrite(ledAlarma, LOW);
  }
}

// Funciones del led, pin 3

void init_led() {
  //
  pinMode(ledAlarma, OUTPUT);
  pinMode(ledPin, OUTPUT);
  //
  Serial.begin(9600);
}
void turn_up_led() {
  // 
  int sensorValue = 0;
  // 0 .. 1023
  sensorValue = analogRead(PotentPin);
  // 0 .. 255  
  ValPot = map(sensorValue, 0, 1023, 0, 255);

  analogWrite(ledPin, ValPot);
  //analogWrite(ledPin,255);

  Serial.println("");
  Serial.print(">>>>>>>> Pot sensorValue=");
  Serial.print(sensorValue);
  Serial.print(" ValPot =");
  Serial.print(ValPot);
  Serial.println(" <<<<<<<<<");
}
void turn_down_led() {
  analogWrite(ledPin, 0);
}

//Funcion del sensor de luz
//100 klx - 100.000 lux máx de unidad de medida de la luz
//
void LDR_Read() {
  int fade;
  fade = analogRead(LDRPin);

  Serial.println("");
  Serial.print("before Lux =");
  Serial.println(fade);
  
  // Filtra medicion
  fade=Filtra_Medicion(fade, &ValLux[0], &FlagFirstLux);

  valorLDR = map(fade, 0, 1023, 0, 100);
 
  Serial.print("Lux fade filtered=");
  Serial.print(fade);
  Serial.print(" valorLDR =");
  Serial.println(valorLDR);

}

void Temp_Read() {
  Vo=analogRead(ThermistorPin);

  Serial.println("");
  Serial.print("before Vo =");
  Serial.println(Vo);
  
  // Filtra medicion
  Vo=Filtra_Medicion(Vo, &ValTemp[0], &FlagFirstTemp);
  
  R2=R1 * (1024 /(float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (A + B*logR2 + C*logR2*logR2*logR2));
  T = T - 273.15;
  //
  // Reajuste.....
  //T = T - 4;
  //

  Serial.println("");
  Serial.print("filtered Vo =");
  Serial.println(Vo);
    
  Serial.print("Temperature: "); 
  Serial.print(T);
  Serial.print(" °C");
  Serial.println("");

  //delay(500);
}

void write_message()  {

  //Serial.print("Planta "); Serial.println(planta);
  //Serial.print("Lux "); Serial.println(valorLDR);
  //Serial.print("Pot "); Serial.println(ValPot);
  //
  //Serial.print( "T = " );
  //Serial.print( T, 1 );
  //Serial.println( "grados" );
  //Serial.print( Humidity, 1 );
  //Serial.println( "%" );
}


//Funciones para el mando
void tipodeplanta() {
  Serial.println("¿ Tipo de planta ( 1-9 )?");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tipo de planta?");
  lcd.setCursor(0, 1);
  lcd.print("Elegir entre 1-9");

  irrecv.enableIRIn();

}

void translateIR() {

  switch (results.value) {
    case 0xFF30CF: Serial.println("1");    planta = 1; break;
    case 0xFF18E7: Serial.println("2");    planta = 2; break;
    case 0xFF7A85: Serial.println("3");    planta = 3; break;
    case 0xFF10EF: Serial.println("4");    planta = 4; break;
    case 0xFF38C7: Serial.println("5");    planta = 5; break;
    case 0xFF5AA5: Serial.println("6");    planta = 6; break;
    case 0xFF42BD: Serial.println("7");    planta = 7; break;
    case 0xFF4AB5: Serial.println("8");    planta = 8; break;
    case 0xFF52AD: Serial.println("9");    planta = 9; break;
    default:
      Serial.println(" Error   ");
  }
  delay(100);
  if (planta > 0) {
    lcd.clear();
    lcd.noCursor();
    lcd.noAutoscroll();

    //empezar a contar una vez elegido la planta:
    previousMillis=millis();
  }
}

void IR_Read() {
  if (irrecv.decode(&results)) {
    translateIR();
    irrecv.resume(); // receive the next value
  }
}

void displayLines() {
  lcd.begin (16, 2);
}

void displayRead() {

  //Datos de los sensores
  int temp=(int) round(T);
  lcd.setCursor(0, 0);
  //lcd.print("T=" + String(temp) + " " + "H=" + String(Humidity) + " " + "L=" + String(valorLDR) + "        ");
  lcd.print("T" + String(T,1) + "c " + "H" + String(Humidity) + "% " + "L" + String(valorLDR) + "K        ");

  //Reloj
  int days;
  int hours;
  int minutes;
  int seconds;
  long timeNow;

  // Constants
  long day = 86400000; // 86400000 milliseconds in a day
  long hour = 3600000; // 3600000 milliseconds in an hour
  long minute = 60000; // 60000 milliseconds in a minute
  long second =  1000; // 1000 milliseconds in a second
 
  timeNow = millis() - previousMillis;
  
  days = timeNow / day ;                                //number of days
  hours = (timeNow % day) / hour;                       //the remainder from days division (in milliseconds) divided by hours, this gives the full hours
  minutes = ((timeNow % day) % hour) / minute ;         //and so on...
  seconds = (((timeNow % day) % hour) % minute) / second;

  if (hours>=24) {
       previousMillis = millis();
       timeNow = millis() - previousMillis;
       days = timeNow / day ;                                //number of days
       hours = (timeNow % day) / hour;                       //the remainder from days division (in milliseconds) divided by hours, this gives the full hours
       minutes = ((timeNow % day) % hour) / minute ;         //and so on...
       seconds = (((timeNow % day) % hour) % minute) / second;
  }

  lcd.setCursor(0, 1);
  lcd.print( (hours < 10 ? "0": "") + String(hours) + ":" + (minutes < 10 ? "0": "") + String(minutes) + ":" + (seconds < 10 ? "0": "") + String (seconds) + "        ");

  //Alarma, Planta y regando
  lcd.setCursor(8, 1);
  if (flagAlarm) {
      lcd.print(" ALARMA!");
      lcd.blink();
  } else {
      lcd.noBlink();
      if (flagWatering == true) {
          lcd.print(" Regando");
      } else {                                       
          lcd.print(" " + sPlantas[planta-1] + "        ");
      }
  }
}

void moistureSensor() {
  pinMode(ledMoisture, INPUT);
}

void moisture_Read() {
  int SensorValue = analogRead(ledMoisture);

  Serial.print ("before SensorValue =");
  Serial.println(SensorValue);
  
  // Filtra medicion
  SensorValue=Filtra_Medicion(SensorValue, &ValHum[0], &FlagFirstHum);

  Humidity = map(SensorValue, 0, 1023, 100, 0);

  Serial.println(""); 
  Serial.print ("***** Humedad filtered SensorValue =");
  Serial.print(SensorValue);
  Serial.print (" % Humidity =");
  Serial.println(Humidity);
  Serial.println("");

}

void setup() {
  init_led();

  turn_down_led();

  displayLines();

  tipodeplanta();

  moistureSensor();

}

void loop() {

  if (planta == 0) {

    IR_Read();


  }   else {

    // Read photo resistor LUX
    LDR_Read();
    //
    moisture_Read();
    //
    Temp_Read();
    //

    flagAlarm=(valorLDR > 80 ? true : false);  
    flagWatering=(Humidity < 50 ? true : false);  
    
    if (flagWatering) {
        turn_up_led();
    } else {
        turn_down_led();      
    }    
    //
    turn_on_emergency();
    //
    displayRead();
    //
    // Enciende Led

    delay(100);
  }
}
