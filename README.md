# WIP
This project is working, but technically still Work In Process. I am trying to get all files together, but there might be some things missing.
if you have intrest in buying one and live in The Netherlands, feel free to contact me.

# Smart light
A smart 3D printed light bulb, its a smart light that includes for example; wifi and manual control, Clock mode, and multiple animations.

# What you need
[The hardware like for example the ESP32](https://dl.espressif.com/dl/package_esp32_index.json)
[Arduino-WiFiManager](https://github.com/jellewie/Arduino-WiFiManager) (already included)
[Arduino-Button](https://github.com/jellewie/Arduino-Button) (already included)
[Arduino-Stable-analog-read](https://github.com/jellewie/Arduino-Stable-analog-read) (already included)
just as a quick hint; make sure to upload the sketch to ESP before embedding it in.

=========================================================

# Quick start guide
Follow the following steps to setup the lamp, stop after the first step if you do not want to set up Wi-Fi.
1.	Connect a proper power supply. The LEDs will blink shortly a soft white for a split second to show that bootup was successful. The power supply is either 5V or 12V and the lamp will consume 18 watts at maximum, so as an example for 5V a 3.6A power supply is required to allow full brightness. 
2.	[Wi-Fi] Long press the button, this will make the LEDs go PURPLE/RED/PURPLE/RED this means it is trying to connect to Wi-Fi, if this takes more than 10 seconds it will have created an Access Point. Long press the button again to cancel this setup.
3.	Connect to this Access Point, by default it will be called "ESP 32".
4.	When connected go to 192.168.4.1 this will show a page where the WI-FI name and password can be set, do not forget to submit.
# Creating a unit
## Printing
There will be 3 files that need to be printed
1.	Lamp body.STL is the main body. It is suggested to do at least 0.5mm walls so light will not shine though as much.
2.	Lamp shield.STL is just a simple shield to add aluminium foil on, and will be intern so can be printed quick and dirty.
3.	Lamp stand.STL can have 3*2 embedded magnets in it, if the magnets are desired a layer pause need to be used.
## PCB & wires
Please refer the in the appendix - PCB & schematic for the schematic, but here are some common notes about the schematic.
The pinout of the DC jack has by default positive polarity. Meaning the outer tube, as shown in Figure 1 - positive polarity, is the ground connection. The lamp will fail to work otherwise. The power supply is either 5V or 12V, and the power consumption is 18 Watt. The difference in the voltage is the used LED strip. If the WS2812B is used it would need to be 5V, but if the WS2815 are used then 12V is required.
## Firmware
Please refer the in the appendix - Firmware for the source code. Not you need to install some libraries to Arduino to compile the code, for example the ESP32 library. This is further explained in the firmware itself.
# Features
## Button
Although these options change a bit over time, here is a list of 3 actions what the button generally does, these can also change in different modes.
>1.	StartPress Triggered when you start a press, will be rejected if sooner than Time_RejectStarts (80ms) of the last press).

>> Will toggle the lamp ON/OFF

>2.	StartDoublePress Triggered when you start a second press between Time_RejectStarts (80) and Time_StartDoublePress (200) ms after the last one ended.

>> Will change the mode to `DoublePressMode' (RAINBOW by default)

>3.	StartLongPress Triggered when you press the button longer than Time_StartLongPressMS (5000) but shorter than Time_ESPrestartMS (15000) ms.

> If in APmode it will cancel and reboot.

> If it was connected to WI-FI it will show its IP. The LEDs are divided into 10 sections, and each char in the IP range will be shown one at a time. Left down will be 0 and the next clockwise will be 1. Where RGB will be the order of the numbers, so red will have a *100 multiplication and green will have *10, so when the IP is 198.168.1.150 and it shows the last section (150) then section 1 will be Red, 5 will be green and 0 will be blue

>> If it isn't connected to WI-FI it will start connecting to WI-FI (and possible go into AP mode, that is explained in 3.4 WI-FI page. 
## Pot meter
The 4 pot meters are Red, Green, Blue and White. Where white stand for the luminescence. When any of these are turned the mode will switch to on/manual and the RGB value will be shown.
## WI-FI page
Here are the 2 most important pages listed with their descriptions. Keep in mind more functions could be added or a different layout could be used, but the intention should be the same as these.
Some special functions like manual updating time (IP/time?h=2&m=4&s=1) are not listed here. If important things missing feel free to contact me, but all functions can be found in the code. 
###Setup
The setup page will be accessible and shown in APmode, but can also be access by going to `IP/setup'.
The password is replaced with starts, and cannot be received without changing the firmware. 
Leave fields blank (or for the password leaving only stars) to not update those values upon sending this forum.
By default the SSID and password is limited to 16 characters by the firmware, and the total bytes that can be stored in memory is limited to 64. Going over these values results in unexpected behaviour. 
###Control
The control page is the default landing page the user will land on when the lamp is connected to Wi-Fi.
* The brightness slider will always change the brightness no matter in what mode it is in (except for AP since that is not a defined mode), these values are only send and updated on releasing the slide bar.
* The RGB sliders will either (try) change the animation accent to the given RGB colour, or switch the mode to Wi-Fi control and set the RGB color.
* Mode is the current mode the light is in.
* Bootmode is the mode the light will go into on start-up.
* Hourly animation is when the light is in the clock mode an hourly animation needs to be played.
* DoublePressMode is the mode the light will switch to when a double press on the button is made.
* Enable OTA will enable Arduino Over The Air updates, so the firmware can be updated remotely with the Arduino IDE.
* Sync Time will get the current updated time from the server, this function is called automatically every day on 04:00 if the light is in clock mode.
* Reset will fully restart the ESP

# Appendix
* Firmware
https://github.com/jellewie/Arduino-Smart-light
* PCB & schematic
https://easyeda.com/jellewietsma/smart-light
* 3D models
At the time of writing these are in this folde included
