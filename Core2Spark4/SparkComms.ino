#include "Spark.h"
#include "SparkComms.h"
#include "BluetoothSerial.h"



// NEW CODE ------------------------------------------



void notifyCB_sp(BLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
  int i;
  byte b;

  Serial.print("Spark: ");
  for (i = 0; i < length; i++) {
    b = pData[i];
    Serial.print(b, HEX);
    Serial.print(" ");
    ble_in.add(b);
  }
  Serial.println();
  ble_in.commit();

  // triggered_to_app = true;

}


void notifyCB_pedal(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
  int i;
  
  Serial.print("Pedal: ");
  for (i = 0; i < length; i++) {
    Serial.print(pData[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // In mode B the BB gives 0x80 0x80 0x90 0xNN 0x64 or 0x80 0x80 0x80 0xNN 0x00 for on and off
  // In mode C the BB gives 0x80 0x80 0xB0 0xNN 0x7F or 0x80 0x80 0xB0 0xNN 0x00 for on and off

  /*
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

    // triggered_pedal = true;
  }
  */
}

/** Handler class for characteristic actions */
class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic){
        int j, l;
        const char *p;
        byte b;
        
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onRead, value: ");

        l = pCharacteristic->getValue().length();
        p = pCharacteristic->getValue().c_str();
        for (j=0; j < l; j++) {
          b = p[j];
          Serial.print(" ");
          Serial.print(b, HEX); 
        };
        Serial.println();
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
//          pass_amp[j] = b;
        }
        Serial.println();
        ble_app_in.commit();
        
//        pass_size_amp = j;
        // triggered_to_amp = true;
    };
   
    
    /** Called before notification or indication is sent, 
     *  the value can be changed here before sending if desired.
     */
    void onNotify(NimBLECharacteristic* pCharacteristic) {
        Serial.println("Sending notification to clients");
    };


    /** The status returned in status is defined in NimBLECharacteristic.h.
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code) {
        String str = ("Notification/Indication status code: ");
        str += status;
        str += ", return code: ";
        str += code;
        str += ", "; 
        str += NimBLEUtils::returnCodeToString(code);
        Serial.println(str);
    };

    void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) {
        String str = "Client ID: ";
        str += desc->conn_handle;
        str += " Address: ";
        str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
        if(subValue == 0) {
            str += " Unsubscribed to ";
        }else if(subValue == 1) {
            str += " Subscribed to notfications for ";
        } else if(subValue == 2) {
            str += " Subscribed to indications for ";
        } else if(subValue == 3) {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID()).c_str();

        if (subValue == 1 && strcmp(std::string(pCharacteristic->getUUID()).c_str(), S_CHAR2) == 0) {
          M5.Lcd.println("App active");
          Serial.println("App active");
        }

        Serial.println(str);
    };
};

static CharacteristicCallbacks chrCallbacks_s, chrCallbacks_r;

bool connected_pedal, connected_sp;


void connect_to_all(bool isBLE) {
  int i;
  is_ble = isBLE;
  
 // Create server to act as Spark
  NimBLEDevice::init("Spark 40 BLE");
  pClient_sp = NimBLEDevice::createClient();
  pScan      = NimBLEDevice::getScan();

  if (is_ble) {  
    // Set up server
    pServer = NimBLEDevice::createServer();
    pService = pServer->createService(S_SERVICE);
    pCharacteristic_receive = pService->createCharacteristic(S_CHAR1, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
    pCharacteristic_send = pService->createCharacteristic(S_CHAR2, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);  
    pCharacteristic_receive->setCallbacks(&chrCallbacks_r);
    pCharacteristic_send->setCallbacks(&chrCallbacks_s);

    pService->start();
    pServer->start();

    pAdvertising = NimBLEDevice::getAdvertising(); // create advertising instance
    pAdvertising->addServiceUUID(pService->getUUID()); // tell advertising the UUID of our service
    pAdvertising->setScanResponse(true);  

    Serial.println("Service set up");
  }
  
  // Connect to Spark
  connected_sp = false;
  connected_pedal = false;
  
  while (!connected_sp /* || !connected_pedal*/ ) {
    pResults = pScan->start(4);
    NimBLEUUID SpServiceUuid(C_SERVICE);
    NimBLEUUID PedalServiceUuid(PEDAL_SERVICE);

    Serial.println("------------------------------");
    for(i = 0; i < pResults.getCount()  && (!connected_sp /* || !connected_pedal */); i++) {
      device = pResults.getDevice(i);

      // Print info
      Serial.print("Name ");
      Serial.println(device.getName().c_str());

      if (device.isAdvertisingService(SpServiceUuid)) {
        Serial.println("Found Spark - trying to connect....");
        if(pClient_sp->connect(&device)) {
          connected_sp = true;
          Serial.println("Spark connected");
          M5.Lcd.println("Spark connected");
        }
      }

      if (strcmp(device.getName().c_str(),"iRig BlueBoard") == 0) {
        Serial.println("Found pedal by name - trying to connect....");
        pClient_pedal = NimBLEDevice::createClient();
        if(pClient_pedal->connect(&device)) {
          connected_pedal = true;
          Serial.println("Pedal connected");
          M5.Lcd.println("Pedal connected");
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
          if (!pReceiver_sp->subscribe(true, notifyCB_sp, true)) {
            connected_sp = false;
            Serial.println("Spark disconnected");
            //pClient_sp->disconnect();
            NimBLEDevice::deleteClient(pClient_sp);
          }
        }
      }
    }

    if (connected_pedal) {
      pService_pedal = pClient_pedal->getService(PedalServiceUuid);
      if (pService_pedal != nullptr) {
        pReceiver_pedal = pService_pedal->getCharacteristic(PEDAL_CHAR);
        if (pReceiver_pedal && pReceiver_pedal->canNotify()) {
          if (!pReceiver_pedal->subscribe(true, notifyCB_pedal, true)) {
            connected_pedal = false;
            pClient_pedal->disconnect();
          }
        }
      }
    }

  }

  Serial.println("Available for app to connect...");
  
  // start advertising
  if (is_ble) {
    pAdvertising->start(); 
  }

}

