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


#include <Adafruit_NeoPixel.h>

#define BROCHE        26 
#define FRAME_WIDTH  30
#define FRAME_HEIGHT  30
#define NB_SCREEN 3
#define TOTAL_PIXELS FRAME_WIDTH*FRAME_HEIGHT 

Adafruit_NeoPixel pixels(TOTAL_PIXELS, BROCHE, NEO_GRB + NEO_KHZ800);
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
  // for (uint i = 0; i < pixelMap.size(); i++) {
  //   Serial.println(pixelMap[i]);
  // }
  return pixelMap;
}


std::vector<byte> revertLine(std::vector<byte> lineInput) {
  return lineInput;
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
    // Serial.println(textCharVector[i].substring(3,5).toInt());
    // Serial.println(textCharVector[i].toInt());

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
    if (!currentHeightIndex%2) {
      currentLine = revertLine(currentLine); // NON IMPLEMENTE ENCORE A VOIR SI UTILE
    }
    finalByteMapText.push_back(currentLine);
  }

  finalByteMapText = fillToFitScreen(finalByteMapText);
  // for(int i = 0; i < finalByteMapText.size(); i++) {
  //   for(int j = 0; j < finalByteMapText[0].size(); j++) {
  //     Serial.print(finalByteMapText[i][j]);
  //   }
  //   Serial.println("");
  // }


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

  // for (uint currentScreen = 0; currentScreen < NB_SCREEN; currentScreen++) {
  //   Serial.println("Screen " + String(currentScreen) + " :");
  //   for (uint h = 0; h < byteMapPerScreen[currentScreen].size(); h++) {
  //     for (uint w = 0; w < byteMapPerScreen[currentScreen][h].size(); w++) {
  //       Serial.print(byteMapPerScreen[currentScreen][h][w]);
  //     }
  //     Serial.println("");
  //   }
  // }

  return byteMapPerScreen;
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  lastStatus = status;
  // Serial.println(lastStatus);
  transmitComplete = true; // Signals the main loop to proceed
  // Serial.println("Callback happened");
}

// Example of sending a single chunk and waiting
bool sendChunkAndWait(const uint8_t *peer_addr, const uint8_t *data, size_t len) {
    transmitComplete = false; // Reset the flag before sending
    lastStatus = ESP_NOW_SEND_SUCCESS; // Reset status

    esp_err_t result = esp_now_send(peer_addr, data, len);

    if (result != ESP_OK) {
        // Failed to even put the packet in the buffer (e.g., buffer full)
        Serial.println("Error: Failed to queue packet ESP.");
        return false;
    }

    // *** WAITING LOOP ***
    // This is where you "wait" for the asynchronous callback.
    // Use a timeout to prevent infinite blocking if the callback never fires.
    unsigned long startTime = millis();
    // Serial.println("Waiting for callback...");
    // Serial.println("Laststatus before wait : " + lastStatus);
    while (!transmitComplete && (millis() - startTime < 15)) { // Wait up to 50ms
        // Do nothing (or handle low priority tasks)
        yield(); 
    }
    
    // Check if the callback fired successfully

    // Serial.printf("Fin attente. Complete: %d, Status: %d\n", transmitComplete, lastStatus);
    
    if (transmitComplete && lastStatus == ESP_NOW_SEND_SUCCESS) {
        return true; // Chunk confirmed received by peer
    } else {
        // Serial.println("Error: Chunk failed or timed out.");
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

  
  // 1. Initialisation de FastLED
    FastLED.addLeds<WS2812B, BROCHE, GRB>(leds, TOTAL_PIXELS); 

    // 2. Définition de la luminosité (fonction standard de FastLED)
    FastLED.setBrightness(20); 

    // 3. Affichage (éteint)
    FastLED.show();
}


void sendData(int screenIndex, std::vector<std::vector<byte>> dataToSend) {
  // Convertir les données en un tableau d'octets
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

        // Serial.println("Retrying...");
        // Serial.println("New index : " + String(toSend.index));
        delay(10);
      }
      nbRowSent = 0;
      byteArray.clear();
    }
  }

  //display the data to send
  // for (size_t i = 0; i < byteArray.size(); i++) {
  //   Serial.print(byteArray[i]);
  // }
  // Serial.println();
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

