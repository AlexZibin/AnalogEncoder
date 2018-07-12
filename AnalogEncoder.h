#include "fifo.h"
#include "timer.h"

class AnalogEncoder {
    public:
        AnalogEncoder (uint8_t pinL, uint8_t pinR, uint8_t bufferSize = 10, uint16_t samplingRate_ms = 10);
    private:
        Fifo *bufferL;
        Fifo *bufferR;
        Timer timer;
}


////////  AnalogEncoder.cpp:

AnalogEncoder::AnalogEncoder (uint8_t pinL, uint8_t pinR, uint8_t bufferSize, uint16_t samplingRate_ms) {
    bufferL = new Fifo (bufferSize);
    bufferR = new Fifo (bufferSize);
    timer.setInterval (samplingRate_ms);
    timer.switchOn ();
}
