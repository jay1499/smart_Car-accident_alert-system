#include<SoftwareSerial.h>
//#include <TinyGPS.h> 

SoftwareSerial gsm(10,11); //make RX arduino line is pin 2, make TX arduino line is pin 3.
//SoftwareSerial gps(4,5);
SoftwareSerial ESP8266(2,3);

#define x A1
#define y A2
#define z A3

int buzzer = 12;
float lati = 12.9410,lon = 77.5655;
String latitude="",longitude="";
String myAPIkey = "VKS0U28B37ENU9VS"; 

long writingTimer = 17; 
long startTime = 0;
long waitTime = 0;
boolean relay1_st = false; 
boolean relay2_st = false; 
unsigned char check_connection=0;
unsigned char times_check=0;
boolean error;

int xsample=0;
int ysample=0;
int zsample=0;

#define samples 10
#define minVal -50
#define MaxVal 50

//int i=0,k=0;
//int  gps_status=0;                      
//String Speed="";
//String gpsString="";
//char *test="$GPRMC";

void initModule(String cmd, char *res, int t)
{
  while(1)
  {
    Serial.println(cmd);
    gsm.println(cmd);
    delay(100);
    while(gsm.available()>0)
    {
       if(gsm.find(res))
       {
        Serial.println(res);
        delay(t);
        return;
       }

       else
       {
        Serial.println("Error");
       }
    }
    delay(t);
  }
}

void setup() 
{
  gsm.begin(9600);
  Serial.begin(9600);

  Serial.println("Initializing....");
  initModule("AT","OK",1000);
  initModule("ATE1","OK",1000); // characters are echoed from the keyboard to the screen.
  initModule("AT+CPIN?","READY",1000);  //sim card is inserted properly
  initModule("AT+CMGF=1","OK",1000);     
  initModule("AT+CNMI=2,2,0,0,0","OK",1000);  
  Serial.println("Initialized Successfully");

  for(int i=0;i<samples;i++)
  {
    xsample+=analogRead(x);
    ysample+=analogRead(y);
    zsample+=analogRead(z);
  }

  xsample/=samples;
  ysample/=samples;
  zsample/=samples;

  Serial.println(xsample);
  Serial.println(ysample);
  Serial.println(zsample);
  delay(1000);  
  
  Serial.println("System Ready..");
  ESP8266.begin(9600); 
  
  startTime = millis(); 
  ESP8266.println("AT+RST");
  delay(2000);
  Serial.println("Connecting to Wifi");
   while(check_connection==0)
  {
    Serial.print(".");
    ESP8266.print("AT+CWJAP=\"No device found\",\"kusu@123\"\r\n");
    ESP8266.setTimeout(5000);   
    Serial.println("WIFI CONNECTED");
    break;
  }
  pinMode(12,OUTPUT);
  digitalWrite(12,LOW);
  
}

void loop() 
{
    int value1=analogRead(x);
    int value2=analogRead(y);
    int value3=analogRead(z);

    int xValue=xsample-value1;
    int yValue=ysample-value2;
    int zValue=zsample-value3;
    
    Serial.print("x=");
    Serial.println(xValue);
    Serial.print("y=");
    Serial.println(yValue);
    Serial.print("z=");
    Serial.println(zValue);

    if(xValue < minVal || xValue > MaxVal  || yValue < minVal || yValue > MaxVal  || zValue < minVal || zValue > MaxVal )
    {
      
        digitalWrite(12,HIGH);
        delay(2000);
        digitalWrite(12,LOW);      
        get_gps();
        Serial.println("Sending SMS");
        Send();
        Serial.println("SMS Sent");
        delay(2000);
        Serial.println("System Ready");
  
        waitTime = millis()-startTime;   
        if (waitTime > (writingTimer*1000)) 
        {
          readSensors();
          writeThingSpeak();
          startTime = millis();   
        }
     }
       delay(1000); 
}
         


void get_gps()
{
    latitude = String(lati,6); 
    longitude = String(lon,6); 
    Serial.println(latitude+";"+longitude); 
    delay(1000); 
      
 }

void Send()
{ 
   gsm.println("AT");
   delay(500);
   serialPrint();
   gsm.println("AT+CMGF=1");
   delay(500);
   serialPrint();
   gsm.print("AT+CMGS=");
   gsm.print('"');
   gsm.print("+919871708626");    //mobile no. for SMS alert
   gsm.println('"');
   delay(500);
   serialPrint();
   gsm.print("Accident has occured, help required.The gps coordinates are latitude:12.910 and longitude:77.56");
   gsm.print("http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=");
   gsm.print(latitude);
   gsm.print("+");              
   gsm.print(longitude);
   gsm.write(char(26));
   delay(2000);
   serialPrint();
}

void serialPrint()
{
  while(gsm.available()>0)
  {
    Serial.print(gsm.read());
  }
}

void readSensors(void)
{
  latitude = lati;
  longitude = lon;
}

void writeThingSpeak(void)
{
  startThingSpeakCmd();
  // preparacao da string GET
  String getStr = "GET /update?api_key=";
  getStr += myAPIkey;
  getStr +="&field1=";
  getStr += String(latitude);
  getStr +="&field2=";
  getStr += String(longitude);
  getStr += "\r\n\r\n";
  GetThingspeakcmd(getStr); 
}

void startThingSpeakCmd(void)
{
  ESP8266.flush();
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com IP address
  cmd += "\",80";
  ESP8266.println(cmd);
  Serial.print("Start Commands: ");
  Serial.println(cmd);

  if(ESP8266.find("Error"))
  {
    Serial.println("AT+CIPSTART error");
    return;
  }
}

String GetThingspeakcmd(String getStr)
{
  String cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  ESP8266.println(cmd);
  Serial.println(cmd);

  if(ESP8266.find(">"))
  {
    ESP8266.print(getStr);
    Serial.println(getStr);
    delay(500);
    String messageBody = "";
    while (ESP8266.available()) 
    {
      String line = ESP8266.readStringUntil('\n');
      if (line.length() == 1) 
      { 
        messageBody = ESP8266.readStringUntil('\n');
      }
    }
    Serial.print("MessageBody received: ");
    Serial.println(messageBody);
    return messageBody;
  }
  else
  {
    ESP8266.println("AT+CIPCLOSE");     
    Serial.println("AT+CIPCLOSE"); 
  } 
}
