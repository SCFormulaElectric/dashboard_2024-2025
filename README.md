# Dash Board

Dashboard implementation for USC's F1 Electric Car. We use a Teensy4.1 microcontroller to send data to the Nextion LED screen to display essential data to the driver such as battery charge and speed. The Teensy recieves these important data by reading data from the VCU.

To get started, flash the dashboard.ino software onto the teensy4.1 via USB connection. Connect UART serial lines to dashboard. Connect CAN bus to VCU.

The dashboard can be modified using a USB to Serial plug, using the Nextion Editor software on Windows or Linux. Designing the dashboard is as simple as dragging different components from the editor and changing coordinates, width, height, and any property listed in the bottom right corner of the nextion editor when highlighting an object.

To change values through code on teensy 4.1, start a serial with baud rate of 9600, connecting Tx of the teensy to Rx of the dashboard. Print through the serial with the format of "Serial.print('[objectname].val=[value]')". To send it over,use the Serial to WRITE "0xFF" 3 times. This is the command to let the dashboard know it should parse the payload you sent it. We condensed it down to a single function called sendNumberToNextion.

Current objects names on dashboard:
Battery percentage bar: probat
Battery number: numbat
Speed: numspeed
Motor Temperature: mtrtemp
Motor Controller Temperature: mtrctrltemp

![alt text](https://github.com/SCFormulaElectric/Dash-Board/blob/main/Photos/teensyNextionLED.jpg)
