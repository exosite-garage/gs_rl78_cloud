========================================
About GainSpan Wifi module with RL78 Demo
========================================
This project is an IAR Renesas RL78 project that using GainSpan wifi module GS1011M to demo Exosite cloud activatation, <br>
and cloud connectivity to send and receive data to/from the cloud by using Exosite's Cloud Data Platform.<br> 

The project functionality includes:<br>
1) New device activation and save cik to EEPROM.<br>
2) Period send data to draw triangle wave to Exosite cloud.<br>
3) Send custom value to Exosite cloud.<br>
4) Read data source on Exosite cloud.<br>

License is BSD, Copyright 2012, Exosite LLC (see LICENSE file)

Tested and developed ith IAR for RL78 30-day evaluation.<br>

========================================
Quick Start
========================================
1) Install IAR Embedded Workbench for Renesas RL78 30-day evaluation<br>
http://www.iar.com/en/Products/IAR-Embedded-Workbench/Renesas-RL78/ <br>
2) Download all project files<br>
3) Edit HostApp.h -> enable the security type that you are using and fill in the SSID and passphrase<br>
4) In the https://renesas.exosite.com Portal, Add a device with of type "YRDKRL78 with GainSpan WiFi" -> enter the MAC address of the WiFi module<br>
5) Plug the GainSpan Wifi module into the J6 connector of the YRDKRL78G13<br>
6) Confirm SW5.2 is OFF then plug in the USB cable to your PC<br>
7) Compile the project and download the program to the board<br>
8) If the module successfully associates with your WiFi Access Point and connects to the Exosite server, the device will exchange security keys and will be activated<br>
9) When connected and activated, the device will send "ping", board temp and ADC1 values periodically<br>
10) When connected, the LEDs on the board can be turned on and off from the cloud by modifying the "LED Control" command data source in your https://renesas.exosite.com Portal (or via the API).<br>

========================================
Release Info
========================================
----------------------------------------
Release 2012-03-20
----------------------------------------
--) v1.01 release<br>


