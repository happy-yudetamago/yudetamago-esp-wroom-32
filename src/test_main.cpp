// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.
#ifdef TEST_ON_TARGET

#include <NeoPixelBus.h>
#include "hardware_defines.h"

RgbColor BLACK_COLOR(0, 0, 0);

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> pixels(NUM_OF_NEO_PIXELS, NEO_PIXEL_PIN);

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting Yudetamago...");
    pixels.Begin();
    for (int i=0; i<OBJECT_ID_SIZE; i++) {
        pixels.SetPixelColor(i, BLACK_COLOR);
    }
}

void loop()
{
    pixels.Show();

    /////////////////////////////////////////////////////////////
    // Use vTaskDelay() instead of delay()                     //
    // https://github.com/espressif/arduino-esp32/issues/595   //
    //                                                         //
    // Trouble log when using delay()                          //
    // - IDLE (CPU 0)                                          //
    // Tasks currently running:                                //
    // CPU 0: wifi                                             //
    // CPU 1: IDLE                                             //
    // Task watchdog got triggered.                            //
    // The following tasks did not reset the watchdog in time: //
    // - IDLE (CPU 0)                                          //
    // Tasks currently running:                                //
    // CPU 0: wifi                                             //
    // CPU 1: IDLE                                             //
    // ...                                                     //
    /////////////////////////////////////////////////////////////
    vTaskDelay(10);
}
#endif // TEST_ON_TARGET
