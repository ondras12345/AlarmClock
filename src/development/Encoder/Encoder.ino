#include <Encoder.h>

#define pin_encoder_clk 2
#define pin_encoder_dt 3
//#define pin_encoder_sw A2  // rotary encoder button

Encoder myEnc(pin_encoder_clk, pin_encoder_dt);

void setup() {
  Serial.begin(9600);
  Serial.println("Basic Encoder Test:");
}

long oldPosition  = -999;

void loop() {
  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    Serial.println(newPosition);
  }

  // Reset the count when a command is received
  if (Serial.available()) {
    while (Serial.available()) Serial.read();
    myEnc.write(0);
  }
}
