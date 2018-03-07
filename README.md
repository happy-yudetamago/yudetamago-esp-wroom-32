# yudetamago-esd-wroom-02

## for developer

### How to modify flash size

edit default.csv

```diff
--- ~/.platformio/packages/framework-arduinoespressif32@src-537c58760dafe7fcc8a1d9bbcf00b6f6/tools/partitions/default.csv.default	2018-03-08 03:54:26.070449130 +0900
+++ ~/.platformio/packages/framework-arduinoespressif32@src-537c58760dafe7fcc8a1d9bbcf00b6f6/tools/partitions/default.csv	2018-03-08 03:55:06.422823814 +0900
@@ -1,7 +1,7 @@
 # Name,   Type, SubType, Offset,  Size, Flags
 nvs,      data, nvs,     0x9000,  0x5000,
 otadata,  data, ota,     0xe000,  0x2000,
-app0,     app,  ota_0,   0x10000, 0x140000,
-app1,     app,  ota_1,   0x150000,0x140000,
+app0,     app,  ota_0,   0x10000, 0x180000,
+app1,     app,  ota_1,   0x190000,0x100000,
 eeprom,   data, 0x99,    0x290000,0x1000,
 spiffs,   data, spiffs,  0x291000,0x16F000,
```

edit esp32dev.json

```diff
--- ~/.platformio/platforms/espressif32@src-9112e7894fcfd1325e3c29a50a2b0d5d/boards/esp32dev.json.default	2018-03-08 04:47:47.884546218 +0900
+++ ~/.platformio/platforms/espressif32@src-9112e7894fcfd1325e3c29a50a2b0d5d/boards/esp32dev.json	2018-03-08 04:48:17.665823356 +0900
@@ -23,7 +23,7 @@
   "upload": {
     "flash_size": "4MB",
     "maximum_ram_size": 294912,
-    "maximum_size": 1310720,
+    "maximum_size": 1572864,
     "require_upload_port": true,
     "speed": 115200,
     "wait_for_upload_port": true
```

```
$ pio run -t clean
$ rm -rf .pioenvs
```

see https://github.com/platformio/platform-espressif32/issues/58

### build firmware

```
$ pio run
```

### upload firmware

1. [yudetamago] push boot
1. [yudetamago] push reset
1. [yudetamago] pull reset
1. [yudetamago] pull boot

```
$ pio run --target upload
```

### monitor serial

```
$ platformio serialports monitor -b 115200
```

## for manufacturer

### How to set up object id

1. [yudetamago]  Power on with button x
1. [Smart phone] Access wifi
   SSID : yudetamago_config
   PASS : yudetamago
1. [Smart phone] Access using web browser
   http://192.168.4.1/admin
1. [Smart phone] Config objectId
1. [yudetamago]  Power OFF, and Power ON

## for user

### How to set up

1. [yudetamago]  Power on with button x
1. [Smart phone] Access wifi
   SSID : yudetamago_config
   PASS : yudetamago
1. [Smart phone] Access using web browser
   http://192.168.4.1
1. [Smart phone] Config your SSID, and PASSWORD
1. [yudetamago]  Power OFF, and Power ON

## Library

- Adafruit NeoPixel Library (LGPL v3.0)
  https://github.com/adafruit/Adafruit_NeoPixel
- Cryptographic suite for Arduino (SHA, HMAC-SHA) 
  https://github.com/simonratner/Arduino-SHA-256
  (Fix compile errors)

## ToDo

- Error log(optional)
  - View from /admin.html
- 5 button(optional)
- Web API
  Wifi config
  objectId

## Link

- ESP-WROOM-32 datasheet
    http://akizukidenshi.com/download/ds/espressifsystems/esp_wroom_32_datasheet_en.pdf
