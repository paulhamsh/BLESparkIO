#ifndef SparkComms_h
#define SparkComms_h

#include "NimBLEDevice.h"
#include "BluetoothSerial.h"
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


// Bluetooth vars
#define  SPARK_NAME  "Spark 40 Audio"
#define  MY_NAME     "Heltec"

void start_ser();
void start_bt(bool isBLE);
void connect_to_all(bool isBLE);

bool ser_available();
bool bt_available();

uint8_t ser_read();
uint8_t bt_read();

void ser_write(byte *buf, int len);
void bt_write(byte *buf, int len);

int ble_getRSSI();

// bluetooth communications

BluetoothSerial *bt;
HardwareSerial *ser;
bool is_ble = true;

boolean isBTConnected;  

/*
// BLE 
NimBLEAdvertisedDevice device;
NimBLEClient *pClient;
NimBLERemoteService *pService;
NimBLERemoteCharacteristic *pSender;
NimBLERemoteCharacteristic *pReceiver;
*/



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


RingBuffer ble_in;
RingBuffer ble_app_in;

// void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);

#endif
