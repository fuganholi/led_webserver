#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

const char* ssid = "REDE-CASA-ENIO";
const char* password = "fuganholi121";

AsyncWebServer server(80);

IPAddress local_IP(192, 168, 0, 107);

IPAddress gateway(192, 168, 0, 1);

IPAddress subnet(255, 255, 255, 0);

#define LED_PIN 13

#define NUMPIXELS 30

bool IS_ON = true;
bool SWITCH_START = true;

uint8_t rgb[3] = {};

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup(){
  
  Serial.begin(115200);

  pixels.begin();

  pinMode(2,OUTPUT);

  if(SWITCH_START){
    for(int i=0;i<NUMPIXELS;i++)
    {
      pixels.setPixelColor(i, pixels.Color(255,255,255));
      pixels.show();
      delay(25);
    }
    digitalWrite(2,HIGH);
    SWITCH_START = false;
  };
  
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  if (!WiFi.config(local_IP,gateway,subnet)) {
    Serial.println("STA Failed to configure");
  }
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  delay(3000);

  Serial.print("connected to ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("==================== CLIENT CONNECTED ====================");
    request->send(SPIFFS, "/index.html");
  });
  
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("ON REQUIRED");
    request->send(SPIFFS, "/index.html");
    if(!IS_ON){
      digitalWrite(2,HIGH);
      for(int i=0;i<NUMPIXELS;i++){
        pixels.setPixelColor(i, pixels.Color(255,255,255)); // Moderately bright green color.
        pixels.show(); // This sends the updated pixel color to the hardware.
        delay(10); // Delay for a period of time (in milliseconds).
      }
      IS_ON = true;
    }else{};
//    request->send(SPIFFS, "/index.html");
  });
  
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("TURN OFF BUTTON CLICKED");
    if(IS_ON){
      for(int i=0;i<NUMPIXELS;i++){
        pixels.setPixelColor(i, pixels.Color(0,0,0));
        pixels.show(); // This sends the updated pixel color to the hardware.
        delay(40); // Delay for a period of time (in milliseconds).
      }
      digitalWrite(2,LOW);
      IS_ON = false;
    }else{};
    request->send(SPIFFS, "/index.html");
  });

  server.on("/color", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("COLOR SET:");
    if(IS_ON){
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* w = request->getParam(i);
        rgb[i]= w->value().toInt();
        Serial.println(w->name() + " = " + rgb[i]);
      };
      for(int j=0;j<NUMPIXELS;j++){
        pixels.setPixelColor(j, pixels.Color(rgb[0],rgb[1],rgb[2]));
        pixels.show();
        delay(1);
      };
    }
    request->send(SPIFFS, "/index.html");
  });

  server.on("/rnbw", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("rainbow");
    if(IS_ON){
      rainbow();
    }else{};    
    request->send(SPIFFS, "/index.html");
  });

  server.on("/pulse", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("pulse");
    if(IS_ON){
      int params = request->params();
      uint8_t p = 100;
      for(int i=0;i<params;i++){
        AsyncWebParameter* w = request->getParam(i);
        p = w->value().toInt();
        Serial.println(w->name() + " = " + p);
      };
      p = map(p,0,100,1000,0);
      pulse(p);
    }else{};
    request->send(SPIFFS, "/index.html");
  });
  
  server.begin();
};
 
void loop(){
};

void rainbow(){
  uint16_t i, j;
  for(j=0; j<256*5; j++) {
    for(i=0; i< NUMPIXELS; i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / NUMPIXELS) + j) & 255));
    }
    pixels.show();
    delay(10);
  }
};

uint32_t Wheel(byte WheelPos){
  if(WheelPos < 85){
    return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } 
  else{
    if(WheelPos < 170){
     WheelPos -= 85;
     return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } 
    else{
     WheelPos -= 170;
     return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
  }
};

void pulse(uint8_t p){
  for(int j=0;j<NUMPIXELS;j++){
    pixels.setPixelColor(j, pixels.Color(0,0,0));
    pixels.show();
    delay(0);
  }
  delay(p);
  for(int j=0;j<NUMPIXELS;j++){
    pixels.setPixelColor(j, pixels.Color(rgb[0],rgb[1],rgb[2]));
    pixels.show();
    delay(0);
  }
  delay(p);  
};
