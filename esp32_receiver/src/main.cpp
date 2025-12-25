#include <WiFi.h>
#include <esp_now.h>

#include <FastLED.h> 
#include <vector>
#include <esp_wifi.h>

#define FRAME_WIDTH   30
#define FRAME_HEIGHT  30
#define TOTAL_PIXELS  (FRAME_WIDTH * FRAME_HEIGHT
#define PIXELS_PER_CHUNK (8 * 30) 

#define BROCHE        26 

volatile uint8_t assembledScreen[TOTAL_PIXELS];

CRGB leds[TOTAL_PIXELS];

volatile bool screenFilled = false; 
volatile uint8_t packetsReceived = 0; 
volatile int8_t lastIndex = -1; 
volatile bool displayRequested = false; 

uint8_t colorToDisplay[3] = {255, 0, 255}; 



typedef struct struct_message {
  uint8_t index;
  uint8_t nbRows;
  uint8_t color[3];
  byte pixels[PIXELS_PER_CHUNK]; //size 249 bytes
} struct_message;

void onDataReceived(const uint8_t * mac, const uint8_t *incomingData, int len) {
    struct_message receivedMessage;

    size_t copySize = min(len, (int)sizeof(struct_message)); 
    memcpy(&receivedMessage, incomingData, copySize);

    colorToDisplay[0] = receivedMessage.color[0];
    colorToDisplay[1] = receivedMessage.color[1];
    colorToDisplay[2] = receivedMessage.color[2];


    uint16_t offset = receivedMessage.index * PIXELS_PER_CHUNK;
    

    size_t dataLength = receivedMessage.nbRows * FRAME_WIDTH; 
    
    if (offset + dataLength > TOTAL_PIXELS) {
        Serial.println("Erreur: Débordement de mémoire potentiel. Paquet ignoré.");
        return;
    }

    memcpy((uint8_t*)assembledScreen + offset, receivedMessage.pixels, dataLength);
    

    if (receivedMessage.index == 3) {
      screenFilled = true;
      packetsReceived = 0;
    }
}
  
void calculate_image() {
  
  fill_solid(leds, TOTAL_PIXELS, CRGB::Black); 
  
  CRGB activeColor(colorToDisplay[0], colorToDisplay[1], colorToDisplay[2]);

  for(int i=0; i<TOTAL_PIXELS; i++) {
    if (assembledScreen[i] == 1) {  
      uint pixel_to_light = 0;
      uint row = i / FRAME_WIDTH;

      if (row % 2 == 1) {
        pixel_to_light = i;
      } else {
        pixel_to_light = FRAME_WIDTH*(row+1) - (i - (row*FRAME_WIDTH))-1;
      }

      leds[pixel_to_light] = activeColor; 
    }
  }    
}


void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  esp_wifi_set_channel(3, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erreur ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(onDataReceived);
  Serial.println("ESP-NOW Récepteur prêt !");

  // ** FastLED : Initialisation **
  FastLED.addLeds<WS2812B, BROCHE, GRB>(leds, TOTAL_PIXELS); 
  
  FastLED.clear();              
  FastLED.show();               
  FastLED.setBrightness(20);    

void loop() {
  if (screenFilled) {
    calculate_image();
    FastLED.show();     
    screenFilled = false;
  }
}