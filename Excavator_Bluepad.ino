#include <Arduino.h>
#include <ESP32Servo.h>  // by Kevin Harrington
#include <Bluepad32.h>

ControllerPtr myController;

#define clawServoPin 13

#define leftMotor0  18  // Used for controlling the left motor movement
#define leftMotor1   4  // Used for controlling the left motor movement
#define rightMotor0 21  // Used for controlling the right motor movement
#define rightMotor1 19  // Used for controlling the right motor movement
#define swingMotor0 23  // Used for controlling cab swing movement
#define swingMotor1 22  // Used for controlling cab swing movement

#define lightsAttach 15 // Used for controlling headlight control

#define boomMotor0   25 // Used for controlling boom movement
#define boomMotor1   26 // Used for controlling boom movement
#define dipperMotor0 27 // Used for controlling dipper movement
#define dipperMotor1 14 // Used for controlling dipper movement
#define bucketMotor0 32 // Used for controlling bucket movement
#define bucketMotor1 33 // Used for controlling bucket movement

#define swingDeadZone 30
#define boomDeadZone 30
#define dipperDeadZone 30
#define bucketDeadZone 30
#define clawDeadZone 30
#define clawInitialPosition 90

#define wiggleCountMax 6

Servo clawServo;

int lightSwitchTime = 0;
int clawValue = clawInitialPosition;
bool lightsOn = false;
unsigned long lastWiggleTime = 0;
int wiggleCount = 0;
int wiggleDirection = 1;
long wiggleDelay = 100;
bool shouldWiggle = false;

void onConnectedController(ControllerPtr ctl) {
  ControllerProperties properties = ctl->getProperties();
  auto btAddress = properties.btaddr;
  if (myController == nullptr) {
    Serial.printf("CALLBACK: Controller is connected, ADDR=%2X:%2X:%2X:%2X:%2X:%2X\n",
                  btAddress[0], btAddress[1], btAddress[2], btAddress[3], btAddress[4], btAddress[5]);
    // Additionally, you can get certain gamepad properties like:
    // Model, VID, PID, BTAddr, flags, etc.
    Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                  properties.product_id);
    myController = ctl;

    // disable any further BT connections so multiple controllers don't control one vehicle
    BP32.enableNewBluetoothConnections(false);

    ctl->playDualRumble(0 /* delayedStartMs */, 250 /* durationMs */, 0x80 /* weakMagnitude */, 0x40 /* strongMagnitude */);
    shouldWiggle = true;
    processLights(true);
  } else {
    Serial.printf("CALLBACK: Controller connected, ADDR=%2X:%2X:%2X:%2X:%2X:%2X, but another controller already connected.\n",
                  btAddress[0], btAddress[1], btAddress[2], btAddress[3], btAddress[4], btAddress[5]);
    ctl->disconnect();
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  ControllerProperties properties = ctl->getProperties();
  auto btAddress = properties.btaddr;
  if (myController == ctl) {
    Serial.printf("CALLBACK: Controller disconnected ADDR=%2X:%2X:%2X:%2X:%2X:%2X\n",
                  btAddress[0], btAddress[1], btAddress[2], btAddress[3], btAddress[4], btAddress[5]);
    myController = nullptr;
    BP32.enableNewBluetoothConnections(true);
  } else {
    Serial.printf("CALLBACK: Controller disconnected, ADDR=%2X:%2X:%2X:%2X:%2X:%2X, but not active controller\n",
                  btAddress[0], btAddress[1], btAddress[2], btAddress[3], btAddress[4], btAddress[5]);
  }
}

void processGamepad(ControllerPtr ctl) {
  processLeftThrottle(ctl->brake(), ctl->l1());
  processRightThrottle(ctl->throttle(), ctl->r1());
  processSwing(ctl->axisX());
  processBoom(ctl->axisY());
  processDipper(ctl->axisRY());
  processBucket(ctl->axisRX);
  
  processLights(ctl->thumbR() | ctl->a());

  processWiggle(ctl->b());
}

void processWiggle(bool value) {
  if (value) {
    shouldWiggle = true;
    processLights(true);
  }
}

void wiggle() {
  unsigned long currentTime = millis();
  if (abs((int)(currentTime - lastWiggleTime)) >= wiggleDelay) {
    lastWiggleTime = currentTime;
    wiggleDirection = -wiggleDirection;
    wiggleCount++;
    moveMotor(leftMotor0, leftMotor1, wiggleDirection * 100);
    moveMotor(rightMotor0, rightMotor1, wiggleDirection * 100);
    if (wiggleCount >= wiggleCountMax) {
      moveMotor(leftMotor0, leftMotor1, 0);
      moveMotor(rightMotor0, rightMotor1, 0);
      wiggleCount = 0;
      shouldWiggle = false;
      processLights(true);
    }
  }
}

void processLeftThrottle(int newValue, bool isReverse) {
  int throttleValue = newValue / 4 * (isReverse ? -1 : 1);
  // Serial.printf("brake: %d isReverse: %d throttleValue: %d\n", newValue, isReverse, throttleValue);
  moveMotor(leftMotor0, leftMotor1, throttleValue);
}