// NEW CODE ENDS ------------------------------------------

/*

// Callbacks for client events
class ClientCallbacks : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient* pClient) {
    isBTConnected = true;
  }

  void onDisconnect(NimBLEClient* pClient) {
    isBTConnected = false;
  }
};

static ClientCallbacks clientCB;


void start_bt(bool isBLE) {
    bt = new BluetoothSerial();
  
    if (!bt->begin (MY_NAME, true)) {
      DEBUG("Bluetooth init fail");
      while (true);
}

void connect_to_spark() {
    while (!connected) {
      connected = bt->connect(SPARK_NAME);
      if (!(connected && bt->hasClient())) {
        connected = false;
        DEBUG("Not connected");
        delay(2000);
      }
    }

    // flush anything read from Spark - just in case
    while (bt->available())
      b = bt->read(); 
  }

*/

bool app_available() {
  // return ser->available();
  return !ble_app_in.is_empty();
}

bool sp_available() {
  if (!is_ble) {
    return bt->available();
  }
  else {
    return !ble_in.is_empty();
  }
}

uint8_t app_read() {
  //return ser->read();
  uint8_t b;
  ble_app_in.get(&b);
  return b;
}

uint8_t sp_read() {
  if (!is_ble) {
    return bt->read();
  }
  else {
    uint8_t b;
    ble_in.get(&b);
    return b;
  }
}

void app_write(byte *buf, int len) {
//  ser->write(buf, len);
  pCharacteristic_send->setValue(buf, len);
  pCharacteristic_send->notify(true);
}

void sp_write(byte *buf, int len) {
  if (!is_ble) 
    bt->write(buf, len);
  else 
    pSender_sp->writeValue(buf, len, false);
}

int ble_getRSSI() {
  return pClient_sp->getRssi();
}
