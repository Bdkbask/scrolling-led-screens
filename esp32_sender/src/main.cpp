#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <HTTPClient.h>
#include <vector>
#include <string>
#include <algorithm>
#include <FastLED.h>

String server_ip = SERVER_IP;


String charsServer = "http://"+ server_ip + ":5000/char";
String textServer = "http://"+ server_ip + ":5000/text";
String setupTextServer = "http://"+ server_ip + ":5000/firstText";
String colorServer = "http://"+ server_ip + ":5000/color";
String setupColorServer = "http://"+ server_ip + ":5000/firstColor";



#define BROCHE        26 
#define FRAME_WIDTH  30
#define FRAME_HEIGHT  30
#define NB_SCREEN 3
#define TOTAL_PIXELS FRAME_WIDTH*FRAME_HEIGHT 

CRGB leds[TOTAL_PIXELS];


const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

uint8_t new_mac[] = {0x6C, 0x8D, 0xC1, 0x01, 0x5A, 0x32};

volatile bool transmitComplete = false;
volatile esp_now_send_status_t lastStatus = ESP_NOW_SEND_SUCCESS;





String textToDisplay;
uint8_t colorToDisplay[3] = {255, 0, 255}; // Default color is purple

std::vector<std::vector<byte>> textPixelArray;

typedef struct struct_message {
  uint8_t index;
  uint8_t nbRows;
  uint8_t color[3];
  byte pixels[8*30];
} struct_message;




const uint8_t broadcastAddress[][6] = {
  {0x28, 0x05, 0xA5, 0x65, 0x50, 0x44}, 
  {0x28, 0x05, 0xA5, 0x65, 0x6E, 0x3C}
};

