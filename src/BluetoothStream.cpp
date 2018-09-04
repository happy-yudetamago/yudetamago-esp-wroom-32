// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "BluetoothStream.h"

#include "log/Log.h"

bool deviceConnected = false;
bool oldDeviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
public:
    MyCallbacks(BluetoothStream* stream) : stream(stream) {
    }

    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();

        for (int i=0; i<rxValue.length(); i++) {
            stream->AppendReadBuffer(rxValue[i]);
        }
    }

private:
    BluetoothStream* stream;
};

BluetoothStream::BluetoothStream() : initialized(false)
{
}

BluetoothStream::~BluetoothStream()
{
}

void BluetoothStream::Initialize()
{
    BLEDevice::init("Yudetamago config");

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pTxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        BLECharacteristic::PROPERTY_READ);

    pTxCharacteristic->addDescriptor(new BLE2902());

    pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE);

    pRxCharacteristic->setCallbacks(new MyCallbacks(this));

    // Start the service
    pService->start();

    // Start advertising
    pServer->getAdvertising()->start();

    initialized = true;
}

boolean BluetoothStream::Update()
{
    if (!initialized) {
        return false;
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        Log::Info("BLE: disconnected.");
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
    return true;
}

void BluetoothStream::AppendReadBuffer(char ch)
{
    readStream.push_back(ch);
}

int BluetoothStream::available()
{
    return readStream.size();
}

int BluetoothStream::read()
{
    if (available() == 0) {
        return -1;
    }
    int ch = readStream[0];
    readStream.erase(readStream.begin());
    return ch;
}

int BluetoothStream::peek()
{
    return -1;
}

void BluetoothStream::flush()
{
}

size_t BluetoothStream::write(uint8_t ch)
{
    if (!initialized) {
        return 0;
    }
    if (!deviceConnected) {
        return 0;
    }
    pTxCharacteristic->setValue(&ch, 1);
    pTxCharacteristic->notify();
    vTaskDelay(10);
    return 1;
}
