#include <ESP8266WiFi.h>
#include <AmazonIOTClient.h>
#include <ESP8266AWSImplementations.h>

Esp8266HttpClient httpClient;
Esp8266DateTimeProvider dateTimeProvider;

AmazonIOTClient iotClient;
ActionError actionError;

const char* ssid = "XXXX";
const char* password = "YYYYY";

void initWLAN()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
}

void initAWS()
{
  iotClient.setAWSRegion("eu-west-1");
  iotClient.setAWSEndpoint("amazonaws.com");
  iotClient.setAWSDomain("a14r85vkucwawz-ats.iot.us-east-2.amazonaws.com");
  iotClient.setAWSPath("/things/xscalar/shadow");
  iotClient.setAWSKeyID("AKIAXACGD5G4ZMNY3EFZ");
  iotClient.setAWSSecretKey("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");
  iotClient.setHttpClient(&httpClient);
  iotClient.setDateTimeProvider(&dateTimeProvider);
}

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println("begin");
  initWLAN();
  Serial.println("wlan initialized");
  initAWS();
  Serial.println("iot initialized");
}

void loop()
{
  char shadow[100];
  strcpy(shadow, "{\"state\":{\"reported\":{\"test_value1\":123, \"test_value2\":234}}}");
  
  Serial.println("Trying to send data");
  Serial.print(shadow);
  
  char* result = iotClient.update_shadow(shadow, actionError);
  Serial.print(result);
  
  delay(10000);
}
