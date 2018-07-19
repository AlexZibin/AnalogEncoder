#include "AnalogEncoder.h"

AnalogEncoder analogEncoder (A1, A2);

void setup () {
    Serial.begin (9600);
}

void loop () {
    static int counter = 0;

    int32_t analogEncoderPosition = analogEncoder.read ();

    if (analogEncoderPosition) {
        Serial.println (++counter);
        Serial.print ("analogEncoderPosition: "); Serial.println (analogEncoderPosition); 
        //analogEncoderPosition = 0;
    }
}
