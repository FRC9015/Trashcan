#include <Arduino.h>
#include <ps4.h>
#include <PS4Controller.h>
#include <ps4_int.h>

#include <driver/dac.h>

void onConnect()
{
  Serial.println("Connected!.");
}

void onDisConnect()
{
  Serial.println("Disconnected!.");
}

void setup()
{
  // Enable Serial output
  Serial.begin(115200);

  // Enable DAC pins
  dac_output_enable(DAC_CHANNEL_1); // GPIO25
  dac_output_enable(DAC_CHANNEL_2); // GPIO26

  PS4.attachOnConnect(onConnect);
  PS4.attachOnDisconnect(onDisConnect);

  PS4.begin();

  Serial.println("Setup complete");
}

void loop()
{
  // Read PS4 Controller Outputs
  int8_t rawXpos = PS4.LStickX();
  int8_t rawYpos = PS4.LStickY();

  // Convert to 0-255 range
  uint8_t xPos = map(rawXpos, -128, 127, 0, 255);
  uint8_t yPos = map(rawYpos, -128, 127, 0, 255);

  // Log out values to Serial console
  Serial.print("X: ");
  Serial.println(xPos);
  Serial.print("Y: ");
  Serial.println(yPos);

  // Write values to DAC (experimental)
  dac_output_voltage(DAC_CHANNEL_1, xPos);
  dac_output_voltage(DAC_CHANNEL_2, yPos);

  delay(1000);
}