void getColorToDisplayFromServer(bool firstTime = false) {
  String errorReturn = "Couldn't get data from server";

  if(WiFi.status()== WL_CONNECTED){
    Serial.println("WiFi connected");
    HTTPClient http;
    
    if (firstTime) {
      http.begin(setupColorServer.c_str());
    } else {
      http.begin(colorServer.c_str());
    }

    int httpResponseCode = http.GET();

    
    while (httpResponseCode!=200 && httpResponseCode!=204) {
      Serial.print("HTTP Response code color: ");
      Serial.println(httpResponseCode);
      delay(500);
      httpResponseCode = http.GET();
    }

    if (httpResponseCode==200) {
      Serial.print("HTTP Response code color: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      http.end();

      Serial.println("Color payload: " + payload);

      // Parse the payload to extract color values
      int commaIndex1 = payload.indexOf(',');
      int commaIndex2 = payload.indexOf(',', commaIndex1 + 1);

      colorToDisplay[0] = payload.substring(0, commaIndex1).toInt();
      colorToDisplay[1] = payload.substring(commaIndex1 + 1, commaIndex2).toInt();
      colorToDisplay[2] = payload.substring(commaIndex2 + 1).toInt();
      
      return;
    }
    else {
      Serial.print("Error code color: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }

  return;
}

String getTextToDisplayFromServer(bool firstTime = false) {
  String errorReturn = "Couldn't get data from server";

  if(WiFi.status()== WL_CONNECTED){
    Serial.println("WiFi connected text");
    HTTPClient http;
    
    if (firstTime) {
      http.begin(setupTextServer.c_str());
    } else {
      http.begin(textServer.c_str());
    }

    int httpResponseCode = http.GET();

    
    while (httpResponseCode!=200 && httpResponseCode!=204) {
      Serial.print("HTTP Response code text: ");
      Serial.println(httpResponseCode);
      delay(500);
      httpResponseCode = http.GET();
    }

    if (httpResponseCode==204) {
      Serial.println("No new text to display");
      http.end();
      return "";
    }
    if (httpResponseCode==200) {
      Serial.print("HTTP Response code text: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      http.end();

      return payload;
    }
    else {
      Serial.print("Error code text: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    return errorReturn;
    http.end();
  }

  return errorReturn;
}




String getPixelsForChar(char charToget) {
  // Serial.println("Getting pixel map for char: " + String(charToget));

  String errorReturn = "Couldn't get data from server";

  if(WiFi.status()== WL_CONNECTED){
    Serial.println("WiFi connected");
    HTTPClient http;
    String serverPath;
    if (charToget == ' ') {
      serverPath = charsServer + "/SPACE";
    } else {
      serverPath = charsServer + "/" + String(charToget);
    }    
    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());
    
    // If you need Node-RED/server authentication, insert user and password below
    //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
    
    // Send HTTP GET request
    int httpResponseCode = http.GET();

    
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code char: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      http.end();

      return payload;
    }
    else {
      Serial.print("Error code char: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    return errorReturn;
    http.end();
  }

  return errorReturn;
}


std::vector<String> getTextToWrite(String texte) {
  Serial.println("Getting pixel map for text: " + texte);
  std::vector<String> pixelMap;
  for (uint i = 0; i < texte.length(); i++) {
    char currentChar = texte[i];
    
    // Serial.println(currentChar);
    String tmpCharPixelMap = getPixelsForChar(currentChar);

    if (!tmpCharPixelMap.startsWith("Couldn't")) {
      pixelMap.push_back(tmpCharPixelMap);
    } 
  }

  return pixelMap;
}


std::vector<std::vector<byte>> fillToFitScreen(std::vector<std::vector<byte>> textPixelArray) {
  uint currentHeight = textPixelArray.size();
  uint currentWidth = textPixelArray[0].size();

  while (currentHeight < FRAME_HEIGHT) {
    std::vector<byte> emptyLine(currentWidth, 0);
    textPixelArray.push_back(emptyLine);
    currentHeight++;
  }

  while (currentWidth < FRAME_WIDTH * NB_SCREEN) {
    for (uint i = 0; i < textPixelArray.size(); i++) {
      textPixelArray[i].push_back(0);
    }
    currentWidth++;
  }

  return textPixelArray;
}

std::vector<std::vector<byte>> getByteMapForTextVector(std::vector<String> textCharVector) {
  uint textWidth = 0;
  uint textHeight = 0;


  std::vector<std::vector<byte>> finalByteMapText;
  for (uint i = 0; i < textCharVector.size(); i++){
    textWidth +=  textCharVector[i].toInt();
    if (textCharVector[i].substring(3,5).toInt() > textHeight) textHeight = textCharVector[i].substring(3,5).toInt();
  }

  for (uint currentHeightIndex = 0; currentHeightIndex < textHeight; currentHeightIndex++){
    std::vector<byte> currentLine;
    for (uint currentLetterIndex = 0; currentLetterIndex < textCharVector.size();currentLetterIndex++){
      uint currentLetterWidth = textCharVector[currentLetterIndex].toInt();
      uint newLineIndex = textCharVector[currentLetterIndex].indexOf("\n")+1;
      for (uint i = 0; i < currentLetterWidth;i++) {
        uint currentCharIndex = currentHeightIndex * currentLetterWidth + i + newLineIndex;
        if (currentCharIndex < textCharVector[currentLetterIndex].length()) {
          char pixelChar = textCharVector[currentLetterIndex][currentCharIndex];
          currentLine.push_back(pixelChar - '0');
        }
      }
    }
    finalByteMapText.push_back(currentLine);
  }

  finalByteMapText = fillToFitScreen(finalByteMapText);



  return finalByteMapText;

}

std::vector<std::vector<std::vector<byte>>> calculatePixelsPerScreen(std::vector<std::vector<byte>> textPixelArray) {
  std::vector<std::vector<std::vector<byte>>> byteMapPerScreen(NB_SCREEN);  // Pre-allocate NB_SCREEN rows
  
  for (uint currentScreen = 0; currentScreen < NB_SCREEN; currentScreen++) {
    byteMapPerScreen[currentScreen].reserve(FRAME_HEIGHT);  // Reserve space for efficiency
  }
  
  for (uint currentHeight = 0; currentHeight < FRAME_HEIGHT; currentHeight++) {
    for (uint currentScreen = 0; currentScreen < NB_SCREEN; currentScreen++) {
      uint startIndex = currentScreen * FRAME_WIDTH;
      uint endIndex = (currentScreen+1) * FRAME_WIDTH;

      // Bounds check to prevent heap issues
      if (currentHeight < textPixelArray.size() && endIndex <= textPixelArray[currentHeight].size()) {
        std::vector<byte> subvector(
          textPixelArray[currentHeight].begin() + startIndex,
          textPixelArray[currentHeight].begin() + endIndex
        );
        byteMapPerScreen[currentScreen].push_back(subvector);
      }
    }
  }


  return byteMapPerScreen;
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  lastStatus = status;
  transmitComplete = true; 
}

bool sendChunkAndWait(const uint8_t *peer_addr, const uint8_t *data, size_t len) {
    transmitComplete = false; 
    lastStatus = ESP_NOW_SEND_SUCCESS; 

    esp_err_t result = esp_now_send(peer_addr, data, len);

    if (result != ESP_OK) {
        Serial.println("Error: Failed to queue packet ESP.");
        return false;
    }

    unsigned long startTime = millis();

    while (!transmitComplete && (millis() - startTime < 15)) { 
        yield(); 
    }
    
    if (transmitComplete && lastStatus == ESP_NOW_SEND_SUCCESS) {
        return true; // Chunk confirmed received by peer
    } else {
        return false; // Failed or timeout
    }
}


void retreiveNewTextFromServer() {
  String newText = getTextToDisplayFromServer(false);
  if (newText != "" && newText != textToDisplay) {
    textToDisplay = newText;
    textPixelArray = getByteMapForTextVector(getTextToWrite(textToDisplay));
  }
}



void retrieveNewColorFromServer() {
  getColorToDisplayFromServer(false);
}


void httpTask(void * pvParameters) {
  for (;;) { 
    retreiveNewTextFromServer(); 
    vTaskDelay(pdMS_TO_TICKS(2500));
    retrieveNewColorFromServer();
    vTaskDelay(pdMS_TO_TICKS(2500)); 
  }
}

//===============================================================================================

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  delay(1000);

  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  textToDisplay = getTextToDisplayFromServer(true);

  textPixelArray = getByteMapForTextVector(getTextToWrite(textToDisplay));

  Serial.println("Setting channel...");
  esp_wifi_set_channel(3, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    return;
  }

  if (esp_now_init() == ESP_OK) {
    esp_now_register_send_cb(OnDataSent); // Register your confirmation function
    Serial.println("Initialzed callback...");
  }

  esp_now_peer_info_t peerInfo = {};
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  memcpy(peerInfo.peer_addr, broadcastAddress[0], 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Erreur ajout du pair 1");
    return;
  }
  memcpy(peerInfo.peer_addr, broadcastAddress[1], 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Erreur ajout du pair 2");
    return;
  }

  xTaskCreatePinnedToCore(
      httpTask,          // Fonction à exécuter
      "HTTP Task",       // Nom de la tâche
      10000,             // Taille de la pile (en octets, pour HTTP c'est beaucoup)
      NULL,              // Paramètres (aucun)
      1,                 // Priorité
      NULL,              // Handle de la tâche (non nécessaire ici)
      0);                // Numéro de Core (0)

  
    FastLED.addLeds<WS2812B, BROCHE, GRB>(leds, TOTAL_PIXELS); 

    FastLED.setBrightness(20); 

    FastLED.show();
}


void sendData(int screenIndex, std::vector<std::vector<byte>> dataToSend) {
  uint part = 0;
  uint nbRowSent = 0;
  struct_message toSend;
  toSend.color[0] = colorToDisplay[0];
  toSend.color[1] = colorToDisplay[1];
  toSend.color[2] = colorToDisplay[2];
  Serial.println("Color to send : R=" + String(toSend.color[0]) + " G=" + String(toSend.color[1]) + " B=" + String(toSend.color[2]));
  std::vector<uint8_t> byteArray;
  for (const auto& row : dataToSend) {
    for (const auto& pixel : row) {
      byteArray.push_back(pixel);
    }
    nbRowSent++;
    if (nbRowSent == 8) {
      for (const auto& val : byteArray) {
        toSend.pixels[&val - &byteArray[0]] = val;
      }
      toSend.index = part;
      part++;
      toSend.nbRows = nbRowSent;
      while (!sendChunkAndWait(broadcastAddress[screenIndex-1], (uint8_t *) &toSend, sizeof(toSend))) {

        delay(10);
      }
      nbRowSent = 0;
      byteArray.clear();
    }
  }

  for (const auto& val : byteArray) {
    toSend.pixels[&val - &byteArray[0]] = val;
  }
  toSend.index = part;
  part++;
  toSend.nbRows = nbRowSent;
  while (!sendChunkAndWait(broadcastAddress[screenIndex-1], (uint8_t *) &toSend, sizeof(toSend))) {
    // Serial.println("Retrying...");
    delay(10);
  }

  nbRowSent = 0;
  byteArray.clear();
}

void slideRight() {
  for (uint i = 0; i < textPixelArray.size(); i++) {
    std::rotate(textPixelArray[i].rbegin(), textPixelArray[i].rbegin() + 1, textPixelArray[i].rend());
  }
}

void slideLeft() {
  for (uint i = 0; i < textPixelArray.size(); i++) {
    std::rotate(textPixelArray[i].begin(), textPixelArray[i].begin() + 1, textPixelArray[i].end());
  }
}

void sendOkToEveryone(){
  const char okMessage[] = "OK";
  for (int i = 1; i < 2; i++) {
    while (!sendChunkAndWait(broadcastAddress[i], (uint8_t *) okMessage, sizeof(okMessage))) {
      Serial.println("Retrying OK...");
      delay(5);
    }
  }
}


void calculate_image(const std::vector<std::vector<byte>>& dataToDisplay) {
    
    const CRGB activeColor(colorToDisplay[0], colorToDisplay[1], colorToDisplay[2]);
    const CRGB offColor(0, 0, 0);

    for (int y = 0; y < FRAME_HEIGHT; y++) {
        for (int x = 0; x < FRAME_WIDTH; x++) {
            int i; 

            if (y % 2 == 0) {
                i = y * FRAME_WIDTH + (FRAME_WIDTH - 1 - x);
            } else {
                i = y * FRAME_WIDTH + x;
            }

            leds[i] = dataToDisplay[y][x] ? activeColor : offColor;
        }
    }

    FastLED.show(); 
}



void loop() {
  //calculate millis for each action in loop 
  // long startMillis = millis();
  std::vector<std::vector<std::vector<byte>>> dataToSend = calculatePixelsPerScreen(textPixelArray);
  // long currentMillis1 = millis();
  // Serial.println("Sending data...");
  sendData(1, dataToSend[1]);
  // long currentMillis2 = millis();
  sendData(2, dataToSend[2]);
  // long currentMillis3 = millis();
  // sendOkToEveryone();        // to be developed on receiver side to adjust timing
  calculate_image(dataToSend[0]);
  // long currentMillis4 = millis();

  slideLeft();
  // long endMillis = millis();
  // Serial.println("Time taken (ms):");
  // Serial.println(" - Calculate screens : " + String(currentMillis1 - startMillis));
  // Serial.println(" - Send screen 1     : " + String(currentMillis2 - currentMillis1));
  // Serial.println(" - Send screen 2     : " + String(currentMillis3 - currentMillis2));
  // Serial.println(" - Display screen 0  : " + String(currentMillis4 - currentMillis3));
  // Serial.println(" - Slide Left        : " + String(endMillis - currentMillis4));

}
