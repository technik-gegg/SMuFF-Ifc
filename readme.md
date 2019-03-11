# SMuFF-Ifc

This project is an extension to the Smart Multi Filament Feeder (SMuFF) project as described here: [SMuFF]


It's basically an additional interface based on a ESP32 (or compatible) microcontroller board, which allows you to split up the serial interface on the Duet3D in order to achieve a communication to the SMuFF without interrupting the serial communication between the PanelDue and the Duet3D mainboard.
This interface can operate in different modes:

## Mode 1: Fully connected
![Mode 1][1]

This mode features a three way connection between the Duet3D, the PanelDue and the SMuFF.
Besides, the integrated Bluetooth serial port (SPP) allows you to monitor (debug) the connections or even to mirror the data comming from the Duet3D towards the PanelDue.
This would enable you to realize some kind of remote display/controller on your smartphone in more or less the same way the PanelDue does (although, you'd need a separate Android/iOS app to accomplish that).
All the data sent by the Duet3D will go straight to the PanelDue and vice versa. Special SMuFF related commands have to be prefixed with  "\s" when sending messages i.e. 
> M118 P2 S"\sT3\n"

This sequence commands the SMuFF-Ifc to send the string "**T3\n**" directly to the SMuFF. The "\n" at the end will be translated to a "Linefeed" character automatically, which triggers the SMuFF to switch to tool 3.

## Mode 2: Serial communication without PanelDue
![Mode 2][2]

Same function as in Mode 1, only without the PanelDue attached. 

## Mode 3: I2C connection from Duet3D
![Mode 3][3]

This mode allows you to send commands to the SMuFF using the I2C/TWI interface instead of the serial interface. On the Duet3D this makes not much sense, since the serial interface does the same but with more flexibility because it's bidirectional.  

## Mode 4: I2C connection from Marlin compatible boards
![Mode 4][4]

As of yet, this mode is the only way to make a Marlin compatible board talk to the SMuFF. It's meant to be used in conjunction with your favorite slicer, which will send a I2C command to the interface, as soon as a tool change is immintent.
Although, for now this is plain theorie which hasn't been tested yet.

# Configuration
To configure the SMuFF-Ifc, first upload the firmware and the file system (stuff in the data directory) to your device, then use the Internet browser of your choice to connect to the (HTTP) web interface.
There you'll be able to modify the settings comfortabely. 
Alternatively, you can edit the config.json file from the **data** directory in order to achive the same.

[SMuFF]: https://github.com/technik-gegg/SMuFF
[1]: https://github.com/technik-gegg/SMuFF-Ifc/blob/master/images/Config_full.png "Mode 1"
[2]: https://github.com/technik-gegg/SMuFF-Ifc/blob/master/images/Config_serial.png "Mode 2"
[3]: https://github.com/technik-gegg/SMuFF-Ifc/blob/master/images/Config_I2C_Duet.png "Mode 3"
[4]: https://github.com/technik-gegg/SMuFF-Ifc/blob/master/images/Config_I2C_Marlin.png "Mode 4"

