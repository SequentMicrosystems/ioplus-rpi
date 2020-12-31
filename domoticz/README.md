# domoticz.plugin.ioplus
Domoticz plugin for IOplus 8 relay board on raspberry
Author: Josef Pavlik

INSTALL:
copy this directory to ~/domoticz/plugins/ioplus and restart domoticz
cp -r . ~/domoticz/plugins/ioplus

SETUP:
Go to the Hardware menu and create new hardware of type "Raspberry IOplus". Set the board id.
Devices will be created automatically.
There are devices Relay_1 to Relay_8 and Input_1 to Input_8.
Currently only relays and opto isolated inputs are supported.
