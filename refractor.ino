

#include <ArduinoJson.h>
#include <FirebaseArduino.h>
#include <OneWire.h>
#include <ESP8266WiFi.h>
#include <stdio.h>

#define FIREBASE_HOST "lockthedoorplease-f94c0.firebaseio.com"
//database api url
#define FIREBASE_AUTH "kEMTojIWOpygNdtK50bhVgJeH0JjDxnFVPcixDkm"
//database secret


//Connection Auth - Partage de connection
const char* ssid = "NokiaMax";
const char* password = "testtest42";

// Déclaration - Input
OneWire ibutton (14); //Badge
char DIO_Buzzer = 2; //Buzzer
int inputPin = 12; //Detecteur de mouvement
       
// Bouton valide
String btn = "1147252292600178";

// Autres Déclaration
String verif = "";
String firebaseResult;

byte buffer[8];
        
int pirState = LOW;             // we start, assuming no motion detected
int val = 0;
int isStarting = 0;
int value = 0;

bool identifie = false;


void setup() {
  
  Serial.begin(115200);
  pinMode(inputPin, INPUT);
  Serial.println("setup");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

}



String firebaseGet(String s) {
  String x = Firebase.getString(s);
  yield();
  delay(100);
  return x;
  delay(100);
}

String verifButton(byte buffer[8]){
  String verification = "";
   for (int x = 0; x < 8; x++) {
      Serial.print(buffer[x], HEX);
      verification = verification + buffer[x], HEX;
      Serial.print(" ");
    }
    return verification;
}

void loop() {

  verif = "";
  ibutton.reset();

  //Verification si Alarm is ON Avec FireBase
  firebaseResult = firebaseGet("result");
  
  if (firebaseResult == "on") {
    if (!identifie) {
      val = digitalRead(inputPin);  // read input value
      if (val == HIGH) {            // check if the input is HIGH
        if (pirState == LOW) {
          // we have just turned on
          Serial.println("Motion detected!");
          Serial.println("SONNE");

           //Verification si Alarm is ON Avec FireBase
          firebaseResult = firebaseGet("result");
          if (firebaseResult == "off") {
            return;
          }

          //Delai de quelques secondes le temps de s'authetifier
          if (isStarting < 5) {
            digitalWrite(DIO_Buzzer, LOW);
            tone(DIO_Buzzer, 2000, 100);
            delay(1000);
            tone(DIO_Buzzer, 0, 100);
            isStarting++;
          } else {
             //Alarme déclenché
            digitalWrite(DIO_Buzzer, LOW);
            for (int note = 1700; note < 2000; note++) {
              tone(DIO_Buzzer, note, 100);
              delay(1);
            }
            
            for (int note = 2000; note >= 1700; note--) {
              tone(DIO_Buzzer, note, 100);
              delay(1);
            }
            
            //Verification si Alarm is ON Avec FireBase
            firebaseResult = firebaseGet("result");
            if (firebaseResult == "off") {
              return;
            }
            Serial.println("set : message intrus ");
            Firebase.setString("message", "intrus ");
          }

          //Initialisation iButton
          if (!ibutton.search (buffer)) {
            ibutton.reset_search();
            delay(250);
            return;
          }
          //Lecture des données recus par Ibutton
          verif = verifButton(buffer);

          Serial.print("\n");

          // At this point an iButton is found
          Serial.println("1-Wire Device Detected, ID is:");
          
          // Check if this is a iButton
          if ( buffer[0] != 0x01) {
            Serial.println("Device is not a iButton");
          } else {
            Serial.println("Device is a iButton");
          }
          //Ibutton incorrect
          if ( ibutton.crc8( buffer, 7) != buffer[7]) {
            Serial.println("CRC is not valid!");
          }
          if (verif == btn) {
            Serial.println("Alarm OFF by badge");
            noTone(4);
            identifie = true;
            Serial.println("set : message Alarm OFF");
            Firebase.setString("message", "Alarm OFF");
          }

        } else {
          // we have just turned of
          Serial.println("Motion ended!");
          // We only want to print on the output change, not state
          pirState = LOW;
        }
        // Search for an iButton and assign the value to the buffer if found.
        delay(1000);
        Serial.println("Done.\n\n");
      }
    } else {
      
      //Reactive l'alarme
      
      if (!ibutton.search (buffer)) {
        ibutton.reset_search();
        delay(250);
        return;
      }

    verif = verifButton(buffer);

      Serial.print("\n");

      if (verif == btn) {
        Serial.println("Alarm ON");
        identifie = false;
        isStarting = 0;
        firebaseResult = firebaseGet("message");
        Serial.println("set : message Alarm ON");
        Firebase.setString("message", "Alarm ON");
        delay(100);
        Serial.println("RESULTAT:" + firebaseResult);
        Serial.print(Firebase.success());

      }
    }
  } else {
    verif="";
    identifie = false;
    isStarting = 0;
    Serial.println("Alarm if off");

    if (!ibutton.search (buffer)) {
      ibutton.reset_search();
      delay(250);
      return;
    }

    verif = verifButton(buffer);
    
    Serial.print("\n");

    delay(2000);

    if (verif == btn) {
      Serial.println("Alarm ON by badge");
      firebaseResult = firebaseGet("message");
      Serial.println("set : message Alarm ON");
      Firebase.setString("message", "Alarm ON");
      Serial.println("set : result on");
      Firebase.setString("result", "on");
      delay(100);
      Serial.println("RESULTAT:" + firebaseResult);
      Serial.print(Firebase.success());

    }
    delay(1000);
  }
}
