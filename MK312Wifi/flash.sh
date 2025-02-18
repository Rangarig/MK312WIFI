#!/bin/bash
PORT="/dev/ttyUSB0"
ARGS="--chip esp8266 --port $PORT --baud 115200"
#EXEC="python3 $HOME/.arduino15/packages/esp8266/hardware/esp8266/3.1.2/tools/esptool/esptool.py"
EXEC="esptool"
$EXEC $ARGS write_flash 0x0 MK312Wifi.ino.bin || exit
$EXEC $ARGS write_flash 0xEB000 MK312Wifi.mklittlefs.bin || exit
beep
