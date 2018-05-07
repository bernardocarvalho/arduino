# Makefile for arduino-mk on Debian
# http://www.mjoldfield.com/atelier/2009/02/arduino-cli.html
#
#.PHONY: list
#list:
#	@$(MAKE) -pRrq -f $(lastword $(MAKEFILE_LIST)) : 2>/dev/null | awk -v RS= -F: '/^# File/,/^# Finished Make data base/ {if ($$1 !~ "^[#.]") {print $$1}}' | sort | egrep -v -e '^[^[:alnum:]]' -e '^$@$$' | xargs

BOARD_TAG    = uno
#ARDUINO_PORT = /dev/cu.usb*
ARDUINO_PORT = /dev/ttyACM*

#ARDUINO_LIBS = DallasTemperature OneWire
ARDUINO_LIBS = MAX31850_DallasTemp MAX31850_OneWire
ARDUINO_SKETCHBOOK = /home/isttok/Arduino
USER_LIB_PATH = /home/isttok/Arduino/libraries

#ARDUINO_LIBS = Ethernet Ethernet/utility SPI

include /usr/share/arduino/Arduino.mk
#/usr/local/arduino/Arduino.mk