void processRightThrottle(int newValue, bool isReverse) {
  int throttleValue = newValue / 4 * (isReverse ? -1 : 1);
  // Serial.printf("throttle: %d isReverse: %d throttleValue: %d\n", newValue, isReverse, throttleValue);
  moveMotor(rightMotor0, rightMotor1, throttleValue);
}

void processSwing(int newValue) {
  int swingValue = newValue / 2;
  if (abs(swingValue) > swingDeadZone) {
    moveMotor(swingMotor0, swingMotor1, swingValue);
  } else {
    moveMotor(swingMotor0, swingMotor1, 0);
  }
}

void processLights(bool buttonValue) {
  if (buttonValue && (millis() - lightSwitchTime) > 200) {
    if (lightsOn) {
      digitalWrite(lightsAttach, LOW);
      lightsOn = false;
    } else {
      digitalWrite(lightsAttach, HIGH);
      lightsOn = true;
    }

    lightSwitchTime = millis();
  }
}

void processBoom(int newValue) {
  int boomValue = newvalue / 2;
  if (abs(boomValue) > boomDeadZone) {
    moveMotor(boomMotor0, boomMotor1, boomValue);
  } else {
    moveMotor(boomMotor0, boomMotor1, 0);
  }
}

void processDipper(int newValue) {
  int dipperValue = newvalue / 2;
  if (abs(dipperValue) > dipperDeadZone) {
    moveMotor(dipperMotor0, dipperMotor1, dipperValue);
  } else {
    moveMotor(dipperMotor0, dipperMotor1, 0);
  }
}

void processBucket(int newValue) {
  int bucketValue = newvalue / 2;
  if (abs(bucketValue) > bucketDeadZone) {
    moveMotor(bucketMotor0, bucketMotor1, bucketValue);
  } else {
    moveMotor(bucketMotor0, bucketMotor1, 0);
  }
}

void moveMotor(int motorPin0, int motorPin1, int velocity) {
  if (velocity > 1) {
    analogWrite(motorPin0, velocity);
    analogWrite(motorPin1, LOW);
  } else if (velocity < -1) {
    analogWrite(motorPin0, LOW);
    analogWrite(motorPin1, (-1 * velocity));
  } else {
    analogWrite(motorPin0, 0);
    analogWrite(motorPin1, 0);
  }
}

void processController() {
  if (myController && myController->isConnected() && myController->hasData()) {
    if (myController->isGamepad()) {
      processGamepad(myController);
    } else {
      Serial.println("Unsupported controller");
    }
  }
}

// Arduino setup function. Runs in CPU 1
void setup() {
  pinMode(leftMotor0, OUTPUT);
  pinMode(leftMotor1, OUTPUT);
  pinMode(rightMotor0, OUTPUT);
  pinMode(rightMotor1, OUTPUT);
  pinMode(swingMotor0, OUTPUT);
  pinMode(swingMotor1, OUTPUT);

  pinMode(lightsAttach, OUTPUT);
  digitalWrite(lightsAttach, LOW);

  pinMode(boomMotor0, OUTPUT);
  pinMode(boomMotor1, OUTPUT);
  pinMode(dipperMotor0, OUTPUT);
  pinMode(dipperMotor1, OUTPUT);
  pinMode(bucketMotor0, OUTPUT);
  pinMode(bucketMotor1, OUTPUT);
  
  // These pins are the serial port used for debugging in IDE
  // Do NOT use unless necessary
  // pinMode(miscMotor0, OUTPUT);
  // pinMode(miscMotor1, OUTPUT);

  clawServo.attach(clawServoPin);
  clawServo.write(clawInitialPosition);
  
  Serial.begin(115200);
  Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
  const uint8_t *addr = BP32.localBdAddress();
  Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

  // Setup the Bluepad32 callbacks
  BP32.setup(&onConnectedController, &onDisconnectedController);

  // "forgetBluetoothKeys()" should be called when the user performs
  // a "device factory reset", or similar.
  // Calling "forgetBluetoothKeys" in setup() just as an example.
  // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
  // But it might also fix some connection / re-connection issues.
  BP32.forgetBluetoothKeys();

  // Enables mouse / touchpad support for gamepads that support them.
  // When enabled, controllers like DualSense and DualShock4 generate two connected devices:
  // - First one: the gamepad
  // - Second one, which is a "virtual device", is a mouse.
  // By default, it is disabled.
  BP32.enableVirtualDevice(false);
  // You could add additional error handling here,
  // such as logging the error or attempting to recover.
  // For example, you might attempt to reset the MCP23X17
  // and retry initialization before giving up completely.
  // Then, you could gracefully exit the program or continue
  // running with limited functionality.
}



// Arduino loop function. Runs in CPU 1.
void loop() {
  // This call fetches all the controllers' data.
  // Call this function in your main loop.
  bool dataUpdated = BP32.update();
  if (dataUpdated) {
    processController();
  }
  if (shouldWiggle) {
    wiggle();
  }
  // The main loop must have some kind of "yield to lower priority task" event.
  // Otherwise, the watchdog will get triggered.
  // If your main loop doesn't have one, just add a simple `vTaskDelay(1)`.
  // Detailed info here:
  // https://stackoverflow.com/questions/66278271/task-watchdog-got-triggered-the-tasks-did-not-reset-the-watchdog-in-time

  //     vTaskDelay(1);
  else { vTaskDelay(1); }
}