// void calculate_image(const std::vector<std::vector<byte>>& dataToDisplay) {
//   pixels.fill(pixels.Color(0,0,0));
//   uint8_t assembledScreen[TOTAL_PIXELS];
//   size_t currentOffset = 0; // Index de destination dans assembledScreen

//     // Boucle sur les lignes du double vecteur
//   for (const auto& row : dataToDisplay) {   
//       size_t rowSize = row.size(); 
//       if (currentOffset + rowSize <= TOTAL_PIXELS) {
//           memcpy(assembledScreen + currentOffset,
//                   row.data(),                     
//                   rowSize);                        
          
//           currentOffset += rowSize;
//       } else {
//         break; 
//       }
//   }
//   for(int i=0; i<TOTAL_PIXELS; i++) { // Les pixels sont numÃ©rotÃ©s de 0 Ã  ...
//     if (assembledScreen[i] == 1) {  
//       uint pixel_to_light = 0;
//       uint row = i / FRAME_WIDTH;

//       if (row%2 == 1) {
//         pixel_to_light = i;
//       } else {
//         pixel_to_light = FRAME_WIDTH*(row+1) - (i - (row*FRAME_WIDTH))-1;
//       }

//       pixels.setPixelColor(pixel_to_light, pixels.Color(colorToDisplay[0], colorToDisplay[1], colorToDisplay[2]));
//       //delay(5);
//     }
//   }
//   pixels.show();    
// }


// Le buffer 'leds' est globalement accessible
// CRGB leds[TOTAL_PIXELS];

void calculate_image(const std::vector<std::vector<byte>>& dataToDisplay) {
    
    // Définir les couleurs une fois pour l'efficacité
    const CRGB activeColor(colorToDisplay[0], colorToDisplay[1], colorToDisplay[2]);
    const CRGB offColor(0, 0, 0);

    for (int y = 0; y < FRAME_HEIGHT; y++) {
        for (int x = 0; x < FRAME_WIDTH; x++) {
            int i; // index final dans le buffer

            // 1. Calcul du mapping en serpent (Logique inchangée)
            if (y % 2 == 0) {
                // Rangées paires : de DROITE à GAUCHE
                i = y * FRAME_WIDTH + (FRAME_WIDTH - 1 - x);
            } else {
                // Rangées impaires : de GAUCHE à DROITE
                i = y * FRAME_WIDTH + x;
            }

            // 2. Écriture directe dans le buffer FastLED
            leds[i] = dataToDisplay[y][x] ? activeColor : offColor;
        }
    }

    // 3. Envoi DMA/RMT/I2S (Bloquant, mais rapide sur ESP32)
    FastLED.show(); 
    
    // NOTE: FastLED utilise nativement le RMT ou I2S pour l'ESP32, assurant 
    // des performances maximales et une gestion efficace du buffer.
}



void loop() {
  //calculate millis for each action in loop 
  long startMillis = millis();
  std::vector<std::vector<std::vector<byte>>> dataToSend = calculatePixelsPerScreen(textPixelArray);
  long currentMillis1 = millis();
  Serial.println("Sending data...");
  sendData(1, dataToSend[1]);
  long currentMillis2 = millis();
  sendData(2, dataToSend[2]);
  long currentMillis3 = millis();
  // sendOkToEveryone();
  calculate_image(dataToSend[0]);
  long currentMillis4 = millis();

  slideLeft();
  long endMillis = millis();
  Serial.println("Time taken (ms):");
  Serial.println(" - Calculate screens : " + String(currentMillis1 - startMillis));
  Serial.println(" - Send screen 1     : " + String(currentMillis2 - currentMillis1));
  Serial.println(" - Send screen 2     : " + String(currentMillis3 - currentMillis2));
  Serial.println(" - Display screen 0  : " + String(currentMillis4 - currentMillis3));
  Serial.println(" - Slide Left        : " + String(endMillis - currentMillis4));

}
