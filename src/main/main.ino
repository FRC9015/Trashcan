/****************************************************************************
http://retro.moe/unijoysticle2

Copyright 2021 Ricardo Quesada

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
****************************************************************************/

#include <Arduino.h>
#include <Bluepad32.h>
#include <iostream>


//
// README FIRST, README FIRST, README FIRST
//
// Bluepad32 has a built-in interactive console.
// By default it is enabled (hey, this is a great feature!).
// But it is incompatible with Arduino "Serial" class.
//
// Instead of using "Serial" you can use Bluepad32 "Console" class instead.
// It is somewhat similar to Serial but not exactly the same.
//
// Should you want to still use "Serial", you have to disable the Bluepad32's console
// from "sdkconfig.defaults" with:
//    CONFIG_BLUEPAD32_USB_CONSOLE_ENABLE=n

GamepadPtr myControllers[BP32_MAX_CONTROLLERS];
float mult = 1;
const int solenoidPIN = 25;  //a placeholder value is defined now, change this to the actual solenoid ID.
int linearRetractorID = 34;
int linearExtractorID = 35;

void onConnectedController(ControllerPtr ctl) {
  bool foundEmptySlot = false;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      Serial.print("CALLBACK: Controller is connected, index=");
      Serial.println(i);
      myControllers[i] = ctl;
      foundEmptySlot = true;
      // Optional, once the gamepad is connected, request further info about the
      // gamepad.
      ControllerProperties properties = ctl->getProperties();
      char buf[80];
      sprintf(buf,
              "BTAddr: %02x:%02x:%02x:%02x:%02x:%02x, VID/PID: %04x:%04x, "
              "flags: 0x%02x",
              properties.btaddr[0], properties.btaddr[1], properties.btaddr[2],
              properties.btaddr[3], properties.btaddr[4], properties.btaddr[5],
              properties.vendor_id, properties.product_id, properties.flags);
      Serial.println(buf);
      break;
    }
  }
  if (!foundEmptySlot) {
    Serial.println(
      "CALLBACK: Controller connected, but could not found empty slot");
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  bool foundGamepad = false;

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Serial.print("CALLBACK: Controller is disconnected from index=");
      Serial.println(i);
      myControllers[i] = nullptr;
      foundGamepad = true;
      break;
    }
  }

  if (!foundGamepad) {
    Serial.println(
      "CALLBACK: Controller disconnected, but not found in myControllers");
  }
}
void openTrashcan() {
  digitalWrite(linearRetractorID, LOW);
  digitalWrite(linearExtractorID, HIGH);
  
  delay(2000);

  digitalWrite(linearRetractorID, HIGH);
  digitalWrite(linearExtractorID, HIGH);
}

void closeTrashcan() {
  digitalWrite(linearRetractorID, HIGH);
  digitalWrite(linearExtractorID, LOW);

  delay(2000);

  digitalWrite(linearRetractorID, HIGH);
  digitalWrite(linearRetractorID, HIGH);
}
void processGamepad(ControllerPtr gamepad) {
  int x, y;
  if (gamepad->dpad() == 0x02 && mult > 0) {
    mult = mult - 0.1;
  }
  if (gamepad->dpad() == 0x01 && mult < 1) {
    mult = mult + 0.1;
  }

  if (gamepad->b()) {
    openTrashcan();
  }
  if (gamepad->a()) {
    closeTrashcan();
  }

  Console.printf(
    "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, "
    "%4d, brake: %4d, throttle: %4d, misc: 0x%02x\n",
    gamepad->index(),       // Gamepad Index
    gamepad->dpad(),        // DPAD
    gamepad->buttons(),     // bitmask of pressed buttons
    gamepad->axisX(),       // (-511 - 512) left X Axis
    gamepad->axisY(),       // (-511 - 512) left Y axis
    gamepad->axisRX(),      // (-511 - 512) right X axis
    gamepad->axisRY(),      // (-511 - 512) right Y axis
    gamepad->brake(),       // (0 - 1023): brake button
    gamepad->throttle(),    // (0 - 1023): throttle (AKA gas) button
    gamepad->miscButtons()  // bitmak of pressed "misc" buttons
  );
  if (abs(gamepad->axisY()) < 40) {
    y = 0;
  } else {
    y = gamepad->axisY();
  }

  if (y > 0) {
    ledcWrite(2, 255);
  } else {
    ledcWrite(2, 0);
  }
  ledcWrite(1, (abs(y) / 2) * mult);

  if (abs(gamepad->axisRY()) < 40) {
    x = 0;
  } else {
    x = gamepad->axisRY();
  }

  if (x > 0) {
    ledcWrite(4, 255);
  } else {
    ledcWrite(4, 0);
  }
  ledcWrite(3, (abs(x) / 2) * mult);
}


// Arduino setup function. Runs in CPU 1
void setup() {
  Serial.begin(9600);
  // Setup the Bluepad32 callbacks
  BP32.setup(&onConnectedController, &onDisconnectedController);

  // "forgetBluetoothKeys()" should be called when the user performs
  // a "device factory reset", or similar.
  // Calling "forgetBluetoothKeys" in setup() just as an example.
  // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
  // But might also fix some connection / re-connection issues.
  BP32.forgetBluetoothKeys();
  ledcAttachPin(18, 1);
  ledcSetup(1, 1000, 8);
  ledcAttachPin(19, 2);
  ledcSetup(2, 1000, 8);

  ledcAttachPin(21, 3);
  ledcSetup(3, 1000, 8);

  ledcAttachPin(22, 4);
  ledcSetup(4, 1000, 8);

  pinMode(LED_BUILTIN, OUTPUT);
  // linear actuator
  pinMode(linearRetractorID, OUTPUT);
  pinMode(linearExtractorID, OUTPUT);

  // Initializes Retractor Pin as Low
  digitalWrite(linearRetractorID, HIGH);
  digitalWrite(linearExtractorID, HIGH);
}

// Arduino loop function. Runs in CPU 1
void loop() {
  BP32.update();
  for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
    ControllerPtr myController = myControllers[i];

    if (myController && myController->isConnected()) {
      if (myController->isGamepad()) {
        processGamepad(myController);
      } else {
        ledcWrite(1, 0);
        ledcWrite(2, 0);
        ledcWrite(3, 0);
        ledcWrite(4, 0);
      }
    }
  }
  delay(150);
}