# yudetamago-esd-wroom-02

## for manufacturer

### How to set up object id

1. [yudetamago]  Power on with button x
1. [Smart phone] Access bluetooth
   Name : Yudetamago config
1. [Smart phone] Access using App
                 (support SPP profile)
1. [Smart phone] Config objectId
                 see CommandLineAPI
1. [yudetamago]  Power OFF, and Power ON

## for user

### How to set up

1. [yudetamago]  Power on with button x
1. [Smart phone] Access bluetooth
   Name : Yudetamago config
1. [Smart phone] Access using App
                 (support SPP profile)
1. [Smart phone] Config your SSID, and PASSWORD
                 see CommandLineAPI
1. [yudetamago]  Power OFF, and Power ON

## System Mode

### Config Mode

To config SSID, and objectId.

1. [yudetamago]  Power on with button 1

### OTA(Over The Air) Mode

To upload the firmware over the air.

1. [yudetamago]  Power on with buttons 1 and 2

### Normal Mode

To show yudetamago state, and to change yudetamago state.

1. [yudetamago]  Power on with no buttons

## CommandLineAPI

CommandLineAPI are enabled in two cases.

1. Bluetooth on ConfigMode
1. USB Serial on NormalMode

### set_ssid

Set Wifi SSID and Wifi Password command.
After restart, new SSID is enabled.

since v0.0.2

#### synopsis

```
set_ssid <Wifi SSID> <Wifi Password>
```

#### example

```
$ set_ssid aterm-xxx-g 38xxxxxxxxx23
[INFO ] Successed to write wifi config.
```

### set_objectid

Set yudetamago's objectId command.
After restart, new objectId is enabled.

since v0.0.2

#### synopsis

```
set_objectid <yudetamago objectId> <yudetamago objectId> <yudetamago objectId> <yudetamago objectId> <yudetamago objectId>
```

#### example

```
$ set_objectid KyPxxxxxxxxxx2Jf UyPxxxxxxxxxx2Jf QyPxxxxxxxxxx2Jf MyPxxxxxxxxxx2Jf lyPxxxxxxxxxx2kf
[INFO ] Successed to write objectId.
```

### set_led

Set led commnad.

since v0.0.2

#### synopsis

```
set_led <led id> <red> <green> <blue>
```

led id : 0, 1, 2, 3, 4
red    : from 0 to 255
green  : from 0 to 255
blue   : from 0 to 255

#### example

```
$ set_led 0 255 0 0
```

### get_button_state

Get button state commnad.

since v0.0.3

#### synopsis

```
get_button_state
```

#### example

```
$ get_button_state
[INFO ] button #0: not pressed
button #0: not pressed
[INFO ] button #1: pressed
button #1: pressed
[INFO ] button #2: not pressed
button #2: not pressed
[INFO ] button #3: not pressed
button #3: not pressed
[INFO ] button #4: not pressed
button #4: not pressed
```

### set_loglevel

Set log level commnad.

since v0.0.2

#### synopsis

```
set_loglevel <loglevel>
```

loglevel : TRACE, DEBUG, INFO, WARN, ERROR, or FATAL

#### example

```
$ set_loglevel TRACE
[INFO ] loglevel: set TRACE
```

### log

Show latest logs command.
The log capacity is 64.

since v0.0.2

#### synopsis

```
log
```

#### example

```
$ log
[INFO ] Detected Normal mode.
[INFO ] objectId : KyPxxxxxxxxxx2Jf
[INFO ] WiFi connecting...
[INFO ] WiFi connected.
[INFO ] IP address: 192.168.11.xx
[INFO ] Detected initial status: exists
[INFO ] SetExistance: not exist
```

### info

Show system info command.

since v0.0.2

#### synopsis

```
info
```

#### example

```
$ info
version=0.0.1
ssid=aterm-xxx-g
pass=38xxxxxxxxx23
objectId=KyPxxxxxxxxxx2Jf
```

### loop_color_change

loop a color change

since v0.0.5

#### synopsis

```
loop_color_change <times>
```

times : 1, 2, 3, ..., default is eternal.


#### example

```
$ loop_color_change
```

### reset

Reset system

since v0.0.7

#### synopsis

```
reset
```


#### example

```
$ reset
```

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
$ pio serialports monitor -b 115200
```

```
$ miniterm.py /dev/ttyUSB0 115200 | awk '{ print strftime(), $0; fflush() }'
Fri Oct 5 02:50:25 JST 2018 clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
Fri Oct 5 02:50:25 JST 2018 mode:DIO, clock div:2
Fri Oct 5 02:50:25 JST 2018 load:0x3fff0018,len:4
Fri Oct 5 02:50:25 JST 2018 load:0x3fff001c,len:952
Fri Oct 5 02:50:25 JST 2018 load:0x40078000,len:6084
Fri Oct 5 02:50:25 JST 2018 load:0x40080000,len:7944
Fri Oct 5 02:50:25 JST 2018 entry 0x40080310
Fri Oct 5 02:50:25 JST 2018 Starting Yudetamago...
Fri Oct 5 02:50:25 JST 2018 [INFO ] Detected Normal mode.
```

### Library

- Makuna/NeoPixelBus Library (LGPL v3.0)
  https://github.com/Makuna/NeoPixelBus
- Cryptographic suite for Arduino (SHA, HMAC-SHA) 
  https://github.com/simonratner/Arduino-SHA-256
  (Fix compile errors)

### ToDo

- Download firmware over TCP/IP(MD5)
- Fix LED bug

## Link

- ESP-WROOM-32 datasheet
    http://akizukidenshi.com/download/ds/espressifsystems/esp_wroom_32_datasheet_en.pdf
