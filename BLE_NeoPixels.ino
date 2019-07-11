#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_NeoPixel.h>

// NEOPIXEL SETUP---------------------------------------------------------------------------------
#define PIN            23
#define NUMPIXELS      250
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
//---------------------------------------------------------------------------------------------------------
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t txValue = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
//--------------------------------------------------------------------------------------------------------------------

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};
//.........................................................................................................................
unsigned int hexToDec(char hexString) {
  unsigned int decValue = 0;
  int nextInt;
  
  for (int i = 0; i < 1; i++) {
    
    nextInt = int(hexString);
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);
    
    decValue = (decValue * 16) + nextInt;
  }
  
  return decValue;
}
//-------------------------------------------------------------------------------------------------------
  char data_buf[15]={0};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received data: ");
        for (int i = 0; i < rxValue.length(); i++){
          Serial.print(rxValue[i]);
          data_buf[i]=char(rxValue[i]);
        }
 
  unsigned int dec=0, dec1=0;
  unsigned int num=0;
  unsigned int color[3]={0};

if(data_buf[0]=='#' && data_buf[7]=='$')
{
for(int i=1; i<=6;i++)
  {
    dec1=hexToDec(data_buf[i]);
    dec1=dec1*16;
    i++;
    dec=dec1 + hexToDec(data_buf[i]);
    int k=(i-1)/2;
    color[k]=dec;
    }

if(data_buf[9]==';')
{num=hexToDec(data_buf[8]);}

else if (data_buf[10]==';')
{dec1=10*hexToDec(data_buf[8]);
num=dec1+hexToDec(data_buf[9]);}

else if(data_buf[11]==';')
{dec1=100*hexToDec(data_buf[8]);
dec1=dec1 + (10*hexToDec(data_buf[9]));
num=dec1+hexToDec(data_buf[10]);
}
}
      pixels.setPixelColor(num, pixels.Color(color[0],color[1],color[2]));
      pixels.show();
      Serial.print("red= "); Serial.println(color[0]);
      Serial.print("green= "); Serial.println(color[1]);
      Serial.print("blue= ");Serial.println(color[2]);
      Serial.print("led number= ");Serial.println(num);
      Serial.println("------------------------------------------------------------------");
      //delay(500);
      }
    } 
    };
    
//---------------------------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  pixels.begin();
  delay(1000);

  // Create the BLE Device
  BLEDevice::init("server");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());
  

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}
//-----------------------------------------------------------------------------------------------------------------

void loop() {
  if (deviceConnected) {
    pCharacteristic->setValue(&txValue, 1);
    pCharacteristic->notify();
  }
  delay(1000);
}

