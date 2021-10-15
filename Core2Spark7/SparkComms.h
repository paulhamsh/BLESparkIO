#ifndef SparkComms_h
#define SparkComms_h

#ifdef IOS
#include "NimBLEDevice.h"
#else
#include "BluetoothSerial.h"
#include "BLEDevice.h"
#endif

#include "RingBuffer.h"

#define HW_BAUD 1000000
#define BLE_BUFSIZE 5000

#define C_SERVICE "ffc0"
#define C_CHAR1   "ffc1"
#define C_CHAR2   "ffc2"

#define S_SERVICE "ffc0"
#define S_CHAR1   "ffc1"
#define S_CHAR2   "ffc2"

#define PEDAL_SERVICE    "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define PEDAL_CHAR       "7772e5db-3868-4112-a1a9-f2669d106bf3"

#define  SPARK_BT_NAME  "Spark 40 Audio"

void connect_to_all();
bool sp_available();
bool app_available();
uint8_t sp_read();
uint8_t app_read();
void sp_write(byte *buf, int len);
void app_write(byte *buf, int len);
int ble_getRSSI();

#ifndef IOS
BluetoothSerial *bt;
#endif

bool is_ble;
boolean isBTConnected;  

#ifdef IOS
NimBLEServer *pServer;
NimBLEService *pService;
NimBLECharacteristic *pCharacteristic_receive;
NimBLECharacteristic *pCharacteristic_send;
NimBLEAdvertising *pAdvertising;
  
NimBLEScan *pScan;
NimBLEScanResults pResults;
NimBLEAdvertisedDevice device;

NimBLEClient *pClient_sp;
NimBLERemoteService *pService_sp;
NimBLERemoteCharacteristic *pReceiver_sp;
NimBLERemoteCharacteristic *pSender_sp;

NimBLEClient *pClient_pedal;
NimBLERemoteService *pService_pedal;
NimBLERemoteCharacteristic *pReceiver_pedal;
NimBLERemoteCharacteristic *pSender_pedal;
#else
  
BLEScan *pScan;
BLEScanResults pResults;
BLEAdvertisedDevice device;

BLEClient *pClient_sp;
BLERemoteService *pService_sp;
BLERemoteCharacteristic *pReceiver_sp;
BLERemoteCharacteristic *pSender_sp;

BLEClient *pClient_pedal;
BLERemoteService *pService_pedal;
BLERemoteCharacteristic *pReceiver_pedal;
BLERemoteCharacteristic *pSender_pedal;
#endif

RingBuffer ble_in;
RingBuffer ble_app_in;

#endif
