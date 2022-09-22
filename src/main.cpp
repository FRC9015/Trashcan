#include <Arduino.h>
#include <ps4.h>
#include <PS4Controller.h>
#include <ps4_int.h>

#include <driver/dac.h>

void setup()
{
  // Enable Serial output
  Serial.begin(115200);

  // Initialize PS4 Controller
  // TODO: Find and use correct MAC address
  PS4.begin("1a:2b:3c:01:01:01");

  // Enable DAC pins
  dac_output_enable(DAC_CHANNEL_1);
  dac_output_enable(DAC_CHANNEL_2);
}

void loop()
{
  int8_t xPos = PS4.LStickX();
  int8_t yPos = PS4.LStickY();

  Serial.print("X: ");
  Serial.println(xPos);
  Serial.print("Y: ");
  Serial.println(yPos);
}