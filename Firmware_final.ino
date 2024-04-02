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
 
//assign buzzer, button and switch pin
#define PIN_BUZZER 18 // buzzer pin
#define PIN_BUTTON 7 // button pin
#define PIN_SWITCH 20 // button pin
//assign pressure sensors pins
#define PRESSURE_SENSOR_RIGHT 4 //pressure sensor pin
#define PRESSURE_SENSOR_TOP 5 //pressure sensor pin
#define PRESSURE_SENSOR_LEFT 6 //pressure sensor pin

#define MAX_TOP_PRESSURE 1500 // (20PSI  ADC value -> (1.3V*4095)/3.3V)
#define MAX_RIGHT_LEFT_PRESSURE 3500 // (20PSI  ADC value -> (2.9V*4095)/3.3V)
#define MIN_TOP_PRESSURE 1100 // (10PSI  ADC value -> (0.9V*4095)/3.3V)
#define MIN_RIGHT_LEFT_PRESSURE 2200 // (10PSI  ADC value -> (1.8V*4095)/3.3V)

WiFiUDP Udp;
const char *ssid_Router     = "Pixel_7107"; //Enter the router name
const char *password_Router = "jesus0214"; //Enter the router password
unsigned int localUdpPort = 4210;  // local port to listen on

bool inflateTop = false;
bool inflateRight = false;
bool inflateLeft = false;
bool checkMinPressureTop = false;
bool checkMinPressureRight = false;
bool checkMinPressureLeft = false;

int ADCvalTop;
int ADCvalLeft;
int ADCvalRight;

unsigned long previousMillis = 0;  // Variable to store the last time ADC was read
const long interval = 10000;  // Interval at which to read ADC values (in milliseconds)

char incomingPacket[255];  // buffer for incoming packets
char  replyPacket[] = "Hi there! Got the message :-)";  // a reply string to send back
//------------------------------------------------------------------------------------//
void setup(){
  Serial.begin(115200);
  delay(2000);
  Serial.println("Setup start");
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_SWITCH, INPUT);
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
  ledcAttachPin(PUMP4, TOP_CHANNEL);
  ledcAttachPin(PUMP3, TOP_CHANNEL);
  ledcAttachPin(PUMP2, LEFT_CHANNEL);
  ledcAttachPin(PUMP1, RIGHT_CHANNEL); 
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
  unsigned long currentMillis = millis();  // Get the current time

  // Check if it's time to read ADC values
  if (currentMillis - previousMillis >= interval) {
    // Save the last time you read the ADC value
    previousMillis = currentMillis; 
    checkMinPressureTop = true;
    checkMinPressureRight = true;
    checkMinPressureLeft = true;
  }
  if (digitalRead(PIN_SWITCH) == HIGH) {
    ledcWrite(TOP_CHANNEL,0);
    ledcWrite(RIGHT_CHANNEL,0);
    ledcWrite(LEFT_CHANNEL,0);
  } else {
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
          ledcWrite(TOP_CHANNEL, 0);
          ledcWrite(RIGHT_CHANNEL, 0);
          ledcWrite(LEFT_CHANNEL, 0);
          inflateTop = true;
          inflateLeft = true;
          inflateRight = true;
        } else if (strcmp(alarmType,"RESET") == 0) {
          digitalWrite(PIN_BUZZER,HIGH);
          delay(500);
          digitalWrite(PIN_BUZZER,LOW);
          inflateTop = false;
          inflateLeft = false;
          inflateRight = false;
        }
      }
    }

    if(inflateTop || inflateRight || inflateLeft) {
      ADCvalTop = analogRead(PRESSURE_SENSOR_TOP);
      if(inflateTop && (ADCvalTop > MAX_TOP_PRESSURE)) { // 10PSI read from 0-100 PSI transducer
          ledcWrite(TOP_CHANNEL, 255);
          inflateTop = false;
          checkMinPressureTop = false;
      } else {
        if(checkMinPressureTop && (ADCvalTop < MIN_TOP_PRESSURE)) { // 10PSI read from 0-100 PSI transducer
          ledcWrite(TOP_CHANNEL, 0);
          inflateTop = true;
          checkMinPressureTop = false;
        }
      }

      ADCvalLeft = analogRead(PRESSURE_SENSOR_LEFT);
      if(inflateLeft && (ADCvalLeft > MAX_RIGHT_LEFT_PRESSURE)) { // 10PSI read from 0-100 PSI transducer
          ledcWrite(LEFT_CHANNEL, 255);
          inflateLeft = false;
          checkMinPressureLeft = false;
      } else {
        if(checkMinPressureLeft && (ADCvalTop < MIN_RIGHT_LEFT_PRESSURE)) { // 10PSI read from 0-100 PSI transducer
          //maxPressureTop = true
          ledcWrite(LEFT_CHANNEL, 0);
          inflateLeft = true;
          checkMinPressureLeft = false;
        }
      }

      ADCvalRight = analogRead(PRESSURE_SENSOR_RIGHT);
      if(inflateRight && (ADCvalRight > MAX_RIGHT_LEFT_PRESSURE)) { // 10PSI read from 0-100 PSI transducer
          ledcWrite(RIGHT_CHANNEL, 255);
          inflateRight = false;
          checkMinPressureRight = false;
      } else {
        if(checkMinPressureRight && (ADCvalTop < MIN_RIGHT_LEFT_PRESSURE)) { // 10PSI read from 0-100 PSI transducer
          //maxPressureTop = true
          ledcWrite(RIGHT_CHANNEL, 0);
          inflateRight = true;
          checkMinPressureRight = false;
        }
      }
    } else {
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
}