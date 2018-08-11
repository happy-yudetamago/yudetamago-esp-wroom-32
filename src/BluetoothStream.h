// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#pragma once

#include <vector>
#include "Stream.h"

class BLEServer;
class BLECharacteristic;

class BluetoothStream : public Stream
{
public:
    BluetoothStream();
    ~BluetoothStream();

public:
    void Initialize();
    boolean Update();
    void AppendReadBuffer(char ch);

public:
    // Stream.h
    int available();
    int read();
    int peek();
    void flush();

    // Print.h
    size_t write(uint8_t ch);

private:
    boolean            initialized;
    std::vector<char>  readStream;
    BLEServer         *pServer;
    BLECharacteristic *pTxCharacteristic;
    BLECharacteristic *pRxCharacteristic;
};

