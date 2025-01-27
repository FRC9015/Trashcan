#include <Bluepad32.h>

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

#include "sdkconfig.h"
#ifndef CONFIG_BLUEPAD32_PLATFORM_ARDUINO
#error "Must only be compiled when using Bluepad32 Arduino platform"
#endif  // !CONFIG_BLUEPAD32_PLATFORM_ARDUINO
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

GamepadPtr myGamepads[BP32_MAX_GAMEPADS];

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedGamepad(GamepadPtr gp) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myGamepads[i] == nullptr) {
            Console.printf("CALLBACK: Gamepad is connected, index=%d\n", i);
            // Additionally, you can get certain gamepad properties like:
            // Model, VID, PID, BTAddr, flags, etc.
            GamepadProperties properties = gp->getProperties();
            Console.printf("Gamepad model: %s, VID=0x%04x, PID=0x%04x\n", gp->getModelName(), properties.vendor_id,
                           properties.product_id);
            myGamepads[i] = gp;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
        Console.println("CALLBACK: Gamepad connected, but could not found empty slot");
    }
}

void onDisconnectedGamepad(GamepadPtr gp) {
    bool foundGamepad = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myGamepads[i] == gp) {
            Console.printf("CALLBACK: Gamepad is disconnected from index=%d\n", i);
            myGamepads[i] = nullptr;
            foundGamepad = true;
            break;
        }
    }

    if (!foundGamepad) {
        Console.println("CALLBACK: Gamepad disconnected, but not found in myGamepads");
    }
}

float mult = 1;
const int solenoidPin = 25; //a placeholder value is defined now, change this to the actual solenoid ID.

void pushSolenoid() {
    digitalWrite(solenoidPin, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);
}

void pullSolenoid(){
    digitalWrite(solenoidPIN, LOW);
    digitalWrite(LED_BUILTIN, LOW);
}


// Arduino setup function. Runs in CPU 1
void setup() {
    Console.printf("Firmware: %s\n", BP32.firmwareVersion());
    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);

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
}

// Arduino loop function. Runs in CPU 1
void loop() {
    
    int x, y;
    // This call fetches all the gamepad info from the NINA (ESP32) module.
    // Just call this function in your main loop.
    // The gamepads pointer (the ones received in the callbacks) gets updated
    // automatically.
    BP32.update();

    // It is safe to always do this before using the gamepad API.
    // This guarantees that the gamepad is valid and connected.
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        GamepadPtr myGamepad = myGamepads[i];

        if (myGamepad && myGamepad->isConnected()) {
            // There are different ways to query whether a button is pressed.
            // By query each button individually:
            //  a(), b(), x(), y(), l1(), etc...

            if (myGamepad->dpad()==0x02 && mult >0) {
                mult = mult-0.1;
            }
            if (myGamepad->dpad()==0x01 && mult <1) {
                mult = mult+0.1;
            }
            
            if (myGamepad->b()) {
                pushSolenoid();

            }
            if (myGamepad->a()) {
                pullSolenoid();
            }
            

            // Another way to query the buttons, is by calling buttons(), or
            // miscButtons() which return a bitmask.
            // Some gamepads also have DPAD, axis and more.
            Console.printf(
                "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, "
                "%4d, brake: %4d, throttle: %4d, misc: 0x%02x\n",
                i,                        // Gamepad Index
                myGamepad->dpad(),        // DPAD
                myGamepad->buttons(),     // bitmask of pressed buttons
                myGamepad->axisX(),       // (-511 - 512) left X Axis
                myGamepad->axisY(),       // (-511 - 512) left Y axis
                myGamepad->axisRX(),      // (-511 - 512) right X axis
                myGamepad->axisRY(),      // (-511 - 512) right Y axis
                myGamepad->brake(),       // (0 - 1023): brake button
                myGamepad->throttle(),    // (0 - 1023): throttle (AKA gas) button
                myGamepad->miscButtons()  // bitmak of pressed "misc" buttons
                
                
            );
            if (abs(myGamepad->axisY())<40){
                y=0;
            }else{
                y= myGamepad->axisY();
            }

            if (y>0){
                ledcWrite(2,255);
            }else{
                ledcWrite(2,0);

            }
            ledcWrite(1, (abs(y)/2)*mult);

            if (abs(myGamepad->axisRY())<40){
                x=0;
            }else{
                x= myGamepad->axisRY();
            }

            if (x>0){
                ledcWrite(4,255);
            }else{
                ledcWrite(4,0);

            }
            ledcWrite(3, (abs(x)/2)*mult);

            // You can query the axis and other properties as well. See Gamepad.h
            // For all the available functions.
        }
    }

    delay(150);
}
