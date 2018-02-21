# yudetamago-esd-wroom-02

## for developer

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
