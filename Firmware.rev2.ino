#include <WiFi.h>
#include <WiFiUdp.h>

//assign pump pinouts
#define PUMP1 47 
#define PUMP2 48 
#define PUMP3 45 
#define PUMP4 0 

//assign PWM Channels pins
#define TOP_CHANNEL 0 
#define RIGHT_CHANNEL 1 
#define LEFT_CHANNEL 2 

#define FRQ 1000 //define the pwm frequency
#define PWM_BIT 8 //define the pwm precision
 
//assign buzzer and button
#define PIN_BUZZER 18 // buzzer pin
#define PIN_BUTTON 7 // button pin
//assign pressure sensors pins
#define PRESSURE_SENSOR_TOP 4 //pressure sensor pin
#define PRESSURE_SENSOR_LEFT 5 //pressure sensor pin
#define PRESSURE_SENSOR_RIGHT 6 //pressure sensor pin

const char *ssid_Router     = "******"; //Enter the router name
const char *password_Router = "******"; //Enter the router password
bool activatePumps = false;
int ADCval1;
int ADCval2;
int ADCval3;

WiFiUDP Udp;
unsigned int localUdpPort = 4210;  // local port to listen on
char incomingPacket[255];  // buffer for incoming packets
char  replyPacket[] = "Hi there! Got the message :-)";  // a reply string to send back
//------------------------------------------------------------------------------------//
void setup(){
  Serial.begin(115200);
  delay(2000);
  Serial.println("Setup start");
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  setupPWM();
  setupWifi();
}

void setupPWM() {
  //setup top, left and right channels
  ledcSetup(TOP_CHANNEL, FRQ, PWM_BIT); //setup top pwm channel
  ledcSetup(LEFT_CHANNEL, FRQ, PWM_BIT); //setup left pwm channel
  ledcSetup(RIGHT_CHANNEL, FRQ, PWM_BIT); //setup right pwm channel
  //attach the pump pin to top(2), right(1), left(1) pwm channels
  ledcAttachPin(PUMP1, TOP_CHANNEL);
  ledcAttachPin(PUMP2, TOP_CHANNEL);
  ledcAttachPin(PUMP3, LEFT_CHANNEL);
  ledcAttachPin(PUMP4, RIGHT_CHANNEL); 
  //set pump state to deactivated (high signal)
  ledcWrite(TOP_CHANNEL, 255);
  ledcWrite(RIGHT_CHANNEL, 255);
  ledcWrite(LEFT_CHANNEL, 255);
}

void setupWifi() {
  WiFi.begin(ssid_Router, password_Router);
  Serial.println(String("Connecting to ") + ssid_Router);

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected, IP address: ");
  Serial.println(WiFi.localIP());

  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);

  Serial.println("Setup End");
}
 
void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if (len > 0){
      incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %s\n", incomingPacket);
    
    char* spacePos = strchr(incomingPacket, ' ');
    if (spacePos != NULL) {
      // Calculate the length of the alarm type
      int alarmTypeLength = spacePos - incomingPacket;
      
      // Allocate memory for the alarm type string
      char alarmType[alarmTypeLength + 1];
      
      // Copy the alarm type string from udpPacket
      strncpy(alarmType, incomingPacket, alarmTypeLength);
      
      // Null-terminate the string
      alarmType[alarmTypeLength] = '\0';

      if(strcmp(alarmType,"ALARM") == 0) {
        warningSound();
        inflateTop();
        inflateRight();
        inflateLeft();
        activatePumps = true;
      } else if (strcmp(alarmType,"RESET") == 0) {
        digitalWrite(PIN_BUZZER,HIGH);
        delay(500);
        digitalWrite(PIN_BUZZER,LOW);
        activatePumps = false;
      }
    }
  }
  
  if(!activatePumps) {
    if (digitalRead(PIN_BUTTON) == LOW) {
      digitalWrite(PIN_BUZZER,HIGH);
      Serial.println("Button pushed");
      ledcWrite(TOP_CHANNEL,0);
      ledcWrite(RIGHT_CHANNEL,0);
      ledcWrite(LEFT_CHANNEL,0);
    } else {
      digitalWrite(PIN_BUZZER,LOW);
      ledcWrite(TOP_CHANNEL,255);
      ledcWrite(RIGHT_CHANNEL,255);
      ledcWrite(LEFT_CHANNEL,255);
    }
  }
}

void warningSound() {
  digitalWrite(PIN_BUZZER,HIGH);
  delay(500);
  digitalWrite(PIN_BUZZER,LOW);
  delay(500);
  digitalWrite(PIN_BUZZER,HIGH);
  delay(500);
  digitalWrite(PIN_BUZZER,LOW);
  delay(500);
  digitalWrite(PIN_BUZZER,HIGH);
  delay(1000);
  digitalWrite(PIN_BUZZER,LOW);
  delay(500);
}

void inflateTop() {
  while(ADCval1 < 1000) {
    ledcWrite(TOP_CHANNEL, 0);  //active low
    ADCval1 = analogRead(PRESSURE_SENSOR_TOP);
    Serial.println(ADCval1);
  } 
  digitalWrite(PIN_BUZZER,HIGH);
  delay(500);
  ledcWrite(TOP_CHANNEL, 255);
}

void inflateRight() {
  while(ADCval2 < 1000) {
    ledcWrite(RIGHT_CHANNEL, 0);  //active low
    ADCval2 = analogRead(PRESSURE_SENSOR_RIGHT);
    Serial.println(ADCval2);
  } 
  digitalWrite(PIN_BUZZER,HIGH);
  delay(500);
  ledcWrite(RIGHT_CHANNEL, 255);
}

void inflateLeft() {
  while(ADCval3 < 1000) {
    ledcWrite(LEFT_CHANNEL, 0);  //active low
    ADCval3 = analogRead(PRESSURE_SENSOR_LEFT);
    Serial.println(ADCval3);
  } 
  digitalWrite(PIN_BUZZER,HIGH);
  delay(500);
  ledcWrite(LEFT_CHANNEL, 255);
}
