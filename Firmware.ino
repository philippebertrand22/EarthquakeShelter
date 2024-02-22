#include <WiFi.h>
#include <WiFiUdp.h>

#define PIN_LED 2 //define the led pin
#define CHN 0 //define the pwm channel
#define FRQ 1000 //define the pwm frequency
#define PWM_BIT 8 //define the pwm precision
#define PIN_BUZZER 14 // buzzer pin
#define PIN_BUTTON 21 // button pin
#define PRESSURE_SENSOR 1

const char *ssid_Router     = "BELL876"; //Enter the router name
const char *password_Router = "5513DFA6"; //Enter the router password
int analogValue = 0;
bool activatePumps = false;

WiFiUDP Udp;
unsigned int localUdpPort = 4210;  // local port to listen on
char incomingPacket[255];  // buffer for incoming packets
char  replyPacket[] = "Hi there! Got the message :-)";  // a reply string to send back

void setup(){
  Serial.begin(115200);
  delay(2000);
  Serial.println("Setup start");

  //setting pins for buzzer and button
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BUTTON, INPUT);

  ledcSetup(CHN, FRQ, PWM_BIT); //setup pwm channel
  ledcAttachPin(PIN_LED, CHN); //attach the led pin to pwm channel
  ledcWrite(CHN, 255);

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
        activatePumps = true;
      } else if (strcmp(alarmType,"RESET") == 0) {
        digitalWrite(PIN_BUZZER,HIGH);
        delay(500);
        digitalWrite(PIN_BUZZER,LOW);
        activatePumps = false;
      }
    }
  }
  
  if(activatePumps) {
    ledcWrite(CHN, 0);  //active low
    analogValue = analogRead(PRESSURE_SENSOR);
    Serial.println(analogValue);
    if (analogValue > 1000) {
      digitalWrite(PIN_BUZZER,HIGH);
      delay(500);
      activatePumps = false;
    }
  } else {
    ledcWrite(CHN, 255);
  }

  if(!activatePumps) {
    if (digitalRead(PIN_BUTTON) == LOW) {
    digitalWrite(PIN_BUZZER,HIGH);
    ledcWrite(CHN,0);
    } else {
      digitalWrite(PIN_BUZZER,LOW);
      ledcWrite(CHN,255);
    }
  }
}