#!/bin/sh

ESPRESSIF32=${HOME}/.platformio/packages/framework-arduinoespressif32
ESP32DEV=.pioenvs/esp32dev

WORK_DIR=firmware
TARGET_FILE=firmware.zip
MAP_FILE=map.txt

rm -rf ${WORK_DIR} || exit 1
mkdir ${WORK_DIR}  || exit 1

cp ${ESPRESSIF32}/tools/sdk/bin/bootloader_dio_40m.bin ${WORK_DIR}/ || exit 1
cp ${ESP32DEV}/partitions.bin ${WORK_DIR}/                          || exit 1
cp ${ESPRESSIF32}/tools/partitions/boot_app0.bin ${WORK_DIR}/       || exit 1
cp .pioenvs/esp32dev/firmware.bin ${WORK_DIR}/                      || exit 1

echo 0x01000 bootloader_dio_40m.bin >> ${WORK_DIR}/${MAP_FILE}
echo 0x08000 partitions.bin         >> ${WORK_DIR}/${MAP_FILE}
echo 0x0e000 boot_app0.bin          >> ${WORK_DIR}/${MAP_FILE}
echo 0x10000 firmware.bin           >> ${WORK_DIR}/${MAP_FILE}

zip -r ${TARGET_FILE} ${WORK_DIR}
