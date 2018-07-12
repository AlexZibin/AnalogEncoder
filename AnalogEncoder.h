#include "fifo.h"
#include "timer.h"

class AnalogEncoder {
    public:
        AnalogEncoder (uint8_t pinL, uint8_t pinR, uint8_t bufferSize = 10, uint16_t samplingRate_ms = 10);
        int32_t read (); // Here runs the main integrating & comparison staff
        inline void write (int32_t p) {position = p;};
        bool registerPattern (Fifo *pattern, CallBackFunction func); // Pattern format: Direction (+1/-1) / delayMs / Direction / delayMs / ...

    private:
        Fifo *bufferL;
        Fifo *bufferR;
        Timer timer;
        int32_t position;
}


////////  AnalogEncoder.cpp:

AnalogEncoder::AnalogEncoder (uint8_t pinL, uint8_t pinR, uint8_t bufferSize, uint16_t samplingRate_ms) {
    pinMode (pinL, INPUT);
    pinMode (pinR, INPUT);
    position = 0;
    bufferL = new Fifo (bufferSize);
    bufferR = new Fifo (bufferSize);
    timer.setInterval (samplingRate_ms);
    timer.switchOn ();
}

int32_t AnalogEncoder::read () { // Here runs the main integrating & comparison staff
    const int positionIncrement = 4;
    
    if (timer.needToTrigger ()) {
        
    }
}
