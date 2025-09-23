<h2>Arduino IDE</h2>

Download IDE from https://www.arduino.cc/en/software

Under File > Preferences, set Additional boards manager URLs to https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json,https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32_files/package_esp32_bluepad32_index.json

Under Tools > Manage Libraries, add these libraries:
 * ESP32Servo by Kevin Harrington

Under Tools > Board > Board Manager, add these boards
* esp32_bluepad32 by Richard Quesada

Under Tools > Board, select esp32_bluepad32 > ESP 32 Dev Module

Connect ESP32 Development Kit with USB cable
Under Tools > Port, select COM port used

Open Excavator_Bluepad.ino file and click right arrow on top tool bar to download code to ESP32

<h2>Controls</h2>

Controls are the same as a John Deere excavator.

<img src="https://github.com/swholmstead/Excavator/blob/main/pictures/Controls.png" alt="Controls" width=800>

There are 2 modes. To switch between these modes, just click in the left joystick button.

<h3>Drive Mode</h3>

* Left joystick y-axis controls speed and x-axis controls steering.

<h3>Excavator Mode</h3>

* Left joystick y-axis controls raising/lowering boom arm and x-axis controls cab swivel.
* Right joystick y-axis controls raising/lowering dipper arm and x-axis control bucket tilt.
* DPAD up/down or left/right bumper controls opening/closing claw
* DPAD left/right or left/right trigger controls rotation of the claw.
* "A" button controls lights on/off.
* "B" button executes "wiggle" function to move motors and flash lights.  If you have more than 1 vehicle,
this makes it easy to identify which one is paired with this controller.
