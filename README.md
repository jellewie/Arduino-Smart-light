# Smart clock
A smart 3D printed light clock/lamp, that includes, but is not limited to; WIFI and manual control, Clock mode, multiple animations, and transitions.

If you have interest in buying one and live in The Netherlands, feel free to contact me.

You can skip to [Quick start guide](#quick-start-guide) if you already own a smart-clock, and just want to run the set-up.

You can skip to [OTA updater](#ota-over-the-air-update) if you just want to update it.

# Index
<details><summary>Show/hide</summary><p>

 - [Quick start guide](#quick-start-guide)
 - [Creating a unit](#creating-a-unit)
    - [Hardware](#hardware)
    - [Creating the model](#creating-the-model)
       - [Desk model](#desk-model)
       - [Wall model](#wall-model)
          - [3D printed only](#3d-printed-only)
          - [CNC](#cnc)
    - [PCB & wires](#pcb--wires)
    - [Firmware ](#firmware)
       - [Compile](#compile)
       - [BIN](#bin)
 - [Features](#features)
    - [Button](#button)
    - [LED status](#led-status)
    - [Potentiometer](#potentiometer)
    - [WIFI page](#wifi-page)
       - [Setup](#setup)
       - [Getting it’s IP](#getting-its-ip)
       - [Control](#control)
       - [Soft settings](#soft-settings)
       - [Clock mode](#clock-mode)
       - [Task List](#task-list)
       - [OTA (Over The Air update)](#ota-over-the-air-update)
       - [Full reset](#full-reset)
 - [Specifications ](#specifications)
    - [Input voltage](#input-voltage)
    - [Power consumption](#power-consumption)
 - [Appendix](#appendix)
</p></details>

# Quick start guide
Follow the following steps to setup the lamp, stop after the first step if you do not want to set up Wi-Fi.
1.	Connect a proper power supply. The LEDs will blink shortly a soft white for a split second to show that bootup was successful.
2.	The [potmeters](#pot-meter) or [button](#button) will change the light.

**Optional for WIFI:**
1.	Long press the button, this will make the LEDs go PURPLE/RED/PURPLE/RED this means it is trying to connect to Wi-Fi, if this takes more than 10 seconds it will have created an Access Point. (Long press the button again to cancel this setup)
2.	Connect to this Access Point, by default it will be called “smart-clock”.
3.	When connected go to [192.168.4.1](http://192.168.4.1/) this will show a page where the WIFI name (SSID) and password (Password) can be set, do not forget to submit to apply. Other settings on this page are explained in [soft settings](#soft-settings), for example 'Clockoffset' is the rotation of the LEDs (there are 60 LEDs so filling in 30 will rotate the clock by 50%). 
~~The light will also try to make a captive portal to prompt you to login and setup these settings. Sadly, this only works with HTTP, use the mentioned IP if the device does not prompt the login page ([Not working as of now, Known issue](https://github.com/jellewie/Arduino-Smart-light/issues/28))~~ 
4.	When the device is connected to WIFI it can be accessed by its IP address, but on devices who support mDNS, like Windows and Apple, [smart-clock.local/](http://smart-clock.local/) can also be used.

<img align="right" src="Images/Desk%20clock.jpeg" alt="Desk clock image" width=50%>
<img align="right" src="Images/Desk%20lamp.jpeg" alt="Desk lamp image" width=50%>

# Creating a unit
## Hardware
- A [PCB for the lamp/wall model](https://easyeda.com/jellewietsma/smart-light), or a lot of patient copying the schematic and doing it by hand.
- Parts listed in the [BOM](BOM.md) (or the BOM on EasyEDA), these include resistors, capacitors, buttons, potmeters, and an ESP32 as examples.
- An [model](#creating-the-model) to put everything in. This can either be completly 3D printed, or CNCed, and can also differ in model itzelf like the [DESK](#desk-model) or the [WALL](#wall-model) model.

## Creating the model
There multiple types of model, the '[DESK model](#desk-model)' is used in this readme as example project. But the other models have the same features, just a different shape. 

All STL files for 3D-printing have a predefined tolerance of 0.2mm build-in.
### Desk model
The model is [Desk.ipt](3DModel/Desk/Desk.ipt), for this model there are 4 files that need to be printed.
1.	[Lamp body.stl](3DModel/Desk/Body.stl) is the main body. It is suggested to do at least (0.4mm*3lines=1.2mm) walls so light will not shine though as much. It is printed on its face (like the STL orientation) and only the inner part needs a bit of support (overhang angle 90 degrees, density 2%).
2.	[Lamp shield.stl](3DModel/Desk/Shield.stl) is just a simple shield to add aluminium foil on, and will be intern so can be printed quick and dirty.
3.	[Lamp stand.stl](3DModel/Desk/Stand.stl) can have 3 times a pair embedded magnets in it, if the magnets are desired a layer pause need to be used. this model should be printed in the normal standing position on the round flat part.
4.	[Diffuser.gcode](3DModel/Desk/Diffuser.gcode) A perfect spiral Gcode, this can be created with [Jespers Perfect Spiral-Gcode](https://github.com/jespertheend/spiral-gcode). Altough a STL file is also included.

### Wall model
This model and PCB is 'Work In Progress'!

There are 2 variants, and fully 3d printed model, and a CNC model.

#### 3D printed only
The model is [Wall Clock.ipt](3DModel/Wall/Wall%20Clock.ipt), for this model there are 2 files that need to be printed.
1.	[Clock Top.stl](3DModel/Wall/Clock%20Top.stl) This is the top of the clock, which houses the PCB controls.
2.	[Clock x2.stl](3DModel/Wall/Clock%20x2.stl) This needs to be printed twice, and is the bottom left and right.

#### CNC
The model is [Wall Clock CNC.ipt](3DModel/Wall/Wall%20Clock&20CNC.ipt).
1.  [Wall Clock CNC.ipt](3DModel/Wall/Wall%20Clock&20CNC.ipt) This is the part that needs to be cut, it has thicker walls and no LED seperates
2.	The CNC .ipt mentioned above does have hidden LED seperates in the file, the idea was to 3d print them but this is still a WIP idea thing.

## PCB & wires
Please refer the in the [appendix](#appendix) - PCB & schematic for the schematic, but here are some common notes about the schematic, for example in [Input voltage](#Specifications) and [power consumption](#power-consumption) more information is given about the specifications of the electrics.

## Firmware 
The firmware needs to be flashed once to enable [OTA](#ota-over-the-air-update) BIN file upload. Either with Arduino IDE and compile it, or using a BIN uploader of choice.

### Compile
- [Arduino sketch](Arduino) The whole sketch is the 'Arduino' folder.
- [ESP32](https://dl.espressif.com/dl/package_esp32_index.json) must be added as an additional board manager in Arduino IDE.
- [FastLED](https://github.com/FastLED/FastLED) can be downloaded though the built-in library anager in Arduino IDE.

- [Arduino-WIFIManager](https://github.com/jellewie/Arduino-WiFiManager) (already included).
- [Arduino-Button](https://github.com/jellewie/Arduino-Button) (already included).
- [Arduino-Stable-analog-read](https://github.com/jellewie/Arduino-Stable-analog-read) (already included).

### BIN
Uploading a BIN file with a cable to the ESP can be done by any [ESP32 BIN file uploader](https://www.espressif.com/en/support/download/other-tools). this step is required for the first flash time.

The BIN files themself can be found at [releases](https://github.com/jellewie/Arduino-Smart-light/releases). The newest version is always recommended and can be updated with [OTA](#ota-over-the-air-update).

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
In the back of the [desk](#desk-model) model is a LED (below the button). This is also the LED on the PCB of the ESP (LED_BUILDIN) this LED reflects thes errors:
- **ON** WIFI starts, goes OFF when WIFI setup is successfully completed.
- **blink 100ms** WIFI setup failed, APmode has started.
- **blink 2000ms** WIFI is lost, so the current time (we are in CLOCK mode) cannot be updated.
- **blink 500ms** it is connecting to the given WIFI.

Some statuses are also reflected by the LED strip itself, but due to the difficulty driving these in some WIFI modes, these are static set at the start of the status.
- **White short blink** It is booting up.
- **PURPLE/BLUE/PURPLE/BLUE** It is starting the WIFI connect code.
- **PURPLE/RED/PURPLE/RED** It is starting the Access Point code (WIFI could not connect).
- **PURPLE/GREEN/PURPLE/GREEN** It is starting the get server time code.

## Potentiometer
The 4 Potentiometers are Red, Green, Blue, and White. Where white stand for the luminescence. 
When any of the RGB Potentiometers are turned the mode will switch to on/manual and the RGB value will be shown.

## WIFI page
The 2 most important pages are the [Setup](#setup) page, where the user can setup the light. And the [main landing page](#control) where the UI is that the user uses to contol the smart-clock.

### Setup
<img align="right" src="Images/ip.png" alt="WIFI page /ip" width=30%>

The setup page will be accessible and shown in APmode, but can also be access by going to [smart-clock.local/ip](http://smart-clock.local/ip). An example of this page is shown in the figure on the right.
The password is replaced with starts, and cannot be received without changing the firmware. 
Leave fields blank (or for the password leaving only stars) to not update those values upon sending this forum.
By default the SSID and password is limited to 16 characters by the firmware, and the total bytes that can be stored in memory is limited to 128. Going over these values results in unexpected behaviour. 

### Getting it’s IP
User devices that support mDNS, like Windows and Apple, can use [smart-clock.local/info](http://smart-clock.local/info) to obtain its IP.

Alternatively the IP can be shown in the LEDs themself. To trigger this see [Button](#Button).
The LEDs are divided into 10 sections, and each byte in the IP range will be shown one at a time, to goto the next section press the button shortly again. The numbers are like a clock and in clockwise direction, top/right is 0 and the one right/down of that is 1. Where RGB will be the order of the numbers, so red will have a x100 multiplication and green will have x10, so when the IP is 198.168.1.150 and it shows the last section (150) then section 1 will be Red, 5 will be green and 0 will be blue. This is shown in the figure below

<img src="Images/ShowIP.png" alt="ShowIP 198.168.1.150">

### Control
<img align="right" src="Images/Home.png" alt="Home page" width=50%>
The control page is the default landing page the user will land on. An example of this page is shown in the figure on the right.

- **The brightness slider** will always change the brightness no matter in what mode it is in (except for AP since that is not a defined mode), these values are only send and updated on releasing the slide bar. This will also turn off auto brightness.
- **The RGB sliders** will either (try) change the animation accent to the given RGB colour, or switch the mode to Wi-Fi control and set the RGB colour.
- **Settings button** The gear on the right lower corner will open the settings page.

<img align="right" src="Images/Settings.png" alt="Settings page" width=50%>
An example of the settings page is shown in the figure on the right.

- **Mode** is the current mode the light is in.
- **Bootmode** is the mode the light will go into on start-up. Altough not all modes are visible on the webpage it supports all modes. To do this a custom webrequest needs to be made (in this case and in the time of writing [smart-clock.local/set?m=MOVE](smart-clock.local/set?m=MOVE)).
- **Doublepress mode** is the mode the light will switch to when a double press on the button is made.
- **Auto brightness** will set the brightness automatically.
- **Hourly animation** is when the light is in the clock mode an hourly animation needs to be played.
- **Hourly lines** when in clock mode, will add lines to each hour, the value is the amount on a byte scale (0-255) dimmed by the brightness itself..
- **Analog hours** will let the hour indicator take 60 instead of 12 steps, so it will not stick to the whole hours indication.
- **Enable OTA** will direct to the [OTA](#ota-over-the-air-update) update page, where the firmware can be updated over the WiFi.
- **Sync time** will get the current updated time from the server, this function is called automatically every day on 04:00 if the light is in clock mode.
- **Reset** will fully restart the ESP.
- **Info** will open the info page with some information like the version compile date.
- **Task** this will show a task menu, so tasks can be scheduled or removed.
- **Saved settings** just redirects you to [smart-clock.local/ip](http://smart-clock.local/ip) to show you the values saved in the EEPROM.

### Soft settings
There are multiple soft settings, these are settings that can be changed after the sketch has been uploaded, but are still saved so they can be restored after a reboot.
The most up-to-date values can be found in the top of the [WiFiManagerBefore.h](Arduino/WiFiManagerBefore.h) source file, and can only be set in [smart-clock.local/ip](http://smart-clock.local/ip).
These settings are saved EEPROMSaveDelayMS (Default 30000ms) after the last change of SoftSettings, or directly after APmode and in the [smart-clock.local/ip](http://smart-clock.local/ip) page.
Note that the character " and TAB (EEPROM_Seperator) cannot be used, these will be replaced with ' and SPACE respectively. Leave black to skip updating these, use spaces ' ' to clear the values
- **Bootmode** In which mode to start in after start-up
- **HourlyAnimationS** If it needs to show hourly animations when in CLOCK mode, defined in time in seconds where 0=off.
- **DoublePressMode** In what mode to go into when the button is double pressed.
- **AutoBrightness** Turns on brightness if enabled, the curve of which can be set with N and P.
- **AutoBrightnessN** Used to calculate auto brightness =P*(X-N)+O. please see https://www.desmos.com/calculator/5tpxxofamn
- **AutoBrightnessP** ^ Just the lowest raw sensor value you can find
- **AutoBrightnessO** ^ Just a brightness offset, so it can be set to be globally more bright
- **ClockHourLines** how bright each hour mark needs to be on a scale of 0 (OFF) to 255.
- **ClockHourAnalog** Will use all 60 LEDs to display the hour, not just stick to wholes and use 12.
- **ClockOffset** Number of LEDs to offset/rotate the clock, so 12 o'clock would be UP. Does NOT work in Animations.
- **ClockAnalog** Makes it so the LEDs do not step but smoothly transition. This does cost considerable more processing power. Makes ClockHourAnalog useless. https://www.desmos.com/calculator/zkl6idhjvx
- **timeZone** Set to you Timezone, see https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv, for example "CET-1CEST,M3.5.0,M10.5.0/3" for Amsterdam (DEFAULT)
- **RESERVED**
- **PotMinChange** How much the pot_value needs to change before we process it.
- **PotStick** If this close to HIGH or LOW stick to it.
- **PotMin** On how much pot_value_change need to change, to set mode to manual.
- **Name** The mDNS, WIFI APmode SSID, and OTA name of the device. This requires a restart to apply, can only be 16 characters long, and special characters are not recommended.
- **Task#** 8 tasks (0-7), the format is described in [task-list](#task-list)

### Clock mode
For the use of clock mode the right time zone needs to be set up, these are stored in timeZone (Default to Amsterdam). 

Clock mode can only be started when WIFI has been connected and the user switches mode, after switching to Clockmode the lamp will turn PURPLE/GREEN/PURPLE/GREEN this means it is getting the time from a time server.

Also the clock can be manual updated with [smart-clock.local/time?h=3&m=59&s=50](http://smart-clock.local/time?h=3&m=59&s=50) where H is hours, M is minutes, and S is seconds. The clock also automatically updates every day at 4:00

Furthermore the clock has some softsettings like ClockHourLines, but these are descibed in [Soft settings](#soft-settings)

### Task List
<img align="right" src="Images/Tasks.png" alt="Tasks page" width=50%>
This page can be accessed on by opening the settings menu and clicking on the button 'Tasks'
There can be 16 tasks, and the first 8 will be saved to EEPROM and will be restored after boot. 
By default the SYNCTIME had been added at 4:0:0

The format of Tasks "TimeH,TimeM,TimeS,TaskType,Var"
- I, the number in the list (0-15) assigned automatically 
- Time, 3x in the format of HH:MM:SS "x,y,z" |OR| time in ms to execute "w" (w are not saved in EEPROM)
- TaskType, the type of task and automatically converted to show the name. (see TaskString in Task.h)
- Var, this can be a custom variable (few examples are given below)

Some examples of TaskType and Var:
	TaskType (x but converted to name) - Var (x but can be converted to name)
1. SWITCHMODE - New mode to switch to "x" either in string or ID form 
2. DIMMING - Stepsize,GoTo,TimeInterfall in ms "x,y,z"
3. BRIGHTEN - Stepsize,GoTo,TimeInterfall in ms "x,y,z"
4. RESETESP
5. CHANGERGB - Red,Green,Blue,OPT_Brightness "x,y,z" |OR| "x,y,z,w"
6. SAVEEEPROM
7. SYNCTIME
8. AUTOBRIGHTNESS - True/false "x" either in string or ID form 
9. HOURLYANIMATIONS - Time in seconds for the animation "x" 

### OTA (Over The Air update)
This page can be accesed on [smart-clock.local/ota](http://smart-clock.local/ota) (or 'IP/ota') and enables you to update firmware over WiFi.
On this page is a 'choose file' button where the new version can be selected. Make sure the right, compatible, most updated file is selected ("Smart_clock.bin"). This can be downloaded from [releases](https://github.com/jellewie/Arduino-Smart-light/releases). 
After which the 'Upload' button needs to be press for the update process to begin, the unit will automatically update and reboot afterwards.
Note that [SoftSettings](#soft-settings) are preserved.

### Full reset
If a full reset is desired it can be achieved by going to 'smart-clock.local/reset'. But note that accessing the page will directly wipe all [SoftSettings](#soft-settings) from existence and there will be no way to restore them back. If the wipe was successful it will be reported back and will execute a restart.

# Specifications 
## Input voltage
The power supply is either 5V or 12V. The difference in the voltage is the used LED strip. If the WS2812B is used it would need to be 5V, but if the WS2815 (with redundant data lines) is used then 12V is required.

<img align="right" src="Images/positive polarity.png" alt="positive polarity">

The pinout of the DC jack has by default positive polarity. Meaning the outer tube -as shown in the figure right- is the ground connection. The lamp will fail to work otherwise (but is able to handle the shock).
## Power consumption
The lamp will consume 18Watts at maximum, so as an example for 12V a 1.5A (or 5V at 3.6A) power supply is required to allow full brightness. Although actual measurements came closer to 12Watt.

The idle power consumption is about 0.65Watt (measured with LEDs turned off, and WIFI turned on)

# Appendix
* Firmware
[This is included in this repository](Arduino)
* PCB & schematic
[This is included in this repository](Schematic-PCB)
Beta/ updated version on [EasyEDA](https://easyeda.com/jellewietsma/smart-light)
* 3D models
[These are included in this repository](3DModel)
* BOM
[This is included in this repository](BOM.md)
