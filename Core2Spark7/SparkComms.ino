#include "Spark.h"
#include "SparkComms.h"

#ifdef IOS
#else
#include "BluetoothSerial.h"
#endif

#ifdef IOS
void notifyCB_sp(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
#else
void notifyCB_sp(BLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
#endif

  int i;
  byte b;

  DEBUG("Spark sent info");
  for (i = 0; i < length; i++) {
    b = pData[i];
    ble_in.add(b);
  }
  ble_in.commit();
}

#ifdef IOS
void notifyCB_pedal(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
#else
void notifyCB_pedal(BLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
#endif

  int i;
  
  DEBUG("Pedal sent info");

  // In mode B the BB gives 0x80 0x80 0x90 0xNN 0x64 or 0x80 0x80 0x80 0xNN 0x00 for on and off
  // In mode C the BB gives 0x80 0x80 0xB0 0xNN 0x7F or 0x80 0x80 0xB0 0xNN 0x00 for on and off


  if (pData[2] == 0x90 || (pData[2] == 0xB0 && pData[4] == 0x7F)) {
    switch (pData[3]) {
      case 0x3C:
      case 0x14:
        curr_preset = 0;
        break;
      case 0x3E:
      case 0x15:
        curr_preset = 1;
        break;
      case 0x40:
      case 0x16:
        curr_preset = 2;
        break;
      case 0x41:
      case 0x17:
        curr_preset = 3;
        break;
    }
  triggered_pedal = true;
  }
}

#ifdef IOS
class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
  void onRead(NimBLECharacteristic* pCharacteristic){
  };

  void onWrite(NimBLECharacteristic* pCharacteristic) {
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onWrite(), value: ");
      
        int j, l;
        const char *p;
        byte b;
        l = pCharacteristic->getValue().length();
        p = pCharacteristic->getValue().c_str();
        for (j=0; j < l; j++) {
          b = p[j];
          ble_app_in.add(b);
          Serial.print(" ");
          Serial.print(b, HEX); 
        }
        Serial.println();
        ble_app_in.commit();
  };
/*   

  void onNotify(NimBLECharacteristic* pCharacteristic) {
  };

  void onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code) {
  };

  void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) {
  };
*/
};

static CharacteristicCallbacks chrCallbacks_s, chrCallbacks_r;
#endif



bool connected_pedal, connected_sp;
bool bt_connected;

void connect_to_all() {
  int i;
  uint8_t b;

#ifdef IOS
  NimBLEDevice::init("Spark 40 BLE");
  pClient_sp =    NimBLEDevice::createClient();
  pClient_pedal = NimBLEDevice::createClient();
  pScan =         NimBLEDevice::getScan();
  
  pServer =       NimBLEDevice::createServer();
  pService =      pServer->createService(S_SERVICE);
  pCharacteristic_receive = pService->createCharacteristic(S_CHAR1, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  pCharacteristic_send =    pService->createCharacteristic(S_CHAR2, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);  
  pCharacteristic_receive->setCallbacks(&chrCallbacks_r);
  pCharacteristic_send->setCallbacks(&chrCallbacks_s);

  pService->start();
  pServer->start();

  pAdvertising = NimBLEDevice::getAdvertising(); // create advertising instance
  pAdvertising->addServiceUUID(pService->getUUID()); // tell advertising the UUID of our service
  pAdvertising->setScanResponse(true);  

#else
  BLEDevice::init("Spark 40 BLE");
  pClient_sp =    BLEDevice::createClient();
  pClient_pedal = BLEDevice::createClient();
  pScan  =        BLEDevice::getScan();
#endif

  DEBUG("Service set up");
  
  // Connect to Spark
  connected_sp = false;
  connected_pedal = false;


#ifdef IOS
  NimBLEUUID SpServiceUuid(C_SERVICE);
  NimBLEUUID PedalServiceUuid(PEDAL_SERVICE);
#else
  BLEUUID SpServiceUuid(C_SERVICE);
  BLEUUID PedalServiceUuid(PEDAL_SERVICE);
#endif

  
  while (!connected_sp /* || !connected_pedal*/ ) {
    pResults = pScan->start(4);
    for(i = 0; i < pResults.getCount()  && (!connected_sp /* || !connected_pedal */); i++) {
      device = pResults.getDevice(i);

      if (device.isAdvertisingService(SpServiceUuid)) {
        DEBUG("Found Spark - trying to connect....");
        if(pClient_sp->connect(&device)) {
          connected_sp = true;
          DEBUG("Spark connected");
        }
      }

      if (strcmp(device.getName().c_str(),"iRig BlueBoard") == 0) {
        DEBUG("Found pedal by name - trying to connect....");

        if(pClient_pedal->connect(&device)) {
          connected_pedal = true;
          DEBUG("Pedal connected");
        }
      }
    }

    // Set up client
    if (connected_sp) {
      pService_sp = pClient_sp->getService(SpServiceUuid);
      if (pService_sp != nullptr) {
        pSender_sp   = pService_sp->getCharacteristic(C_CHAR1);
        pReceiver_sp = pService_sp->getCharacteristic(C_CHAR2);
        if (pReceiver_sp && pReceiver_sp->canNotify()) {
#ifdef IOS
          if (!pReceiver_sp->subscribe(true, notifyCB_sp, true)) {
            connected_sp = false;
          }
#else
          pReceiver_sp->registerForNotify(notifyCB_sp);
#endif
        }
      }
    }

    if (connected_pedal) {
      pService_pedal = pClient_pedal->getService(PedalServiceUuid);
      if (pService_pedal != nullptr) {
        pReceiver_pedal = pService_pedal->getCharacteristic(PEDAL_CHAR);
        if (pReceiver_pedal && pReceiver_pedal->canNotify()) {
#ifdef IOS
          if (!pReceiver_pedal->subscribe(true, notifyCB_pedal, true)) {
            connected_pedal = false;
          }
#else
          pReceiver_pedal->registerForNotify(notifyCB_pedal);
#endif
        }
      }
    }
  }



#ifdef IOS
  DEBUG("Available for app to connect...");
  pAdvertising->start(); 
#else
  const uint8_t notifyOn[] = {0x1, 0x0};
  
  BLERemoteDescriptor* p2902 = pReceiver_sp->getDescriptor(BLEUUID((uint16_t)0x2902));
  if(p2902 != nullptr)
  {
    p2902->writeValue((uint8_t*)notifyOn, 2, true);
  }

  
  Serial.println("Starting classic bluetooth");
  // now advertise Serial Bluetooth
  bt = new BluetoothSerial();
  if (!bt->begin (SPARK_BT_NAME)) {
    DEBUG("Bluetooth init fail");
    while (true);
  }

  // flush anything read from Spark - just in case
  while (bt->available())
    b = bt->read(); 

  DEBUG("Spark 40 Audio set up");
#endif
}



bool app_available() {
#ifdef IOS
  return !ble_app_in.is_empty();
#else
  return bt->available();
#endif
}

uint8_t app_read() {
#ifdef IOS
  uint8_t b;
  ble_app_in.get(&b);
  return b;
#else
  return bt->read();
#endif
}

void app_write(byte *buf, int len) {
#ifdef IOS
  pCharacteristic_send->setValue(buf, len);
  pCharacteristic_send->notify(true);
#else  
  bt->write(buf, len);
#endif
}

bool sp_available() {
  return !ble_in.is_empty();
}

uint8_t sp_read() {
  uint8_t b;
  ble_in.get(&b);
  return b;
}

void sp_write(byte *buf, int len) {
  pSender_sp->writeValue(buf, len, false);
}

int ble_getRSSI() {
  return pClient_sp->getRssi();
}
