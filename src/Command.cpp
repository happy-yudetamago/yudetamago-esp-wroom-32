#include "Command.h"
#include "BluetoothSerial.h"

#include "CommandLineParser.hpp"

static BluetoothSerial SerialBT;

void Command::Start()
{
    SerialBT.begin("Yudetamago config"); // Bluetooth device name

    while (1) {
        if (SerialBT.available()) {
            Serial.write(SerialBT.read());
        }
        delay(20);
    }
}
