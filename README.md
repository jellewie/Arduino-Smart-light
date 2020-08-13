# WIP
This project is technically still Work In Process. I am in the process of rounding off, just a few more last touches before it first release. 
if you have interest in buying one and live in The Netherlands, feel free to contact me.

# Smart clock
A smart 3D printed light clock/lamp, that includes, but is not limited to; WIFI and manual control, Clock mode, multiple animations, and transitions.

<img align="right" src="Images/Desk%20clock.jpeg" alt="Desk clock image" width=50%>
<img align="right" src="Images/Desk%20lamp.jpeg" alt="Desk lamp image" width=50%>

# What you need to make one
## Hardware
- a [PCB lamp](https://easyeda.com/jellewietsma/smart-light) or [PCB wall](https://easyeda.com/jellewietsma/smart-clock)
- All parts listed in the BOM of the PCB, these include resistors, capacitors, buttons, potmeters, and an ESP32
- Others things like Potmeter caps, DC jack cable and adapter, but also some magnets if you want the magnetic option for the desk version (these are by default 4x10mm round magnets)
## Firmware (To compile)
- [Arduino sketch](Arduino) The whole sketch is the 'Arduino' folder.
- [ESP32](https://dl.espressif.com/dl/package_esp32_index.json).
- [Arduino-WIFIManager](https://github.com/jellewie/Arduino-WiFiManager) (already included).
- [Arduino-Button](https://github.com/jellewie/Arduino-Button) (already included).
- [Arduino-Stable-analog-read](https://github.com/jellewie/Arduino-Stable-analog-read) (already included).

Upon release an .HEX file will also be supplied, this could then be uploaded with an HEX uploader of choice.

# Creating a unit
Please see [Quick start guide](#quick-start-guide) if you have already a light, and just want to run the set-up.
## Printing
### Desk model
There are 4 files that need to be printed
1.	[Lamp body.stl](3DModel/Desk/Body.stl) is the main body. It is suggested to do at least 0.5mm walls so light will not shine though as much.
2.	[Lamp shield.stl](3DModel/Desk/Shield.stl) is just a simple shield to add aluminium foil on, and will be intern so can be printed quick and dirty.
3.	[Lamp stand.stl](3DModel/Desk/Stand.stl) can have 3 times a pair embedded magnets in it, if the magnets are desired a layer pause need to be used.
4.	[Diffuser.gcode](3DModel/Desk/Diffuser.gcode) A perfect spiral Gcode, this can be created with [Jespers Perfect Spiral-Gcode](https://github.com/jespertheend/spiral-gcode).
### Wall model
There are 2 files that need to be printed
1.	[Clock Top.stl](3DModel/Wall/Clock%20Top.stl) This is the top of the clock, which houses the PCB controls.
2.	[Clock x2.stl](3DModel/Wall/Clock%20x2.stl) This needs to be printed twice, and is the bottom left and right.

## PCB & wires
Please refer the in the [appendix](#appendix) - PCB & schematic for the schematic, but here are some common notes about the schematic, for example in [Input voltage](#Specifications) and [Power consumption](#Power consumption) more information is given about the specifications of the electrics.

# Features
## Button
Although these options change a bit over time, here is a list of 3 actions what the button generally does, these can also change in different modes.
1.	**StartPress** Triggered when you start a press, will be rejected if sooner than Time_RejectStarts (80ms) of the last press).
- Will toggle the lamp ON/OFF

2.	**StartDoublePress** Triggered when you start a second press between Time_RejectStarts (80ms) and Time_StartDoublePress (200ms) after the last one ended.
- Will change the mode to 'DoublePressMode' (RAINBOW by default)

3.	**StartLongPress** Triggered when you press the button longer than Time_StartLongPressMS (5000) but shorter than Time_ESPrestartMS (15000) ms.
- If in APmode it will cancel and reboot.
- If it was connected to WIFI it will show its IP, Which is explained in [WIFI page](#wifi-page) - Getting it’s IP.
- If it isn’t connected to WIFI it will start connecting to WIFI (and possibly go into AP mode, that is explained in 
## LED status
In the back of the desk lamp is a LED. This is also the LED into PCB of the ESP (LED_BUILDIN) this LED reflects some more errors.
- **ON** WIFI starts, goes OFF when WIFI setup is successfully completed.
- **blink 100ms** WIFI setup failed, APmode has started.
- **blink 500ms** it is connecting to its known WIFI.
- **blink 1000ms** OTA is on.
- **blink 2000ms** WIFI is lost, so the current time (we are in CLOCK mode) cannot be updated.

Some statuses are also reflected by the LED strip itself, but due to the difficulty driving these in some WIFI modes, these are not used much.
- **White short blink** It is booting up.
- **PURPLE/RED/PURPLE/RED** It is starting the WIFI connect code.
- **PURPLE/GREEN/PURPLE/GREEN** It is starting the get server time code.
## Pot meter
The 4 pot meters are Red, Green, Blue, and White. Where white stand for the luminescence. When any of these are turned the mode will switch to on/manual and the RGB value will be shown.
## WIFI page
Here are the 2 most important pages listed with their descriptions. Keep in mind more functions could be added or a different layout could be used, but the intention should be the same as these.
Some special functions might not be listed here. If important things missing feel free to contact me!
### Setup
<img align="right" src="Images/ip.png" alt="WIFI page /ip">

The setup page will be accessible and shown in APmode, but can also be access by going to [smart-light.local/ip](http://smart-light.local/ip). An example of this page is shown in the figure on the right.
The password is replaced with starts, and cannot be received without changing the firmware. 
Leave fields blank (or for the password leaving only stars) to not update those values upon sending this forum.
By default the SSID and password is limited to 16 characters by the firmware, and the total bytes that can be stored in memory is limited to 128. Going over these values results in unexpected behaviour. 

### Getting it’s IP
To trigger this menu please see [Button](#Button), this part is just about how to read it. The LEDs are divided into 10 sections, and each char in the IP range will be shown one at a time. The numbers are like a clock and in clockwise direction, top/right is 0 and the one right/down of that is 1. Where RGB will be the order of the numbers, so red will have a x100 multiplication and green will have x10, so when the IP is 198.168.1.150 and it shows the last section (150) then section 1 will be Red, 5 will be green and 0 will be blue. This is shown in the figure below

<img src="Images/ShowIP.png" alt="ShowIP 198.168.1.150">
But on user devices who support mDNS, like Windows and Apple, [smart-light.local/info](smart-light.local/info) can be also be used to obtain its IP.

### Control
<img align="right" src="Images/Home.png" alt="Home page" width=50%>
The control page is the default landing page the user will land on. An example of this page is shown in the figure on the right.

- **The brightness slider** will always change the brightness no matter in what mode it is in (except for AP since that is not a defined mode), these values are only send and updated on releasing the slide bar. This will also turn off auto brightness.
- **The RGB sliders** will either (try) change the animation accent to the given RGB colour, or switch the mode to Wi-Fi control and set the RGB colour.
- **Settings button** The gear on the right lower corner will open the settings page.

<img align="right" src="Images/Settings.png" alt="Settings page" width=50%>
An example of the settings page is shown in the figure on the right.

- **Mode** is the current mode the light is in.
- **Bootmode** is the mode the light will go into on start-up.
- **Doublepress mode** is the mode the light will switch to when a double press on the button is made.
- **Auto brightness** will set the brightness automatically.
- **Hourly animation** is when the light is in the clock mode an hourly animation needs to be played.
- **Hourly lines** when in clock mode, will add lines to each hour, the value is the amount on a byte scale (0-255) dimmed by the brightness itself.
- **Analog hours** will let the hour indicator take 60 instead of 12 steps, so it will not stick to the whole hours indication.
- **Enable OTA** will enable Arduino Over The Air updates, so the firmware can be updated remotely with the Arduino IDE.
- **Sync time** will get the current updated time from the server, this function is called automatically every day on 04:00 if the light is in clock mode.
- **Reset** will fully restart the ESP.
- **Info** will open the info page with some information like the version compile date
- **Task** WIP this will show a task menu, so tasks can be scheduled, removed, changed, and listed.
- **Saved settings** just redirects you to [smart-light.local/ip](http://smart-light.local/ip) to show you the values saved in the EEPROM

### Soft settings
There are multiple soft settings, these are settings that can be changed after the sketch has been uploaded, but are still saved so they can be restored after a reboot.
The most up-to-date values can be found in the top of the [WifiManager.ino](Arduino/WifiManager.ino) source file, and can only be set in [smart-light.local/ip](http://smart-light.local/ip).
- **Bootmode** In which mode to start in after start-up
- **HourlyAnimation** If it needs to show hourly animations when in CLOCK mode.
- **DoublePressMode** In what mode to go into when the button is double pressed.
- **AutoBrightness** Turns on brightness if enabled, the curve of which can be set with N and P.
- **AutoBrightnessN** Brightness =(X-N)xP+O.
- **AutoBrightnessP**
- **AutoBrightnessO**
- **ClockHourLines** how bright each hour mark needs to be on a scale of 0 (OFF) to 255.
- **ClockHourAnalog** Will use all 60 LEDs to display the hour, not just stick to wholes and use 12.
- **ClockOffset** Number of LEDs to offset/rotate the clock, so 12 o'clock would be UP. Does NOT work in Animations.
- **ClockAnalog** Makes it so the LEDs dont step, but smootly transition. This does cost considerable more processing power. Makes ClockHourAnalog useless.
- **gmtOffset_sec** Offset of time in seconds of GMT, for example Amsterdam is GMT+1h so thats 3600 seconds.
- **daylightOffset_sec** Offset of time in seconds daylight saving time, for example Amsterdam has a 1 hour saving time so thats 3600 seconds.
- **PotMinChange** How much the pot_value needs to change before we process it.
- **PotStick** If this close to HIGH or LOW stick to it.
- **PotMin** On how much pot_value_change need to change, to set mode to manual.
- **Name** The mDNS, WIFI APmode SSID, and OTA name of the device. This requires a restart to apply, can only be 16 characters long, and special characters are not recommended.

### Clock mode
For the use of clock mode the right time zone needs to be set up, these are stored in gmtOffset_sec and daylightOffset_sec (Default to Amsterdam time). 

Clock mode can only be started when WIFI has been connected and the user switches mode, after switching to Clockmode the lamp will turn PURPLE/GREEN/PURPLE/GREEN this means it is getting the time from a time server.

Also the clock can be manual updated with [smart-light.local/time?h=2&m=4&s=1](http://smart-light.local/time?h=2&m=4&s=1) where H is hours, M is minutes, and S is seconds. The clock also automatically updates every day at 4:00

Furthermore the clock has some softsettings like ClockHourLines, but these are descibed in [Soft settings](#Soft%20settings)
# Quick start guide 
Follow the following steps to setup the lamp, stop after the first step if you do not want to set up Wi-Fi.
1.	Connect a proper power supply. The LEDs will blink shortly a soft white for a split second to show that bootup was successful.
2.	[Optional for WIFI] Long press the button, this will make the LEDs go PURPLE/RED/PURPLE/RED this means it is trying to connect to Wi-Fi, if this takes more than 10 seconds it will have created an Access Point. Long press the button again to cancel this setup.
3.	Connect to this Access Point, by default it will be called “smart-light”.
4.	When connected go to 192.168.4.1 this will show a page where the WIFI name and password can be set, do not forget to submit. The light will also try to make a captive portal to prompt you to login and setup these settings. Sadly, this only works with HTTP, use the mentioned IP if the device does not prompt the login page.
5.	When the device is connected to WIFI it can be accessed by its IP address, but on devices who support mDNS, like Windows and Apple, [smart-light.local/info](http://smart-light.local/info) can be used.

# Specifications 
## Input voltage
The power supply is either 5V or 12V. The difference in the voltage is the used LED strip. If the WS2812B (older model) is used it would need to be 5V, but if the WS2815 are used then 12V is required. The newer model WS2815 with redundant data lines is 12V. 

<img align="right" src="Images/positive polarity.png" alt="positive polarity">

The pinout of the DC jack has by default positive polarity. Meaning the outer tube, as shown in the figure right, is the ground connection. The lamp will fail to work otherwise (but is able to handle the shock).
## Power consumption
The lamp will consume 18Watts at maximum, so as an example for 12V a 1.5A (or 5V at 3.6A) power supply is required to allow full brightness. Although actual measurements came closer to 12Watt.

The idle power consumption is about 0.65Watt (measured with LEDs turned off, and WIFI turned on)

# Appendix
* Firmware
[This is included in this repository](Arduino)
* PCB & schematic
[Smart light](https://easyeda.com/jellewietsma/smart-light)
[Smart clock](https://easyeda.com/jellewietsma/smart-clock)
* 3D models
[These are included in this repository](3DModel)