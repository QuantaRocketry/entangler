# Entangler #

The entangler board is designed to be a telemetry board capable of communication between ground and air.

## About ##

The Entangler board is primarily a GPS beacon. It operates on the LoRa protocol in the 915 MHz band.

The board includes an RS485 bus to connect either to other Quanta devices or extra custom devices.

## Receiving the telemetry ##

The Entangler board can be used as either the onboard communicator or used as the ground station receiver. There is no modifications needed to the program to use it as the receiver, just simply plug it into a computer and open the serial device.

There is the option to use the Adafruit Feather RP2040 LoRa board instead as the ground receiver. If this board is selected, the device will only enable the LoRa module.

