#include <HardwareSerial.h>

#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#endif

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "UPIoT"
//#define WIFI_PASSWORD "Beerpong1"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyDrO1gmfCEXjjXwfARqqcbb7pOoVt1HYEM"

/* 3. Define the RTDB URL */
#define DATABASE_URL "theremin-synth-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "rcmadison26@gmail.com"
#define USER_PASSWORD "123456"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;
HardwareSerial Unit1(1);

//THEREMIN PINS
#define trigPin 13 // define trigPin
#define echoPin 14 // define echoPin.
#define MAX_DISTANCE 700 // Maximum sensor distance is rated at 400-500cm. 

void setup()
{
  Unit1.begin(9600, SERIAL_8N1,2,15);
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);

  // Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);

  //THEREMIN SETUP CODE
  pinMode(trigPin,OUTPUT);// set trigPin to output mode
  pinMode(echoPin,INPUT); // set echoPin to input 

}

int myInt = 60;
float timeOut = MAX_DISTANCE * 60;
int soundVelocity = 340; // define sound speed=340m/s
float oldDistance = -1.0;
float noteVelocity = 1.0f;
bool isPitchBendOn = false;

void loop(){
  while(Unit1.available()){
    Serial.printf("Distance: ");
    float distance = Theremin_GetSonar();
    Serial.print(distance); // Send ping, get distance in cm and print result 
    Serial.println("cm");
    
    if (distance < 24.0) { //24 cm away or closer
      if (oldDistance == -1.0) { //first note played (there isn't an existing note to turn off yet)
        //Synth_NoteOn(0, 60 + round(distance/2), noteVelocity);
        Firebase.setInt(fbdo, F("/notes/data"), 60 + round(distance/2));
      }
      else if (distance != oldDistance) { //there was a prior note we need to turn off to play something different
//        Synth_NoteOff(0, 60 + round(oldDistance/2)); 
//        Synth_NoteOn(0, 60 + round(distance/2), noteVelocity);
          Firebase.setInt(fbdo, F("/notes/data"), 60 + round(distance/2));
  //      if (noteVelocity <= 60.0) {
  //        noteVelocity += 10.0;
  //        Serial.printf("Note Vel: ");
  //        Serial.println(noteVelocity);
  //      }
      }
  
      //if (isPitchBendOn) {
        //Synth_PitchBend(0, 10);
      //}
   
      oldDistance = distance;
      int note = 60 + round(distance/2);
      Unit1.write(note);
      delay(150);
      //Firebase.setInt(fbdo, F("/notes/data"), myInt);
    }
  }
}



float Theremin_GetSonar() {
  unsigned long pingTime;
  float distance;
  // make trigPin output high level lasting for 10us to trigger HC_SR04
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Wait HC-SR04 returning to the high level and measure out this waiting time 
  pingTime = pulseIn(echoPin, HIGH, timeOut);
  // calculate the distance according to the time
  distance = (float)pingTime * soundVelocity / 2 / 10000;
  return distance; // return the distance value
}
